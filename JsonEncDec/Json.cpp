
#include "Json.hpp"

using namespace json;

std::string_view util::strip(std::string_view v) {
	static constexpr char Spaces[] = "\t\n\r ";
	auto p1 = v.find_first_not_of(Spaces);
	auto p2 = v.find_last_not_of(Spaces);
	if (p1 == std::string_view::npos) {
		return {};
	}
	return v.substr(p1, p2);
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

bool ArrNode::isMonotype() const {
	if (val.size() == 0) return true;
	Type t0 = val[0]->type;
	return std::all_of(val.cbegin(), val.cend(), [t0](const auto& v) {
		return v->type == t0;
		});
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

Json::Json(Node* r)
	: root{ r }
{
	;
}

Json::~Json() {
	delete root;
}

Node* Json::get(const std::string& key) {
	return _getImpl(util::split(key, "."));
}