
#include "Json.hpp"
#include <stdexcept>
#include <charconv>
#include <iostream>
#include <format>

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

Node::Type check::getType(std::string_view v) {
	if (v.empty()) return Node::Type::NoType;
	if (v.size() >= 2) {
		if (v.front() == '{' && v.back() == '}') {
			return Node::Type::Object;
		}
		else if (v.front() == '[' && v.back() == ']') {
			return Node::Type::Array;
		}
		else if (v.front() == '"' && v.back() == '"') {
			return Node::Type::String;
		}
	}
	if (v == "null") {
		return Node::Type::Null;
	}
	else if (v == "true" || v == "false") {
		return Node::Type::Bool;
	}
	else if (v.find_first_of(".") != v.npos) {
		return Node::Type::Float;
	}
	else {
		return Node::Type::Int;
	}
}

bool check::isValueType(Node::Type t) {
	return t == Node::Type::Bool || t == Node::Type::Null || t == Node::Type::Int || t == Node::Type::Float || t == Node::Type::String;
}

Node::~Node() {
	;
}

Node::Node(Type type)
	: type{ type }
{
	;
}

ValNode::ValNode(int64_t v)
	: Node{ Node::Type::Int }, val{ v }
{
	;
}

ValNode::ValNode(double d)
	: Node{ Node::Type::Float }, val{ d }
{

}
ValNode::ValNode(const std::string& s)
	: Node{ Node::Type::String }, val{ s }
{

}

ValNode::ValNode(bool b)
	: Node{Node::Type::Bool}, val{b}
{

}

ValNode::ValNode() 
	: Node{ Node::Type::Null }, val{Null()}
{
	
}

ArrNode::ArrNode()
	: Node{ Node::Type::Array }, val{}
{

}

ArrNode::ArrNode(std::vector<Node*>&& v)
	: Node{ Node::Type::Array }, val{ std::move(v) }
{

}

ArrNode::~ArrNode() {
	for (Node* node : val) {
		delete node;
	}
}

const auto& ArrNode::get() {
	return val;
}

Node* ArrNode::get(size_t idx) {
	return val.at(idx);
}

void ArrNode::add(Node* node) {
	val.push_back(node);
}

bool ArrNode::isMonotype() const {
	if (val.size() == 0) return true;
	Type t0 = val[0]->type;
	return std::all_of(val.cbegin(), val.cend(), [t0](const auto& v) {
		return v->type == t0;
		});
}

ObjNode::ObjNode()
	: Node{ Node::Type::Object }
{

}

ObjNode::ObjNode(std::unordered_map<std::string, Node*>&& m)
	: Node{ Node::Type::Object }, val{ std::move(m) } {

}

ObjNode::~ObjNode() {
	for (const auto& v : val) {
		delete v.second;
	}
}

const auto& ObjNode::get() {
	return val;
}

Node* ObjNode::get(const std::string& key) {
	return val.at(key);
}

void ObjNode::add(const std::string& key, Node* node) {
	val[key] = node;
}

std::vector<std::string> ObjNode::keys() {
	std::vector<std::string> res;
	for (const auto& [key, v] : val) {
		res.push_back(key);
	}
	return res;
}

Json::Json(Node* r)
	: root{ r }
{
	;
}

Json::Json() 
	: root{nullptr}
{

}

Json::~Json() {
	delete root;
}

bool Json::empty() const {
	return root == nullptr;
}

// getting keys of object nodes
std::vector<std::string> Json::keys(const std::string& key) {
	Node* node = get(key);
	return _keysImpl(node);
}

size_t Json::arrSize(const std::string& key) {
	Node* node = get(key);
	return _arrSizeImpl(node);
}

std::vector<std::string> Json::_keysImpl(Node* node) {
	if (!node) {
		throw std::out_of_range(std::format("Invalid keys"));
	}
	if (node->type != Node::Type::Object) {
		throw std::runtime_error(std::format("Invalid keys: node is not an object"));
	}
	if (auto ptr = dynamic_cast<ObjNode*>(node); ptr) {
		return ptr->keys();
	}
	else {
		assert(false);
	}
	return {};
}

size_t Json::_arrSizeImpl(Node* node) {
	if (!node) {
		throw std::out_of_range(std::format("Invalid keys"));
	}
	if (node->type != Node::Type::Array) {
		throw std::runtime_error(std::format("Invalid keys: node is not an array"));
	}
	if (auto ptr = dynamic_cast<ArrNode*>(node); ptr) {
		return ptr->size();
	}
	else {
		assert(false);
	}
	return {};
}

Node* Json::get(const std::string& key) {
	Node* node = _getImpl(util::split(key, "."));
	if (!node) {
		throw std::out_of_range(std::format("invalid keys: {}", key));
	}
	return node;
}

JsonDecoder::JsonDecoder() {

}

Json JsonDecoder::decode(std::string_view v) {
	return Json(decodeImpl(v));
}

Node* JsonDecoder::decodeImpl(std::string_view v) {
	if (v.empty()) return nullptr;
	std::string_view body = util::strip(v);
	Node::Type type = check::getType(v);
	switch (type) {
	case Node::Type::Object:
		return decodeObj(v);
	case Node::Type::Array:
		return decodeArr(v);
	case Node::Type::String:
		return decodeStr(v);
	case Node::Type::Int:
		return decodeInt(v);
	case Node::Type::Float:
		return decodeFloat(v);
	case Node::Type::Bool:
		return decodeBool(v);
	case Node::Type::Null:
		return decodeNull(v);
	default:
		throw std::logic_error("JSON: invalid node type");
	}
}

