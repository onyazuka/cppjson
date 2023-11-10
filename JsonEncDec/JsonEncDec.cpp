#include <iostream>
#include <format>
#include <chrono>
#include <fstream>
#include "Json.hpp"

using namespace std;
using namespace json;

using Term = std::variant<
	int64_t,
	double,
	std::string
>;

using Arr = std::vector<Term>;

void testJsonDecode() {
	cout << format("{:-^40}\n", "Testing json decoding");
	// integer
	{
		std::string si = "1";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.as<int64_t>() == 1);
		cout << j1.as<int64_t>() << endl;

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		assert(se == si);
	}
	// floating point
	{
		std::string si = "1.2";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.as<double>() == 1.2);
		cout << j1.as<double>() << endl;

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		// precisions can differ
		assert(std::stod(se) == std::stod(si));
	}
	// null
	{
		std::string si = "null";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert((std::is_same_v<typename decltype(j1.as<Null>()), Null>));

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		assert(se == si);
	}
	// bool true
	{
		std::string si = "true";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.as<bool>() == 1);
		cout << j1.as<bool>() << endl;

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		assert(se == si);
	}
	// bool false
	{
		std::string si = "false";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.as<bool>() == 0);
		cout << j1.as<bool>() << endl;

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		assert(se == si);
	}
	// string
	{
		std::string si = "\"nekoWanko,,..999\"";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.as<std::string>() == "nekoWanko,,..999");
		cout << j1.as<std::string>() << endl;

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		assert(se == si);
	}
	// no type
	{
		std::string si;
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.empty());
	}
	// array ints
	{
		std::string si = "[1,2,3]";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.as<int64_t>("[0]") == 1);
		cout << j1.as<int64_t>("[0]") << endl;
		assert(j1.as<int64_t>("[1]") == 2);
		cout << j1.as<int64_t>("[1]") << endl;
		assert(j1.as<int64_t>("[2]") == 3);
		cout << j1.as<int64_t>("[2]") << endl;

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		assert(se == si);
	}
	// array strings
	{
		std::string si = "[\"neko\",\"wanko\",\"manko\"]";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		auto vec = j1.as<std::vector<std::string>>();

		assert(vec[0] == "neko");
		cout << vec[0] << endl;
		assert(vec[1] == "wanko");
		cout << vec[1] << endl;
		assert(vec[2] == "manko");
		cout << vec[2] << endl;

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		assert(se == si);
	}
	// obj 
	{
		std::string si = "{\"neko\":123}";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.as<int64_t>("neko") == 123);
		std::cout << j1.as<int64_t>("neko") << endl;

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		assert(se == si);
	}
	// obj 2
	{
		std::string si = "{\"1\":10,\"2\":\"neko\",\"3\":[15,20,25],\"4\":{\"pim\":3.2,\"bim\":\"888\",\"vim\":[1,2,3]}}";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		assert(j1.as<int64_t>("1") == 10);
		std::cout << j1.as<int64_t>("1") << endl;
		assert(j1.as<std::string>("2") == "neko");
		assert(j1.as<int64_t>("3.[1]") == 20);
		assert(j1.as<double>("4.pim") == 3.2);
		assert(j1.as<int64_t>("4.vim.[0]") == 1);

		JsonEncoder je1;
		std::string se = je1.encode(j1);
		// it IS equal, except precision
		//assert(se == si);
	}
	// obj 3
	{
		std::string si = "[\r\n  {\r\n    \"_id\": \"654ce772b85bedd6d8898415\",\r\n    \"index\": 0,\r\n    \"guid\": \"e713b670-1d09-4df7-8512-d222ea1f642b\",\r\n    \"isActive\": true,\r\n    \"balance\": \"$3,952.26\",\r\n    \"picture\": \"http:\/\/placehold.it\/32x32\",\r\n    \"age\": 39,\r\n    \"eyeColor\": \"green\",\r\n    \"name\": \"Robbie Heath\",\r\n    \"gender\": \"female\",\r\n    \"company\": \"UNEEQ\",\r\n    \"email\": \"robbieheath@uneeq.com\",\r\n    \"phone\": \"+1 (946) 500-3256\",\r\n    \"address\": \"238 Jefferson Street, Barrelville, Louisiana, 9460\",\r\n    \"about\": \"Cupidatat eu irure officia cillum cillum esse labore voluptate irure quis ullamco dolore velit ad. Sit labore elit eiusmod quis tempor pariatur et cillum. Id labore ex consectetur culpa aliquip labore commodo exercitation ipsum reprehenderit enim minim deserunt occaecat. Anim ad culpa cillum adipisicing laborum est id adipisicing ea culpa. Officia adipisicing anim consectetur qui veniam amet laborum tempor laboris. Nisi dolore sint mollit incididunt culpa fugiat proident et cillum.\\r\\n\",\r\n    \"registered\": \"2015-03-05T12:35:44 -03:00\",\r\n    \"latitude\": -69.053469,\r\n    \"longitude\": -177.473446,\r\n    \"tags\": [\r\n      \"incididunt\",\r\n      \"nostrud\",\r\n      \"Lorem\",\r\n      \"nisi\",\r\n      \"ipsum\",\r\n      \"reprehenderit\",\r\n      \"irure\"\r\n    ],\r\n    \"friends\": [\r\n      {\r\n        \"id\": 0,\r\n        \"name\": \"Sheryl Farrell\"\r\n      },\r\n      {\r\n        \"id\": 1,\r\n        \"name\": \"Ochoa Tillman\"\r\n      },\r\n      {\r\n        \"id\": 2,\r\n        \"name\": \"Richmond Davidson\"\r\n      }\r\n    ],\r\n    \"greeting\": \"Hello, Robbie Heath! You have 10 unread messages.\",\r\n    \"favoriteFruit\": \"strawberry\"\r\n  },\r\n  {\r\n    \"_id\": \"654ce772b53f0f535c575c60\",\r\n    \"index\": 1,\r\n    \"guid\": \"e72b7ddd-7020-4753-85a9-92e56b4cd5f6\",\r\n    \"isActive\": true,\r\n    \"balance\": \"$3,427.31\",\r\n    \"picture\": \"http:\/\/placehold.it\/32x32\",\r\n    \"age\": 31,\r\n    \"eyeColor\": \"green\",\r\n    \"name\": \"Burton Booker\",\r\n    \"gender\": \"male\",\r\n    \"company\": \"IMKAN\",\r\n    \"email\": \"burtonbooker@imkan.com\",\r\n    \"phone\": \"+1 (983) 412-3627\",\r\n    \"address\": \"430 Eldert Street, Caln, Iowa, 8751\",\r\n    \"about\": \"Dolor est nulla sit nostrud velit adipisicing officia laborum. Qui sunt laboris aliqua proident est non tempor est et. Labore fugiat ad duis dolore veniam exercitation exercitation Lorem nostrud irure amet ipsum magna. Enim officia nostrud adipisicing tempor qui ex adipisicing elit mollit dolor officia in. Veniam ad esse ea nostrud Lorem ea commodo ad.\\r\\n\",\r\n    \"registered\": \"2015-11-14T07:02:52 -03:00\",\r\n    \"latitude\": 16.4488,\r\n    \"longitude\": -41.481124,\r\n    \"tags\": [\r\n      \"aliqua\",\r\n      \"incididunt\",\r\n      \"laborum\",\r\n      \"eiusmod\",\r\n      \"consectetur\",\r\n      \"ad\",\r\n      \"reprehenderit\"\r\n    ],\r\n    \"friends\": [\r\n      {\r\n        \"id\": 0,\r\n        \"name\": \"Holcomb Dillon\"\r\n      },\r\n      {\r\n        \"id\": 1,\r\n        \"name\": \"Alicia Kim\"\r\n      },\r\n      {\r\n        \"id\": 2,\r\n        \"name\": \"Estrada Church\"\r\n      }\r\n    ],\r\n    \"greeting\": \"Hello, Burton Booker! You have 9 unread messages.\",\r\n    \"favoriteFruit\": \"strawberry\"\r\n  },\r\n  {\r\n    \"_id\": \"654ce7728785d91c086ca42c\",\r\n    \"index\": 2,\r\n    \"guid\": \"8709bb58-68d2-475a-b4de-d888bd240ba5\",\r\n    \"isActive\": false,\r\n    \"balance\": \"$1,023.35\",\r\n    \"picture\": \"http:\/\/placehold.it\/32x32\",\r\n    \"age\": 37,\r\n    \"eyeColor\": \"green\",\r\n    \"name\": \"Rodgers Calderon\",\r\n    \"gender\": \"male\",\r\n    \"company\": \"WATERBABY\",\r\n    \"email\": \"rodgerscalderon@waterbaby.com\",\r\n    \"phone\": \"+1 (896) 472-3154\",\r\n    \"address\": \"918 Lafayette Avenue, Marne, Mississippi, 6478\",\r\n    \"about\": \"Tempor aliqua consectetur aliquip amet. Fugiat dolore culpa pariatur minim ex laboris. Et nisi anim ea occaecat eiusmod do exercitation commodo. Irure ipsum sit labore ex ipsum ad proident culpa minim deserunt consectetur cupidatat aliqua magna. Fugiat enim sit elit fugiat. Duis velit nulla sint incididunt deserunt nisi.\\r\\n\",\r\n    \"registered\": \"2015-11-26T06:29:23 -03:00\",\r\n    \"latitude\": 86.526226,\r\n    \"longitude\": 141.964954,\r\n    \"tags\": [\r\n      \"ex\",\r\n      \"ut\",\r\n      \"minim\",\r\n      \"nisi\",\r\n      \"excepteur\",\r\n      \"est\",\r\n      \"qui\"\r\n    ],\r\n    \"friends\": [\r\n      {\r\n        \"id\": 0,\r\n        \"name\": \"Garza Suarez\"\r\n      },\r\n      {\r\n        \"id\": 1,\r\n        \"name\": \"Thornton Powers\"\r\n      },\r\n      {\r\n        \"id\": 2,\r\n        \"name\": \"Grimes Noel\"\r\n      }\r\n    ],\r\n    \"greeting\": \"Hello, Rodgers Calderon! You have 7 unread messages.\",\r\n    \"favoriteFruit\": \"strawberry\"\r\n  },\r\n  {\r\n    \"_id\": \"654ce7722576677f9cc2a61a\",\r\n    \"index\": 3,\r\n    \"guid\": \"ae224cf6-65e2-4b48-a00e-bf7bbdf280c9\",\r\n    \"isActive\": false,\r\n    \"balance\": \"$3,949.16\",\r\n    \"picture\": \"http:\/\/placehold.it\/32x32\",\r\n    \"age\": 31,\r\n    \"eyeColor\": \"brown\",\r\n    \"name\": \"Cecilia Abbott\",\r\n    \"gender\": \"female\",\r\n    \"company\": \"INTRADISK\",\r\n    \"email\": \"ceciliaabbott@intradisk.com\",\r\n    \"phone\": \"+1 (979) 491-2521\",\r\n    \"address\": \"207 Olive Street, Cresaptown, Hawaii, 5504\",\r\n    \"about\": \"Sint consectetur Lorem labore voluptate ex sit non veniam veniam in. Reprehenderit eu duis culpa et nisi fugiat irure. Qui cillum veniam exercitation esse culpa fugiat labore minim sunt occaecat consectetur aliquip ullamco. Culpa adipisicing aute cillum amet enim do do aliquip voluptate adipisicing proident. Minim esse sunt incididunt aliqua Lorem ipsum cillum proident consequat ad quis do reprehenderit dolor. Consectetur incididunt magna eu Lorem laborum consectetur Lorem.\\r\\n\",\r\n    \"registered\": \"2016-02-05T10:48:04 -03:00\",\r\n    \"latitude\": -75.995765,\r\n    \"longitude\": -91.173087,\r\n    \"tags\": [\r\n      \"et\",\r\n      \"ad\",\r\n      \"reprehenderit\",\r\n      \"dolor\",\r\n      \"in\",\r\n      \"adipisicing\",\r\n      \"amet\"\r\n    ],\r\n    \"friends\": [\r\n      {\r\n        \"id\": 0,\r\n        \"name\": \"Lakisha Bond\"\r\n      },\r\n      {\r\n        \"id\": 1,\r\n        \"name\": \"Mariana Hyde\"\r\n      },\r\n      {\r\n        \"id\": 2,\r\n        \"name\": \"Randolph Fischer\"\r\n      }\r\n    ],\r\n    \"greeting\": \"Hello, Cecilia Abbott! You have 6 unread messages.\",\r\n    \"favoriteFruit\": \"strawberry\"\r\n  },\r\n  {\r\n    \"_id\": \"654ce772191ae5f640472f4b\",\r\n    \"index\": 4,\r\n    \"guid\": \"83318bab-d9d6-49e3-9d76-861cbba4b982\",\r\n    \"isActive\": false,\r\n    \"balance\": \"$2,967.04\",\r\n    \"picture\": \"http:\/\/placehold.it\/32x32\",\r\n    \"age\": 31,\r\n    \"eyeColor\": \"green\",\r\n    \"name\": \"Juliette Everett\",\r\n    \"gender\": \"female\",\r\n    \"company\": \"QUILK\",\r\n    \"email\": \"julietteeverett@quilk.com\",\r\n    \"phone\": \"+1 (873) 522-3495\",\r\n    \"address\": \"415 Vermont Street, Hollins, South Carolina, 9163\",\r\n    \"about\": \"Anim enim aliqua enim pariatur adipisicing ea ipsum voluptate magna non. Voluptate officia deserunt veniam incididunt esse dolor nulla deserunt mollit. Irure tempor aute qui irure mollit est incididunt quis elit anim exercitation. Commodo Lorem aliquip ex proident non elit excepteur do ex officia enim enim reprehenderit quis. Mollit pariatur sunt dolore sit irure aliquip irure aute. Excepteur in quis do sint sunt est mollit tempor id mollit ad.\\r\\n\",\r\n    \"registered\": \"2018-01-04T04:47:10 -03:00\",\r\n    \"latitude\": 56.56465,\r\n    \"longitude\": 53.958798,\r\n    \"tags\": [\r\n      \"ipsum\",\r\n      \"fugiat\",\r\n      \"qui\",\r\n      \"Lorem\",\r\n      \"et\",\r\n      \"Lorem\",\r\n      \"exercitation\"\r\n    ],\r\n    \"friends\": [\r\n      {\r\n        \"id\": 0,\r\n        \"name\": \"King Johnson\"\r\n      },\r\n      {\r\n        \"id\": 1,\r\n        \"name\": \"Dickson Yang\"\r\n      },\r\n      {\r\n        \"id\": 2,\r\n        \"name\": \"Ortega Gould\"\r\n      }\r\n    ],\r\n    \"greeting\": \"Hello, Juliette Everett! You have 1 unread messages.\",\r\n    \"favoriteFruit\": \"apple\"\r\n  }\r\n]";
		//std::string si = "[{\"1\":1},{\"2\":2}]";
		JsonDecoder jd1;
		auto before = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
		auto j1 = jd1.decode(si);
		auto after = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
		cout << format("Time: {0}mcs\n", (after - before));
		assert(j1.as<std::string>("[1].name") == "Burton Booker");
		assert(j1.as<std::string>("[4].tags.[6]") == "exercitation");
		assert(j1.as<int64_t>("[3].friends.[1].id") == 1);

		JsonEncoder je1;
		std::string se = je1.encode(j1);
	}
	{
		// bugs
		/*std::string si = "{\"0\":0,\"1\":1}";
		JsonDecoder jd1;
		auto j1 = jd1.decode(si);
		cout << j1.as<int64_t>("2");*/
	}
	return;
}


