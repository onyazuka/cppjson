
#include "Json.hpp"
#include <stdexcept>
#include <charconv>
#include <iostream>
#include <format>
#include <sstream>

using namespace json;

static constexpr char Spaces[] = "\t\n\r ";

std::string_view util::strip(std::string_view v) {
	auto p1 = v.find_first_not_of(Spaces);
	auto p2 = v.find_last_not_of(Spaces);
	if (p1 == std::string_view::npos) {
		return {};
	}
	return v.substr(p1, p2 ? p2 - p1 + 1 : v.npos);
}

std::vector<std::string_view> util::split(std::string_view v, const std::string& delim) {
	if (v.empty()) return {};
	size_t pos = 0;
	std::vector<std::string_view> res;
	do {
		size_t newPos = v.find(delim, pos);
		if (newPos != std::string_view::npos) {
			res.push_back(v.substr(pos, newPos - pos));
			newPos += delim.size();
		}
		else {
			res.push_back(v.substr(pos));
		}
		pos = newPos;
	} while (pos != std::string_view::npos);
	return res;
}

// not counts 'instring' characters, it means if 'delim' is inside a string, it will be no split here
std::vector<std::string_view> util::smartSplit(std::string_view v, char delim) {
	// not supported characters
	assert(delim != '"' && delim != '\\');
	bool insideString = false;
	size_t insideArray = 0;
	size_t insideObj = 0;
	size_t lastPos = 0;
	size_t pos = 0;
	std::vector<std::string_view> res;
	while (pos < v.size()) {
		if (v[pos] == delim) {
			if (insideString || insideArray || insideObj) {
				;
			}
			else {
				res.push_back(std::string_view(v.begin() + lastPos, v.begin() + pos));
				// skipping delim
				lastPos = pos + 1;
			}
		}
		else if (v[pos] == '"') {
			if ((pos > 0) && (v[pos - 1] == '\\')) {
				// escaped quote - do nothing
				;
			}
			else {
				insideString = !insideString;
			}
		}
		else if (v[pos] == '[' || v[pos] == ']') {
			if ((pos > 0) && (v[pos - 1] == '\\')) {
				// escaped quote - do nothing
				;
			}
			else {
				if (v[pos] == '[') ++insideArray;
				// if v[pos] == ']'
				else --insideArray;
			}
		}
		else if (v[pos] == '{' || v[pos] == '}') {
			if ((pos > 0) && (v[pos - 1] == '\\')) {
				// escaped quote - do nothing
				;
			}
			else {
				if (v[pos] == '{') ++insideObj;
				// if v[pos] == '}'
				else --insideObj;
			}
		}
		else {
			;
		}
		++pos;
	}
	res.push_back(std::string_view(v.begin() + lastPos, v.end()));
	return res;
}

std::optional<size_t> util::getIdx(std::string_view v) {
	size_t pos1 = v.find_last_of("[");
	size_t pos2 = v.find_last_of("]");
	if ((pos1 != v.npos) && (pos2 != v.npos) && ((pos1 + 1) < pos2)) {
		auto sub = v.substr(pos1 + 1, pos2 - (pos1 + 1));
		return std::stoull(std::string(sub.begin(), sub.end()));
	}
	else {
		return std::nullopt;
	}
}

bool check::isStr(std::string_view v) {
	if ((v.size() < 2) || (v.front() != '"' || v.back() != '"')) {
		return false;
	}
	size_t pos = 1;
	if (v[v.size() - 2] == '\\') {
		return false;
	}
	// excluding closing quote
	while (pos < (v.size() - 1)) {
		pos = v.find('"', pos);
		if (pos < (v.size() - 1)) {
			if (v[pos - 1] != '\\') {
				return false;
			}
		}
	}
	return true;
}

bool check::isNumberStart(char ch) {
	return (ch >= '0' && ch <= '9') || (ch == '-');
}

NodeType check::getType(std::string_view v) {
	if (v.empty()) return NodeType::NoType;
	if (v.size() >= 2) {
		if (v.front() == '{' && v.back() == '}') {
			return NodeType::Object;
		}
		else if (v.front() == '[' && v.back() == ']') {
			return NodeType::Array;
		}
		else if (v.front() == '"' && v.back() == '"') {
			return NodeType::String;
		}
	}
	if (v == "null") {
		return NodeType::Null;
	}
	else if (v == "true" || v == "false") {
		return NodeType::Bool;
	}
	else if (v.find_first_of(".") != v.npos) {
		return NodeType::Float;
	}
	else {
		return NodeType::Int;
	}
}

bool check::isValueType(NodeType t) {
	return t == NodeType::Bool || t == NodeType::Null || t == NodeType::Int || t == NodeType::Float || t == NodeType::String;
}

