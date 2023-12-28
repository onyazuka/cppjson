#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <variant>
#include <any>
#include <optional>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <format>
#include <fstream>
#include <cstdint>
#include <functional>
#include "Utils_String.hpp"
#include "Utils_Traits.hpp"


namespace util::web::json {

	struct Null {

	};

	namespace utils {
		std::vector<std::string_view> smartSplit(std::string_view v, char delim);
		std::optional<size_t> getIdx(std::string_view v);
	}

	template <typename Cont>
	concept StringVector = std::same_as<Cont, std::vector<std::string>> || std::same_as<Cont, std::vector<std::string_view>>;

	template <typename T>
	concept ElemToObjNodeConvertable = requires(T val) {
		std::declval<typename T::value_type>().toObjNode();
	};

	class ValNode;
	class ArrNode;
	class ObjNode;

	enum class NodeType {
		Int,
		Float,
		String,
		Bool,
		Null,
		Array,
		Object,
		NoType
	};

	using Node = std::variant<ValNode, ArrNode, ObjNode>;

	namespace check {
		//bool isObj(std::string_view v);
		//bool isArr(std::string_view v);
		//bool isInt(std::string_view v);
		//bool isFloat(std::string_view v);
		bool isStr(std::string_view v);
		//bool isBool(std::string_view v);
		//bool isNull(std::string_view v);
		bool isNumberStart(char ch);
		NodeType getType(std::string_view v);
		bool isValueType(NodeType t);
	}

	class ValNode {
	public:
		using Val = std::variant<
			int64_t,
			double,
			std::string,
			bool,
			Null
		>;
		template<typename IntT>
			requires std::is_integral_v<IntT>
		ValNode(IntT v);
		// potential overflow (not a problem since correct cast from i64 to u64 and vice versa)
		//ValNode(uint64_t v) = delete;
		template<typename FloatT>
			requires std::is_floating_point_v<FloatT>
		ValNode(FloatT d);
		ValNode(const std::string& s);
		ValNode(std::string&& s);
		// defined, because bool constructor is called instead with this argument type
		ValNode(const char* s);
		ValNode(bool b);
		ValNode();
		template<typename T>
			requires util::traits::IsOneOfVariants<T, Val>::value
		inline T as() const {
			return std::get<T>(val);
		}
		inline NodeType type() const { return _type; }
	private:
		Val val;
		NodeType _type;
	};

	template<typename IntT>
		requires std::is_integral_v<IntT>
	ValNode::ValNode(IntT v)
		: val{ v }, _type{ NodeType::Int }
	{

	}

	template<typename FloatT>
		requires std::is_floating_point_v<FloatT>
	ValNode::ValNode(FloatT d)
		: val{ d }, _type{ NodeType::Float }
	{

	}

	class ArrNode {
	public:
		ArrNode();
		ArrNode(const std::vector<Node>& v);
		ArrNode(std::vector<Node>&& v);
		const std::vector<Node>& ccont() const;
		std::vector<Node>& cont();
		template<typename T>
		std::vector<T> as() const;
		inline size_t size() const { return val.size(); }
		inline NodeType type() const { return _type; }
		template<typename T> requires ElemToObjNodeConvertable<T>
		static ArrNode makeFrom(const T& cont);
	private:
		bool isMonotype() const;
		std::vector<Node> val;
		NodeType _type;
	};

	template<typename T> requires ElemToObjNodeConvertable<T>
	ArrNode ArrNode::makeFrom(const T& cont) {
		std::vector<Node> nodes;
		for (const auto& elem : cont) {
			nodes.push_back(elem.toObjNode());
		}
		return ArrNode(std::move(nodes));
	}

	class ObjNode {
	public:
		ObjNode();
		ObjNode(const std::unordered_map<std::string, Node>& m);
		ObjNode(std::unordered_map<std::string, Node>&& m);
		const std::unordered_map<std::string, Node>& ccont() const;
		std::unordered_map<std::string, Node>& cont();
		std::vector<std::string> keys() const;
		inline NodeType type() const { return _type; }
		template<typename T, typename F>
		static ObjNode makeFrom(const T& cont, F extractor);

	private:
		std::unordered_map<std::string, Node> val;
		NodeType _type;
	};

	template<typename T, typename F>
	ObjNode ObjNode::makeFrom(const T& cont, F extractor) {
		std::unordered_map<std::string, Node> nodes;
		for (const auto& elem : cont) {
			nodes.insert(extractor(elem));
		}
		return nodes;
	}

	class Json {
		friend class JsonDecoder;
		friend class JsonEncoder;
	public:
		Json();
		Json(Node& r);
		Json(Node&& r);

		template<typename T>
		T as();
		template<typename T>
		T as(const std::string& key);
		template<typename T, StringVector Cont>
		T as(const Cont& keys);

		bool empty() const;

		std::vector<std::string> keys(const std::string& key);
		template<StringVector Cont>
		std::vector<std::string> keys(const Cont& keys);

		size_t arrSize(const std::string& key);
		template<StringVector Cont>
		size_t arrSize(const Cont& keys);

		Node& get(const std::string& key);
		template<StringVector Cont>
		Node& get(const Cont& keys);
		inline Node& get() { return root.value(); }
	private:
		template<StringVector Cont >
		Node& _getImpl(const Cont& keys);
		template<typename T>
		T _asImpl(Node& node);
		std::vector<std::string> _keysImpl(const Node& node) const;
		size_t _arrSizeImpl(Node& node);
		std::optional<Node> root;
	};

