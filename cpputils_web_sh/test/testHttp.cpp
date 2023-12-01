#include <iostream>
#include "testHttp.hpp"

using namespace std;
using namespace util::web::http::test;

void util::web::http::test::testHttpMain() {
    cout << "-------------------------TESTING HTTP---------------------------\n";
    std::string r1 = "GET /hello.htm HTTP/1.1\nUser-Agent: Mozilla / 4.0 (compatible; MSIE5.01; Windows NT)\nHost : www.tutorialspoint.com\nAccept-Language : en - us\nAccept-Encoding : gzip, deflate\nConnection : Keep-Alive";
    HttpParser<HttpRequest> hp1(r1);
    std::string resp1 = "HTTP/1.1 200 OK\r\nDate: Mon, 27 Jul 2009 12 : 28 : 53 GMT\r\nServer : Apache / 2.2.14 (Win32)\r\nLast-Modified : Wed, 22 Jul 2009 19 : 15 : 56 GMT\r\nContent-Length : 62\r\nContent-Type : text/html\r\nConnection : Closed\r\n\r\n<html><body>\r\n\r\n<h1>Hello, World!</h1>\r\n\r\n</body>\r\n</html>\r\n\r\n";
    HttpParser<HttpResponse> hp2(resp1);
    std::string r2 = "GET /hello.htm HTTP/1.1";
    HttpParser<HttpRequest> hp3(r2);

    hp1.headers().add("USER-AGENT", "ME");
    hp1.message().url = "/baka";
    hp1.body() = "<h1>Hello world</h1>";

    cout << hp1.message().encode() << endl << "-----------------" << endl;
    cout << hp2.message().encode() << endl << "-----------------" << endl;
    cout << hp3.message().encode() << endl << "-----------------" << endl;
}