ValNode::ValNode(const std::string& s)
	: val{ s }, _type{ NodeType::String }
{

}

ValNode::ValNode(const char* s) 
	: val{ s }, _type{ NodeType::String }
{

}

ValNode::ValNode(bool b)
	: val{b}, _type{ NodeType::Bool }
{

}

ValNode::ValNode() 
	: val{Null()}, _type{ NodeType::Null }
{
	
}

ArrNode::ArrNode()
	: val{}, _type{ NodeType::Array }
{

}


ArrNode::ArrNode(const std::vector<Node>& v)
	: _type{ NodeType::Array }
{
	val = v;
}

ArrNode::ArrNode(std::vector<Node>&& v)
	: _type{NodeType::Object}
{
	val = std::move(v);
}

const std::vector<Node>& ArrNode::ccont() const {
	return val;
}

std::vector<Node>& ArrNode::cont() {
	return val;
}

Node& ArrNode::get(size_t idx) {
	return val.at(idx);
}

void ArrNode::add(const Node& node) {
	val.push_back(node);
}

bool ArrNode::isMonotype() const {
	if (val.size() == 0) return true;
	NodeType t0 = std::get<ValNode>(val[0]).type();
	return std::all_of(val.cbegin(), val.cend(), [t0](const auto& v) {
		return std::get<ValNode>(v).type() == t0;
		});
}

ObjNode::ObjNode()
	: _type{ NodeType::Object }
{

}

ObjNode::ObjNode(const std::unordered_map<std::string, Node>& m)
	: _type{ NodeType::Object } 
{
	val = m;
}

ObjNode::ObjNode(std::unordered_map<std::string, Node>&& m)
	: _type{ NodeType::Object }
{
	val = std::move(m);
}

const std::unordered_map<std::string, Node>& ObjNode::ccont() const {
	return val;
}

std::unordered_map<std::string, Node>& ObjNode::cont() {
	return val;
}

Node& ObjNode::get(const std::string& key) {
	return val.at(key);
}

void ObjNode::add(const std::string& key, const Node& node) {
	val[key] = node;
}

std::vector<std::string> ObjNode::keys() {
	std::vector<std::string> res;
	for (const auto& [key, v] : val) {
		res.push_back(key);
	}
	return res;
}

Json::Json(Node& r) 
	: root{ r }
{

}

Json::Json(Node&& r)
	: root{ std::move(r) }
{
	;
}

Json::Json() 
	: root{std::nullopt}
{

}

bool Json::empty() const {
	return root == std::nullopt;
}

// getting keys of object nodes
std::vector<std::string> Json::keys(const std::string& key) {
	Node& node = get(key);
	return _keysImpl(node);
}

size_t Json::arrSize(const std::string& key) {
	Node& node = get(key);
	return _arrSizeImpl(node);
}

std::vector<std::string> Json::_keysImpl(Node& node) {
	if (std::holds_alternative<ObjNode>(node)) {
		return std::get<ObjNode>(node).keys();
	}
	else {
		assert(false);
	}
	return {};
}

size_t Json::_arrSizeImpl(Node& node) {
	if (std::holds_alternative<ArrNode>(node)) {
		return std::get<ArrNode>(node).size();
	}
	else {
		assert(false);
	}
	return {};
}

Node& Json::get(const std::string& key) {
	Node& node = _getImpl(util::split(key, "."));
	return node;
}

JsonDecoder::JsonDecoder() {

}

Json JsonDecoder::decode(std::string_view v) {
	if (v.empty()) return Json();
	return Json(decodeImpl(v));
}

Json JsonDecoder::decode(std::ifstream& is) {
	std::stringstream ss;
	ss << is.rdbuf();
	return Json(decodeImpl(ss.str()));
}

Json JsonDecoder::decode(std::ifstream&& is) {
	std::stringstream ss;
	ss << is.rdbuf();
	return Json(decodeImpl(ss.str()));
}

Node JsonDecoder::decodeImpl(std::string_view v) {
	std::string_view body = util::strip(v);
	NodeType type = check::getType(body);
	switch (type) {
	case NodeType::Object:
		return decodeObj(body);
	case NodeType::Array:
		return decodeArr(body);
	case NodeType::String:
		return decodeStr(body);
	case NodeType::Int:
		return decodeInt(body);
	case NodeType::Float:
		return decodeFloat(body);
	case NodeType::Bool:
		return decodeBool(body);
	case NodeType::Null:
		return decodeNull(body);
	default:
		throw std::logic_error("JSON: invalid node type");
	}
}

Node JsonDecoder::decodeBool(std::string_view v) {
	if (v == "true") {
		return ValNode((bool)(true));
	}
	else if (v == "false") {
		return ValNode((bool)(false));
	}
	throw std::runtime_error("invalid bool node");
}

