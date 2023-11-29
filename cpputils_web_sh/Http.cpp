#include "Http.hpp"
#include "Utils_String.hpp"
#include <format>

using namespace util::web::http;

HttpRequest::HttpRequest() {
	;
}

HttpRequest::HttpRequest(Method method, const std::string& url, const std::string& version, const std::unordered_map<std::string, std::string>& headers, const std::string& body)
	: method{method}, url{url}, version{version}, headers{headers}, body{body}
{
	;
}

std::string HttpRequest::encode() const {
	std::string shttp;
	shttp.append(std::format("{} {} {}\r\n", methodToStr(method), url, version));
	for (const auto& header : headers) {
		shttp.append(std::format("{}:{}\r\n", header.first, header.second));
	}
	shttp.append("\r\n");
	shttp.append(body);
	return shttp;
}

HttpResponse::HttpResponse() {
	;
}

HttpResponse::HttpResponse(const std::string& version, size_t status, const std::string& statusText, const std::unordered_map<std::string, std::string>& headers, const std::string& body) 
	: version{version}, status{status}, statusText{statusText}, headers{headers}, body{body}
{
	;
}

std::string HttpResponse::encode() const {
	std::string shttp;
	shttp.append(std::format("{} {} {}\r\n", version, std::to_string(status), statusText));
	for (const auto& header : headers) {
		shttp.append(std::format("{}:{}\r\n", header.first, header.second));
	}
	shttp.append("\r\n");
	shttp.append(body);
	return shttp;
}

std::string util::web::http::methodToStr(Method m) {
	switch (m) {
	case Method::OPTIONS:
		return "OPTIONS";
	case Method::GET:
		return "GET";
	case Method::HEAD:
		return "HEAD";
	case Method::PUT:
		return "PUT";
	case Method::POST:
		return "POST";
	case Method::DELETE:
		return "DELETE";
	case Method::PATCH:
		return "PATCH";
	default:
		assert(false);
	}
	return "";
}