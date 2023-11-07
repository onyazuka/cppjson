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


namespace json {

	struct Null {

	};

	namespace util {
		std::string_view strip(std::string_view v);
		std::vector<std::string_view> split(std::string_view v, const std::string& delim);
		std::vector<std::string_view> smartSplit(std::string_view v, char delim);
		std::optional<size_t> getIdx(std::string_view v);
	}

	template<typename T, typename Args>
	struct IsOneOfVariants : std::false_type {

	};

	template<typename T, typename...Args>
		requires (std::same_as<T, Args> || ...)
	struct IsOneOfVariants<T, std::variant<Args...>> : std::true_type {

	};

	template <typename Cont>
	concept StringVector = std::same_as<Cont, std::vector<std::string>> || std::same_as<Cont, std::vector<std::string_view>>;

	struct Node {
		enum class Type {
			Int,
			Float,
			String,
			Bool,
			Null,
			Array,
			Object,
			NoType
		};
		Node(Type type);
		virtual ~Node();
		Type type;
	};

	namespace check {
		//bool isObj(std::string_view v);
		//bool isArr(std::string_view v);
		//bool isInt(std::string_view v);
		//bool isFloat(std::string_view v);
		bool isStr(std::string_view v);
		//bool isBool(std::string_view v);
		//bool isNull(std::string_view v);
		bool isNumberStart(char ch);
		Node::Type getType(std::string_view v);
	}

	class ValNode : public Node {
	public:
		using Val = std::variant<
			int64_t,
			double,
			std::string,
			bool,
			Null
		>;
		ValNode(int64_t v);
		ValNode(double d);
		ValNode(const std::string& s);
		ValNode(bool b);
		ValNode();
		template<typename T>
			requires IsOneOfVariants<T, Val>::value
		inline T as() {
			return std::get<T>(val);
		}
	private:
		Val val;
	};

	class ArrNode : public Node {
	public:
		ArrNode();
		ArrNode(std::vector<Node*>&& v);
		~ArrNode();
		const auto& get();
		Node* get(size_t idx);
		template<typename T>
		std::vector<T> as();
		void add(Node* node);
	private:
		bool isMonotype() const;
		std::vector<Node*> val;
	};

	class ObjNode : public Node {
	public:
		ObjNode();
		ObjNode(std::unordered_map<std::string, Node*>&& m);
		~ObjNode();
		const auto& get();
		Node* get(const std::string& key);
		void add(const std::string& key, Node* val);
	private:
		std::unordered_map<std::string, Node*> val;
	};

	class Json {
	public:
		Json();
		Json(Node* r);
		~Json();
		template<typename T>
		T as(const std::string& key);
		template<typename T, StringVector Cont>
		T as(const Cont& keys);
		bool empty() const;
	private:
		Node* get(const std::string& key);
		template<StringVector Cont>
		Node* get(const Cont& keys);
		template<StringVector Cont >
		Node* _getImpl(const Cont& keys);
		template<typename T>
		T _asImpl(Node* node);
		Node* root;
	};

	template<typename T>
	std::vector<T> ArrNode::as() {
		std::vector<T> res;
		for (Node* node : val) {
			if (auto ptr = dynamic_cast<ValNode*>(node); ptr) {
				res.push_back(ptr->as<T>());
			}
		}
		return res;
	}

	template<StringVector Cont>
	Node* Json::get(const Cont& keys) {
		return _getImpl(keys);
	}

	template<StringVector Cont>
	Node* Json::_getImpl(const Cont& keys) {
		Node* curNode = root;
		for (auto& key : keys) {
			if (curNode->type == Node::Type::Object) {
				if (auto ptr = dynamic_cast<ObjNode*>(curNode); ptr) {
					curNode = ptr->get(std::string(key.begin(), key.end()));
				}
				else {
					assert(false);
				}
			}
			else if (curNode->type == Node::Type::Array) {
				if (auto idx = util::getIdx(key); idx) {
					if (auto ptr = dynamic_cast<ArrNode*>(curNode); ptr) {
						curNode = ptr->get(idx.value());
					}
					else {
						assert(false);
					}
				}
				else {
					assert(false);
				}
			}
			else {
				assert(false);
			}
		}
		return curNode;
	}


	// limitations - '.' delimiter and '[]' indexes
	template<typename T>
	T Json::as(const std::string& key) {
		return _asImpl<T>(get(key));
	}

	template<typename T, StringVector Cont>
	T Json::as(const Cont& keys) {
		return _asImpl<T>(get(keys));
	}

	template<typename T>
	T Json::_asImpl(Node* node) {
		if constexpr (IsOneOfVariants<T, ValNode::Val>::value) {
			if (auto ptr = dynamic_cast<ValNode*>(node); ptr) {
				return ptr->as<T>();
			}
			else {
				assert(false);
			}
		}
		else {
			if (auto ptr = dynamic_cast<ArrNode*>(node); ptr) {
				return ptr->as<typename T::value_type>();
			}
			else {
				assert(false);
			}
		}
		assert(false);
		return T();
	}

	class JsonDecoder {
	public:
		JsonDecoder();
		Json decode(std::string_view v);
	private:
		Node* decodeImpl(std::string_view v);
		Node* decodeObj(std::string_view v);
		Node* decodeArr(std::string_view v);
		Node* decodeInt(std::string_view v);
		Node* decodeFloat(std::string_view v);
		Node* decodeStr(std::string_view v);
		Node* decodeBool(std::string_view v);
		Node* decodeNull(std::string_view v);
	};

}