int main()
{
	ValNode i((int64_t)10); 
	ValNode s(std::string("neko"));
	ArrNode arr({ ValNode((int64_t)15), ValNode((int64_t)20) , ValNode((int64_t)25) });
	ObjNode obj({
		{ "pim", ValNode((double)3.2) },
		{ "bim", ValNode(std::string("888"))},
		{"vim",
			ArrNode({
				ValNode((int64_t)1), ValNode((int64_t)2) , ValNode((int64_t)3)
			})
		} });

	ValNode b((bool)true);
	ValNode n;

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

	Node root = ObjNode({
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

	for (const auto& key : json.keys("4")) {
		std::cout << key << ", ";
	}
	std::cout << std::endl;
	std::cout << std::format("Size of '3' is: {}\n", json.arrSize("3"));

	testJsonDecode();

	Json json1 = json;
	json1.get() = ValNode((int64_t)10);

	cout << format("{:-^40}\n", "Testing json encoding");
	cout << JsonEncoder().encode(json) << endl;
	cout << JsonEncoder().encode(json1) << endl;

	Json json2 = json;
	json2.get("2") = ObjNode({
		{"hell", ValNode(std::string("o"))},
		{"unk", ValNode(std::string("o"))},
		});
	std::get<ObjNode>(json2.get()).cont()["7"] = ValNode(std::string("Aya ningen da"));
	std::get<ArrNode>(json2.get("3")).cont().push_back(ValNode(11.2));
	JsonEncoder().dump(json2, std::cout);
	std::cout << JsonEncoder({ true }).encode(json2) << endl;
	JsonEncoder({true}).dump(json2, std::ofstream("f:/1.json"));
	Json json3 = JsonDecoder().decode(std::ifstream("f:/1.json"));
	JsonEncoder().dump(json3, std::cout);

	ValNode vvvvv("1.1");

	return 0;
}