Node* JsonDecoder::decodeBool(std::string_view v) {
	if (v == "true") {
		return new ValNode((bool)(true));
	}
	else if (v == "false") {
		return new ValNode((bool)(false));
	}
	throw std::runtime_error("invalid bool node");
}

Node* JsonDecoder::decodeNull(std::string_view v) {
	if (v == "null") {
		return new ValNode();
	}
	throw std::runtime_error("invalid null node");
}

Node* JsonDecoder::decodeInt(std::string_view v) {
	int64_t val;
	if (std::from_chars(v.data(), v.data() + v.size(), val).ec != std::errc{}) {
		throw std::runtime_error("invalid int node");
	}
	return new ValNode(val);
}

Node* JsonDecoder::decodeFloat(std::string_view v) {
	double val;
	if (std::from_chars(v.data(), v.data() + v.size(), val).ec != std::errc{}) {
		throw std::runtime_error("invalid float node");
	}
	return new ValNode(val);
}

Node* JsonDecoder::decodeStr(std::string_view v) {
	if (!check::isStr(v)) {
		throw std::runtime_error("invalid string node");
	}
	return new ValNode(std::string(v.data() + 1, v.size() - 2));
}

Node* JsonDecoder::decodeArr(std::string_view v) {
	auto elems = util::smartSplit(util::strip(std::string_view{v.data() + 1, v.size() - 2}), ',');
	ArrNode* arr = new ArrNode();
	try {
		for (std::string_view elem : elems) {
			arr->add(decodeImpl(util::strip(elem)));
		}
	}
	catch (std::exception& ex) {
		delete arr;
		throw ex;
	}
	return arr;
}

Node* JsonDecoder::decodeObj(std::string_view v) {
	auto elems = util::smartSplit(util::strip(std::string_view{v.data() + 1, v.size() - 2}), ',');
	ObjNode* obj = new ObjNode();
	Node* valNode = nullptr;
	try {
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
			valNode = decodeImpl(util::strip(pair[1]));
			obj->add(std::string(pair[0].data() + 1, pair[0].size() - 2), valNode);
		}
	}
	catch (std::exception& ex) {
		delete obj;
		if (valNode) delete valNode;
		throw ex;
	}
	return obj;
}

JsonEncoder::JsonEncoder() {
	;
}

std::string JsonEncoder::encode(const Json& json) {
	if (json.empty()) {
		return "";
	}
	std::string res;
	Node* node = json.root;
	encodeImpl(res, node);
	return res;
}

void JsonEncoder::encodeImpl(std::string& res, Node* node) {
	switch (node->type) {
	case Node::Type::Null:
		encodeNull(res);
		break;
	case Node::Type::Bool:
		encodeBool(res, node);
		break;
	case Node::Type::Int:
		encodeInt(res, node);
		break;
	case Node::Type::Float:
		encodeFloat(res, node);
		break;
	case Node::Type::String:
		encodeStr(res, node);
		break;
	case Node::Type::Array:
		encodeArray(res, node);
		break;
	case Node::Type::Object:
		encodeObj(res, node);
		break;
	default:
		assert(false);
	}
}

void JsonEncoder::encodeNull(std::string& s) {
	s.append("null");
}

void JsonEncoder::encodeBool(std::string& s, Node* node) {
	if (auto ptr = dynamic_cast<ValNode*>(node); ptr) {
		bool val = ptr->as<bool>();
		s.append(val ? "true" : "false");
	}
	else {
		assert(false);
	}
}

void JsonEncoder::encodeInt(std::string& s, Node* node) {
	if (auto ptr = dynamic_cast<ValNode*>(node); ptr) {
		s.append(std::to_string(ptr->as<int64_t>()));
	}
	else {
		assert(false);
	}
}

void JsonEncoder::encodeFloat(std::string& s, Node* node) {
	if (auto ptr = dynamic_cast<ValNode*>(node); ptr) {
		s.append(std::to_string(ptr->as<double>()));
	}
	else {
		assert(false);
	}
}

void JsonEncoder::encodeStr(std::string& s, Node* node) {
	if (auto ptr = dynamic_cast<ValNode*>(node); ptr) {
		s.append("\"").append(ptr->as<std::string>()).append("\"");
	}
	else {
		assert(false);
	}
}

void JsonEncoder::encodeArray(std::string& s, Node* node) {
	if (auto ptr = dynamic_cast<ArrNode*>(node); ptr) {
		s.append("[");
		const auto& arrNodes = ptr->get();
		for (size_t i = 0; i < arrNodes.size(); ++i) {
			encodeImpl(s, arrNodes[i]);
			if (i < (arrNodes.size() - 1)) {
				s.append(",");
			}
		}
		s.append("]");
	}
	else {
		assert(false);
	}
}

void JsonEncoder::encodeObj(std::string& s, Node* node) {
	if (auto ptr = dynamic_cast<ObjNode*>(node); ptr) {
		s.append("{");
		const auto& objItems = ptr->get();
		size_t i = 0;
		for (const auto& [key, curNode] : objItems) {
			s.append(std::format("\"{}\":", key));
			encodeImpl(s, curNode);
			if (i < objItems.size() - 1) {
				s.append(",");
			}
			++i;
		}
		s.append("}");
	}
	else {
		assert(false);
	}
}