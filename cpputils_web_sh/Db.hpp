#pragma once
#include <string>
#include <any>
#include <vector>
#include <cassert>
#include <cstdint>

namespace db {
	
	class IDb {
	public:
		inline virtual ~IDb() {};
		virtual int connect(const std::string& url, const std::string& username, const std::string& password) = 0;
		virtual int setSchema(const std::string& schema) = 0;
		virtual int query(const std::string& request) = 0;
		// general modify request
		virtual int modify(const std::string& request) = 0;
		// only for select queries
		virtual size_t resultRowsSize() = 0;
		// only for select queries
		virtual bool resultEmpty() = 0;
		template<typename... Args>
		std::vector<std::tuple<Args...>> result(const std::vector<std::string>& fields);
		template<typename... Args>
		std::vector<std::tuple<Args...>> result(const std::vector<size_t>& fields);
		// to get single-row single-column result
		template<typename Arg>
		Arg result();
		// to get multi-row single-column results
		template<typename Arg>
		std::vector<Arg> result(const std::string& field);
		template<typename Arg>
		std::vector<Arg> result(size_t field);
	protected:
		// proceeds result to next row
		virtual bool next() = 0;
		template<typename T, typename ArgT>
		T getCol(ArgT&& field);
		virtual int64_t getColInt64(const std::string& field) = 0;
		virtual int64_t getColInt64(size_t field) = 0;
		virtual uint64_t getColUint64(const std::string& field) = 0;
		virtual uint64_t getColUint64(size_t field) = 0;
		virtual std::string getColString(const std::string& field) = 0;
		virtual std::string getColString(size_t field) = 0;
		virtual double getColDouble(const std::string& field) = 0;
		virtual double getColDouble(size_t field) = 0;
	};

	template<typename... Args>
	std::vector<std::tuple<Args...>> IDb::result(const std::vector<std::string>& fields) {
		std::vector<std::tuple<Args...>> vres;
		size_t iArg = 0;
		while (next()) {
			vres.push_back(std::tuple<Args...>{getCol<Args>(fields[iArg++])...});
			iArg = 0;
		}
		return vres;
	}

	template<typename... Args>
	std::vector<std::tuple<Args...>> IDb::result(const std::vector<size_t>& fields) {
		std::vector<std::tuple<Args...>> vres;
		size_t iArg = 0;
		while (next()) {
			vres.push_back(std::tuple<Args...>{getCol<Args>(fields[iArg++])...});
			iArg = 0;
		}
		return vres;
	}

	template<typename Arg>
	Arg IDb::result() {
		next();
		return getCol<Arg>(1);
	}

	template<typename Arg>
	std::vector<Arg> IDb::result(const std::string& field) {
		std::vector<Arg> vres;
		while (next()) {
			vres.push_back(getCor<Arg>(field));
		}
		return vres;
	}

	template<typename Arg>
	std::vector<Arg> IDb::result(size_t field) {
		std::vector<Arg> vres;
		while (next()) {
			vres.push_back(getCol<Arg>(field));
		}
		return vres;
	}

	template<typename T, typename ArgT>
	T IDb::getCol(ArgT&& field) {
		if constexpr (std::is_same_v<T, int64_t>) {
			return getColInt64(field);
		}
		else if constexpr (std::is_same_v<T, uint64_t>) {
			return getColUint64(field);
		}
		else if constexpr (std::is_same_v<T, std::string>) {
			return getColString(field);
		}
		else if constexpr (std::is_same_v<T, double>) {
			return getColDouble(field);
		}
		else {
			assert(false);
		}
	}


}