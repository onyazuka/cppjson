#include "Http.hpp"
#include "Utils_String.hpp"
#include <format>

using namespace util::web::http;

std::string util::web::http::HttpStatusToStr(size_t code)
{
	switch (code)
	{

		//####### 1xx - Informational #######
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 102: return "Processing";
	case 103: return "Early Hints";

		//####### 2xx - Successful #######
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 207: return "Multi-Status";
	case 208: return "Already Reported";
	case 226: return "IM Used";

		//####### 3xx - Redirection #######
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Found";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 305: return "Use Proxy";
	case 307: return "Temporary Redirect";
	case 308: return "Permanent Redirect";

		//####### 4xx - Client Error #######
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Timeout";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Required";
	case 412: return "Precondition Failed";
	case 413: return "Content Too Large";
	case 414: return "URI Too Long";
	case 415: return "Unsupported Media Type";
	case 416: return "Range Not Satisfiable";
	case 417: return "Expectation Failed";
	case 418: return "I'm a teapot";
	case 421: return "Misdirected Request";
	case 422: return "Unprocessable Content";
	case 423: return "Locked";
	case 424: return "Failed Dependency";
	case 425: return "Too Early";
	case 426: return "Upgrade Required";
	case 428: return "Precondition Required";
	case 429: return "Too Many Requests";
	case 431: return "Request Header Fields Too Large";
	case 451: return "Unavailable For Legal Reasons";

		//####### 5xx - Server Error #######
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Timeout";
	case 505: return "HTTP Version Not Supported";
	case 506: return "Variant Also Negotiates";
	case 507: return "Insufficient Storage";
	case 508: return "Loop Detected";
	case 510: return "Not Extended";
	case 511: return "Network Authentication Required";

	default: return std::string();
	}
}

HttpHeaders::HttpHeaders()
{
	;
}

HttpHeaders::HttpHeaders(const std::unordered_map<std::string, std::string>& _map) 
	: headers{_map}
{
	;
}

HttpHeaders::HttpHeaders(std::unordered_map<std::string, std::string>&& _map) 
	: headers{ std::move(_map) }
{
	;
}

std::string HttpHeaders::find(const std::string& key) const {
	if (auto iter = headers.find(key); iter != headers.end()) {
		return iter->second;
	}
	return "";
}

void HttpHeaders::add(const std::string& key, const std::string& val) {
	headers[key] = val;
}

void HttpHeaders::remove(const std::string& key) {
	headers.erase(key);
}

void HttpHeaders::borrow(const HttpHeaders& other, const std::string& key, const std::string& defVal) {
	if (auto iter = other.headers.find(key); iter != other.headers.end()) {
		add(key, iter->second);
	}
	else {
		if (!defVal.empty()) add(key, defVal);
	}
}

void HttpHeaders::clear() {
	headers.clear();
}

std::unordered_map<std::string, std::string> HttpHeaders::cookies() const {
	std::unordered_map<std::string, std::string> res;
	if (auto iter = headers.find("Cookie"); iter != headers.end()) {
		auto parts = util::string::split(util::string::strip(iter->second), ";");
		for (auto part : parts) {
			auto subparts = util::string::split(util::string::strip(part), "=");
			if (subparts.size() != 2) {
				continue;
			}
			res[util::string::v2str(subparts[0])] = util::string::v2str(subparts[1]);
		}
	}
	return res;
}

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
	for (const auto& header : headers.get()) {
		shttp.append(std::format("{}:{}\r\n", header.first, header.second));
	}
	shttp.append("\r\n");
	shttp.append(body);
	return shttp;
}

HttpResponse::HttpResponse() {
	;
}

HttpResponse::HttpResponse(const std::string& version, size_t status, const std::unordered_map<std::string, std::string>& headers, const std::string& body, const HttpHeaders& reqHeaders)
	: version{version}, status{status}, statusText{ HttpStatusToStr(status) }, headers{headers}, body{body}
{
	finish(reqHeaders);
}

HttpResponse::HttpResponse(std::string&& version, size_t status, std::unordered_map<std::string, std::string>&& headers, std::string&& body, HttpHeaders&& reqHeaders)
	: version{ std::move(version) }, status{ status }, statusText{ HttpStatusToStr(status) }, headers{ std::move(headers) }, body{ std::move(body) }
{
	finish(reqHeaders);
}

HttpResponse::HttpResponse(size_t status, const std::unordered_map<std::string, std::string>& headers, const std::string& body, const HttpHeaders& reqHeaders)
	: version{ Default_Http_Version }, status{ status }, statusText{ HttpStatusToStr(status)}, headers{headers}, body{body}
{
	finish(reqHeaders);
}

HttpResponse::HttpResponse(size_t status, std::unordered_map<std::string, std::string>&& headers, std::string&& body, HttpHeaders&& reqHeaders)
	: version{ Default_Http_Version }, status{ status }, statusText{ HttpStatusToStr(status)}, headers{std::move(headers)}, body{std::move(body)}
{
	finish(reqHeaders);
}

HttpResponse::HttpResponse(const std::string& version, size_t status, const HttpHeaders& headers, const std::string& body, const HttpHeaders& reqHeaders)
	: version{ version }, status{ status }, statusText{ HttpStatusToStr(status) }, headers{ headers }, body{ body }
{
	finish(reqHeaders);
}

HttpResponse::HttpResponse(std::string&& version, size_t status, HttpHeaders&& headers, std::string&& body, HttpHeaders&& reqHeaders)
	: version{ std::move(version) }, status{ status }, statusText{ HttpStatusToStr(status) }, headers{ std::move(headers) }, body{ std::move(body) }
{
	finish(reqHeaders);
}

HttpResponse::HttpResponse(size_t status, const HttpHeaders& headers, const std::string& body, const HttpHeaders& reqHeaders)
	: version{ Default_Http_Version }, status{ status }, statusText{ HttpStatusToStr(status) }, headers{ headers }, body{ body }
{
	finish(reqHeaders);
}

HttpResponse::HttpResponse(size_t status, HttpHeaders&& headers, std::string&& body, HttpHeaders&& reqHeaders)
	: version{ Default_Http_Version }, status{ status }, statusText{ HttpStatusToStr(status) }, headers{ std::move(headers) }, body{ std::move(body) }
{
	finish(reqHeaders);
}

void HttpResponse::finish(const HttpHeaders& reqHeaders) {
	headers.add("Content-Length", body.size());
	headers.borrow(reqHeaders, "Connection");
}

std::string HttpResponse::encode() const {
	std::string shttp;
	shttp.append(std::format("{} {} {}\r\n", version, std::to_string(status), statusText));
	for (const auto& header : headers.get()) {
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
	case Method::TRACE:
		return "TRACE";
	case Method::CONNECT:
		return "CONNECT";
	default:
		assert(false);
	}
	return "";
}