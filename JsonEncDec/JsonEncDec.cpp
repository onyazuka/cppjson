#include <iostream>
#include "Json.hpp"

using namespace std;
using namespace json;

using Term = std::variant<
	int64_t,
	double,
	std::string
>;

using Arr = std::vector<Term>;



int main()
{
	Node* i = new ValNode((int64_t)10);
	Node* s = new ValNode(std::string("neko"));
	Node* arr = new ArrNode({ new ValNode((int64_t)15), new ValNode((int64_t)20) , new ValNode((int64_t)25) });
	Node* obj = new ObjNode({
		{ "pim", new ValNode((double)3.2) },
		{ "bim", new ValNode(std::string("888"))},
		{"vim",
			new ArrNode({
				new ValNode((int64_t)1), new ValNode((int64_t)2) , new ValNode((int64_t)3)
			})
		} });

	Node* b = new ValNode((bool)true);
	Node* n = new ValNode();

	/*
		{
			"1": 10,
			"2": "neko",
			"3": [15, 20, 25],
			"4": {
					"pim": 3.2,
					"bim": "888",
					"vim": [1,2,3]
				}
		}
	*/

	Node* root = new ObjNode({
		{"1", i},
		{"2", s},
		{"3", arr},
		{"4", obj},
		{"5", b},
		{"6", n}
		});

	Json json(root);

	cout << json.as<double>("4.pim") << endl;

	auto v = json.as<std::vector<int64_t>>("4.vim");
	for (auto val : v) {
		cout << val << endl;
	}

	cout << json.as<int64_t>("4.vim.[1]") << endl;
	cout << "-------\n";
	cout << json.as<int64_t>(std::vector<std::string_view>{ "4", "vim", "[1]" }) << endl;
	cout << json.as<int64_t>(std::vector<std::string>{ "4", "vim", "[1]" }) << endl;
	cout << json.as<bool>("5") << endl;
	
	auto vvv = json.as<Null>("6");

	std::string str1 = "1,2,3";
	std::string str2 = "\"wan,ko\",1,\"ne,ko\",2,\",ko\",3,\"ko,\",4,5";
	std::string str3 = "";
	std::string str4 = "\"ne,ko\"";
	std::string str5 = ",";

	auto v1 = util::smartSplit(str1, ',');
	auto v2 = util::smartSplit(str2, ',');
	auto v3 = util::smartSplit(str3, ',');
	auto v4 = util::smartSplit(str4, ',');
	auto v5 = util::smartSplit(str5, ',');

	return 0;
}

