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
#include <format>
#include "Utils_String.hpp"

namespace util::web::http {

	std::string HttpStatusToStr(size_t code);

	enum class Method {
		OPTIONS,
		GET,
		HEAD,
		PUT,
		POST,
		DELETE,
		PATCH,
		CONNECT,
		TRACE
	};

	std::string methodToStr(Method m);

	// value(std::string) specification is selected by default
	template<typename Str>
		requires (std::is_same_v<Str, std::string> || std::is_same_v<Str, std::string&>)
	Method strToMethod(Str s) {
		std::transform(s.begin(), s.end(), s.begin(), ::toupper);
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
		else if (s == "CONNECT") {
			return Method::CONNECT;
		}
		else if (s == "TRACE") {
			return Method::TRACE;
		}
		else {
			assert(false);
		}
		return Method::GET;
	}

	template<typename T>
	concept Formattable = requires (T) {
		typename std::formatter<T>;
	};

	class HttpHeaders {
	public:
		HttpHeaders();
		HttpHeaders(const std::unordered_map<std::string, std::string>& _map);
		HttpHeaders(std::unordered_map<std::string, std::string>&& _map);
		inline std::unordered_map<std::string, std::string>& get() { return headers; }
		inline const std::unordered_map<std::string, std::string>& get() const { return headers; }
		std::string find(const std::string& key) const;
		void add(const std::string& key, const std::string& val);
		void remove(const std::string& key);
		void borrow(const HttpHeaders& other, const std::string& key, const std::string& defVal = "");
		void clear();
		template<Formattable T>
		void add(const std::string& key, const T& val);
		template<Formattable T>
		void add(const std::string& key, T&& val);
		std::unordered_map<std::string, std::string> cookies() const;
	private:
		std::unordered_map<std::string, std::string> headers;
	};

	template<Formattable T>
	void HttpHeaders::add(const std::string& key, const T& val) {
		headers[key] = std::format("{}", val);
	}

	template<Formattable T>
	void HttpHeaders::add(const std::string& key, T&& val) {
		headers[key] = std::format("{}", val);
	}

	struct HttpRequest {
		HttpRequest();
		HttpRequest(Method method, const std::string& url, const std::string& version, const std::unordered_map<std::string, std::string>& headers = {}, const std::string& body = "");
		std::string encode() const;
		Method method = Method::GET;
		std::string url;
		std::string version;
		HttpHeaders headers;
		std::string body;
	};

	struct HttpResponse {
		HttpResponse();
		HttpResponse(const std::string& version, size_t status, const std::unordered_map<std::string, std::string>& headers = {}, const std::string& body = "", const HttpHeaders& reqHeaders = {});
		HttpResponse(std::string&& version, size_t status, std::unordered_map<std::string, std::string>&& headers = {}, std::string&& body = "", HttpHeaders&& reqHeaders = {});
		HttpResponse(size_t status, const std::unordered_map<std::string, std::string>& headers = {}, const std::string& body = "", const HttpHeaders& reqHeaders = {});
		HttpResponse(size_t status, std::unordered_map<std::string, std::string>&& headers = {}, std::string&& body = "", HttpHeaders&& reqHeaders = {});
		HttpResponse(const std::string& version, size_t status, const HttpHeaders& headers = {}, const std::string& body = "", const HttpHeaders& reqHeaders = {});
		HttpResponse(std::string&& version, size_t status, HttpHeaders&& headers = {}, std::string&& body = "", HttpHeaders&& reqHeaders = {});
		HttpResponse(size_t status, const HttpHeaders& headers = {}, const std::string& body = "", const HttpHeaders& reqHeaders = {});
		HttpResponse(size_t status, HttpHeaders&& headers = {}, std::string&& body = "", HttpHeaders&& reqHeaders = {});
		// finishes response by auto appending headers (for example, "content-length")
		void finish(const HttpHeaders& reqHeaders);
		std::string encode() const;
		std::string version;
		size_t status = 0;
		std::string statusText;
		HttpHeaders headers;
		std::string body;

		static constexpr char Default_Http_Version[] = "HTTP/1.1";
	};

	template<typename T>
	concept CHttpMessage = std::is_same_v<T, HttpRequest> || std::is_same_v<T, HttpResponse>;

	template<CHttpMessage HttpMessage>
	class HttpParser {
	public:
		HttpParser();
		HttpParser(std::string_view s);
		HttpMessage& message();
		const HttpMessage& message() const;

		inline const auto& headers() const { return msg.headers; }
		inline auto& headers() { return msg.headers; }
		inline const auto& body() const { return msg.body; }
		inline auto& body() { return msg.body; }
		inline bool parsed() const { return _parsed; }
		bool parse(std::string_view s);
	private:
		bool parseFirstLine(std::string_view s);
		bool parseHeader(std::string_view line);

		HttpMessage msg;
		bool _parsed = false;
	};

	template<CHttpMessage HttpMessageStart>
	HttpParser<HttpMessageStart>::HttpParser() {
		;
	}

	template<CHttpMessage HttpMessageStart>
	HttpParser<HttpMessageStart>::HttpParser(std::string_view s) {
		if (s.empty()) {
			throw std::invalid_argument("http: empty string has been passed");
		}
		if (!parse(s)) {
			throw std::invalid_argument("http: invalid input sring");
		}
	}

	template<CHttpMessage HttpMessage>
	HttpMessage& HttpParser<HttpMessage>::message() {
		return msg;
	}

	template<CHttpMessage HttpMessage>
	const HttpMessage& HttpParser<HttpMessage>::message() const {
		return msg;
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
				msg.body = std::string(line.data(), s.data() + s.size());
				break;
			}
			// header
			else {
				if (!parseHeader(sline)) {
					return false;
				}
			}
		}
		_parsed = true;
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
			std::transform(method.begin(), method.end(), method.begin(), ::toupper);
			msg = HttpRequest{ strToMethod(method), v2str(parts[1]), v2str(parts[2]) };
		}
		// response
		else {
			msg = HttpResponse(v2str(parts[0]), std::stoull(v2str(parts[1])), HttpHeaders());
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
		StringTitlefier titlefier(key, { ' ', '-' });
		std::transform(key.begin(), key.end(), key.begin(), titlefier);
		msg.headers.add(std::move(key), v2str(strip(vval)));
		return true;
	}
}