#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <variant>
#include <type_traits>
#include <cassert>
#include "Utils_String.hpp"

namespace util::web::http {

	enum class Method {
		OPTIONS,
		GET,
		HEAD,
		PUT,
		POST,
		DELETE,
		PATCH
	};

	std::string methodToStr(Method m);

	// value(std::string) specification is selected by default
	template<typename Str>
		requires (std::is_same_v<Str, std::string> || std::is_same_v<Str, std::string&>)
	Method strToMethod(Str s) {
		std::transform(s.begin(), s.end(), s.begin(), std::toupper);
		if (s == "OPTIONS") {
			return Method::OPTIONS;
		}
		else if (s == "GET") {
			return Method::GET;
		}
		else if (s == "HEAD") {
			return Method::HEAD;
		}
		else if (s == "PUT") {
			return Method::PUT;
		}
		else if (s == "POST") {
			return Method::POST;
		}
		else if (s == "DELETE") {
			return Method::DELETE;
		}
		else if (s == "PATCH") {
			return Method::PATCH;
		}
		else {
			assert(false);
		}
		return Method::GET;
	}

	struct HttpRequest {
		Method method = Method::GET;
		std::string url;
		std::string version;
	};

	struct HttpResponse {
		std::string version;
		size_t status = 0;
		std::string statusText;
	};

	template<typename T>
	concept CHttpMessage = std::is_same_v<T, HttpRequest> || std::is_same_v<T, HttpResponse>;

	template<CHttpMessage HttpMessage>
	class HttpParser {
	public:
		HttpParser(const std::string& s);
		HttpMessage& message();
		const HttpMessage& message() const;

		inline const auto& headers() const { return _headers; }
		inline auto& headers() { return _headers; }
		inline const auto& body() const { return _body; }
		inline auto& body() { return _body; }

	private:
		bool parse(std::string_view s);
		bool parseFirstLine(std::string_view s);
		bool parseHeader(std::string_view line);

		HttpMessage _msg;
		std::unordered_map<std::string, std::string> _headers;
		std::string _body;
	};

	template<CHttpMessage HttpMessage>
	HttpParser<HttpMessage>::HttpParser(const std::string& s) {
		if (s.empty()) {
			throw std::invalid_argument("http: empty string has been passed");
		}
		if (!parse(s)) {
			throw std::invalid_argument("http: invalid input sring");
		}
	}

	template<CHttpMessage HttpMessage>
	HttpMessage& HttpParser<HttpMessage>::message() {
		return _msg;
	}

	template<CHttpMessage HttpMessage>
	const HttpMessage& HttpParser<HttpMessage>::message() const {
		return _msg;
	}

	template<CHttpMessage HttpMessage>
	bool HttpParser<HttpMessage>::parse(std::string_view s) {
		using namespace util::string;
		Splitter splitter(s, "\n");
		bool firstParsed = false;
		bool emptyFound = false;
		for (auto line : splitter) {
			auto sline = strip(line);
			// empty line
			if (sline.empty()) {
				emptyFound = true;
				continue;
			}
			// first line
			else if (!firstParsed) {
				if (!parseFirstLine(sline)) {
					return false;
				}
				firstParsed = true;
			}
			// body
			else if (emptyFound) {
				_body = std::string(line.data(), s.data() + s.size());
				break;
			}
			// header
			else {
				if (!parseHeader(sline)) {
					return false;
				}
			}
		}
		return true;
	}

	template<CHttpMessage HttpMessage>
	bool HttpParser<HttpMessage>::parseFirstLine(std::string_view s) {
		using namespace util::string;
		auto parts = split(s, " ");
		if (parts.size() != 3) {
			return false;
		}
		if constexpr (std::is_same_v<HttpMessage, HttpRequest>) {
			std::string method = v2str(parts[0]);
			std::transform(method.begin(), method.end(), method.begin(), std::toupper);
			_msg = HttpRequest{ strToMethod(method), v2str(parts[1]), v2str(parts[2]) };
		}
		// response
		else {
			_msg = HttpResponse(v2str(parts[0]), std::stoull(v2str(parts[1])), v2str(parts[2]));
		}
		return true;
	}

	// returns idx of empty line
	template<CHttpMessage HttpMessage>
	bool HttpParser<HttpMessage>::parseHeader(std::string_view line) {
		using namespace util::string;
		// not using split, because it can be ':' chars in value
		size_t pos = line.find(":");
		if (pos == line.npos) return false;
		std::string_view vkey = line.substr(0, pos);
		std::string_view vval = line.substr(pos + 1);
		std::string key = v2str(strip(vkey));
		std::transform(key.begin(), key.end(), key.begin(), std::toupper);
		_headers[std::move(key)] = v2str(strip(vval));
		return true;
	}
}