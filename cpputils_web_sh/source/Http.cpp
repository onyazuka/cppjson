#include "../include/Http.hpp"
#include "Utils_String.hpp"

using namespace util::web::http;

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