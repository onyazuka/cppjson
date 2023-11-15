#include <iostream>
#include <format>
#include <chrono>
#include <fstream>
#include "test/testJson.hpp"
#include "test/testHttp.hpp"


using namespace util::web::json::test;
using namespace util::web::http::test;

int main()
{
	testJsonMain();
	testHttpMain();
	return 0;
}

