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
		{"4", obj}
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
}