Node JsonDecoder::decodeNull(std::string_view v) {
	if (v == "null") {
		return ValNode();
	}
	throw std::runtime_error("invalid null node");
}

Node JsonDecoder::decodeInt(std::string_view v) {
	int64_t val;
	if (std::from_chars(v.data(), v.data() + v.size(), val).ec != std::errc{}) {
		throw std::runtime_error("invalid int node");
	}
	return ValNode(val);
}

Node JsonDecoder::decodeFloat(std::string_view v) {
	double val;
	if (std::from_chars(v.data(), v.data() + v.size(), val).ec != std::errc{}) {
		throw std::runtime_error("invalid float node");
	}
	return ValNode(val);
}

Node JsonDecoder::decodeStr(std::string_view v) {
	if (!check::isStr(v)) {
		throw std::runtime_error("invalid string node");
	}
	return ValNode(std::string(v.data() + 1, v.size() - 2));
}

Node JsonDecoder::decodeArr(std::string_view v) {
	auto elems = util::smartSplit(util::strip(std::string_view{v.data() + 1, v.size() - 2}), ',');
	ArrNode arr;
	for (std::string_view elem : elems) {
		arr.add(decodeImpl(util::strip(elem)));
	}
	return arr;
}

Node JsonDecoder::decodeObj(std::string_view v) {
	auto elems = util::smartSplit(util::strip(std::string_view{v.data() + 1, v.size() - 2}), ',');
	ObjNode obj;
	size_t i = 0;
	for (std::string_view elem : elems) {
		auto pair = util::smartSplit(util::strip(std::string_view{ elem.data(), elem.size() }), ':');
		if (pair.size() != 2) {
			throw std::runtime_error("invalid object node");
		}
		if (pair[0].front() != '"' || pair[0].back() != '"') {
			throw std::runtime_error("invalid object node");
		}
		if (!check::isStr(pair[0])) {
			throw std::runtime_error("invalid object node");
		}
		Node valNode = decodeImpl(util::strip(pair[1]));
		obj.add(std::string(pair[0].data() + 1, pair[0].size() - 2), valNode);
	}
	return obj;
}

JsonEncoder::JsonEncoder() {
	;
}

JsonEncoder::JsonEncoder(const Opts& opts) 
	: opts{opts}
{
	;
}

std::string JsonEncoder::encode(const Json& json) {
	ctx.reset();
	if (json.empty()) {
		return "";
	}
	std::string res;
	const Node& node = json.root.value();
	encodeImpl(res, node);
	return res;
}

void JsonEncoder::EncodingCtx::reset() {
	intendationLvl = 0;
}

void JsonEncoder::encodeImpl(std::string& res, const Node& node) {
	if (std::holds_alternative<ValNode>(node)) {
		const ValNode& vnode = std::get<ValNode>(node);
		switch (vnode.type()) {
		case NodeType::Null:
			encodeNull(res);
			break;
		case NodeType::Bool:
			encodeBool(res, vnode);
			break;
		case NodeType::Int:
			encodeInt(res, vnode);
			break;
		case NodeType::Float:
			encodeFloat(res, vnode);
			break;
		case NodeType::String:
			encodeStr(res, vnode);
			break;
		}
	}
	else if (std::holds_alternative<ArrNode>(node)) {
		if (opts.humanReadable) encodeArray<true>(res, std::get<ArrNode>(node));
		else encodeArray<false>(res, std::get<ArrNode>(node));
	}
	else if (std::holds_alternative<ObjNode>(node)) {
		if (opts.humanReadable) encodeObj<true>(res, std::get<ObjNode>(node));
		else encodeObj<false>(res, std::get<ObjNode>(node));
	}
	else {
		assert(false);
	}
}

void JsonEncoder::encodeNull(std::string& s) {
	s.append("null");
}

void JsonEncoder::encodeBool(std::string& s, const ValNode& node) {
	bool val = node.as<bool>();
	s.append(val ? "true" : "false");
}

void JsonEncoder::encodeInt(std::string& s, const ValNode& node) {
	s.append(std::to_string(node.as<int64_t>()));
}

void JsonEncoder::encodeFloat(std::string& s, const ValNode& node) {
	s.append(std::to_string(node.as<double>()));
}

void JsonEncoder::encodeStr(std::string& s, const ValNode& node) {
	s.append("\"").append(node.as<std::string>()).append("\"");
}

void JsonEncoder::appendIntendation(std::string& s) {
	for (size_t i = 0; i < ctx.intendationLvl; ++i) {
		s.append("    ");
	}
}

void JsonEncoder::dump(const Json& json, std::ostream& os) {
	os << encode(json) << std::endl;
}

void JsonEncoder::dump(const Json& json, std::ostream&& os) {
	os << encode(json) << std::endl;
}