	template<typename T>
	std::vector<T> ArrNode::as() const {
		std::vector<T> res;
		for (const Node& node : val) {
			const ValNode& val = std::get<ValNode>(node);
			res.push_back(val.as<T>());
		}
		return res;
	}

	template<StringVector Cont>
	Node& Json::get(const Cont& keys) {
		return  _getImpl(keys);
	}

	template<StringVector Cont>
	Node& Json::_getImpl(const Cont& keys) {
		Node* curNode = &(root.value());
		for (auto& key : keys) {
			if (std::holds_alternative<ObjNode>(*curNode)) {
				curNode = &(std::get<ObjNode>(*curNode).cont().at(std::string(key.begin(), key.end())));
			}
			else if (std::holds_alternative<ArrNode>(*curNode)) {
				if (auto idx = utils::getIdx(key); idx) {
					curNode = &(std::get<ArrNode>(*curNode).cont().at(idx.value()));
				}
				else {
					throw std::out_of_range("couldn't get node");
				}
			}
			else {
				throw std::out_of_range("couldn't get node");
			}
		}
		return *curNode;
	}

	template<typename T>
	T Json::as() {
		return _asImpl<T>(root.value());
	}

	// limitations - '.' delimiter and '[]' indexes
	template<typename T>
	T Json::as(const std::string& key) {
		Node& node = get(key);
		return _asImpl<T>(node);
	}

	template<typename T, StringVector Cont>
	T Json::as(const Cont& keys) {
		Node& node = get(keys);
		return _asImpl<T>(node);
	}

	template<StringVector Cont>
	std::vector<std::string> Json::keys(const Cont& keys) {
		const Node& node = get(keys);
		return _keysImpl(node);
	}

	template<StringVector Cont>
	size_t Json::arrSize(const Cont& keys) {
		Node& node = get(keys);
		return _arrSizeImpl(node);
	}

	template<typename T>
	T Json::_asImpl(Node& node) {
		if constexpr (util::traits::IsOneOfVariants<T, ValNode::Val>::value) {
			if (std::holds_alternative<ValNode>(node)) {
				return std::get<ValNode>(node).as<T>();
			}
		}
		else if constexpr (std::is_integral_v<T>) {
			if (std::holds_alternative<ValNode>(node)) {
				return static_cast<T>(std::get<ValNode>(node).as<int64_t>());
			}
		}
		else if constexpr (std::is_floating_point_v<T>) {
			if (std::holds_alternative<ValNode>(node)) {
				return static_cast<T>(std::get<ValNode>(node).as<double>());
			}
		}
		else {
			if (std::holds_alternative<ArrNode>(node)) {
				return std::get<ArrNode>(node).as<typename T::value_type>();
			}
		}
		assert(false);
		return T();
	}

	class JsonDecoder {
	public:
		JsonDecoder();
		Json decode(std::string_view v);
		Json decode(std::ifstream& is);
		Json decode(std::ifstream&& is);
	private:
		Node decodeImpl(std::string_view v);
		Node decodeObj(std::string_view v);
		Node decodeArr(std::string_view v);
		Node decodeInt(std::string_view v);
		Node decodeFloat(std::string_view v);
		Node decodeStr(std::string_view v);
		Node decodeBool(std::string_view v);
		Node decodeNull(std::string_view v);
	};

	class JsonEncoder {
	public:
		struct Opts {
			bool humanReadable = false;
		};
		JsonEncoder();
		JsonEncoder(const Opts& opts);
		std::string encode(const Json& json);
		void dump(const Json& json, std::ostream& os);
		void dump(const Json& json, std::ostream&& os);
	private:
		void encodeImpl(std::string& s, const Node& node);
		void encodeNull(std::string& s);
		void encodeBool(std::string& s, const ValNode& node);
		void encodeInt(std::string& s, const ValNode& node);
		void encodeFloat(std::string& s, const ValNode& node);
		void encodeStr(std::string& s, const ValNode& node);
		template <bool Hr>
		void encodeArray(std::string& s, const ArrNode& node);
		template <bool Hr>
		void encodeObj(std::string& s, const ObjNode& node);
		void appendIntendation(std::string& s);
		Opts opts;
		struct EncodingCtx {
			size_t intendationLvl = 0;
			void reset();
		} ctx;
	};

	template <bool Hr>
	void JsonEncoder::encodeArray(std::string& s, const ArrNode& node) {
		++ctx.intendationLvl;
		s.append("[");
		if constexpr (Hr) s.append("\n");
		const auto& arrNodes = node.ccont();
		for (size_t i = 0; i < arrNodes.size(); ++i) {
			if constexpr (Hr) appendIntendation(s);
			encodeImpl(s, arrNodes[i]);
			if (i < (arrNodes.size() - 1)) {
				s.append(",");
			}
			if constexpr (Hr) s.append("\n");
		}
		--ctx.intendationLvl;
		if constexpr (Hr) appendIntendation(s);
		s.append("]");
	}

	template <bool Hr>
	void JsonEncoder::encodeObj(std::string& s, const ObjNode& node) {
		++ctx.intendationLvl;
		s.append("{");
		if constexpr (Hr)  s.append("\n");
		const auto& objItems = node.ccont();
		size_t i = 0;
		for (const auto& [key, curNode] : objItems) {
			if constexpr (Hr)  appendIntendation(s);
			s.append(std::format("\"{}\":", key));
			encodeImpl(s, curNode);
			if (i < objItems.size() - 1) {
				s.append(",");
			}
			++i;
			if constexpr (Hr)  s.append("\n");
		}
		--ctx.intendationLvl;
		if constexpr (Hr)  appendIntendation(s);
		s.append("}");
	}

}
