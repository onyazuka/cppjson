#pragma once
#include "Db.hpp"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

namespace db {

	/*
		mysql cpp connector is exception based, and those exceptions are not catched in this class,
		and still returning int syntax remains for better compability with other dbs
	*/
	class DbMysql : public IDb {
	public:
		DbMysql();
		DbMysql(const std::string& url, const std::string& username, const std::string& password);
		DbMysql(const DbMysql&) = delete;
		DbMysql& operator=(const DbMysql&) = delete;
		~DbMysql();
		int connect(const std::string& url, const std::string& username, const std::string& password) override;
		int setSchema(const std::string& schema) override;
		int query(const std::string& request) override;
		int modify(const std::string& request) override;
		size_t resultRowsSize() override;
		bool resultEmpty() override;
	private:
		bool next() override;
		int64_t getColInt64(const std::string& field) override;
		int64_t getColInt64(size_t field) override;
		uint64_t getColUint64(const std::string& field) override;
		uint64_t getColUint64(size_t field) override;
		std::string getColString(const std::string& field) override;
		std::string getColString(size_t field) override;
		double getColDouble(const std::string& field) override;
		double getColDouble(size_t field) override;
		void initDriver();
		// no need to delete driver, so using plain pointer
		sql::Driver* driver;
		std::unique_ptr<sql::Connection> con;
		std::unique_ptr<sql::Statement> stmt;
		std::unique_ptr<sql::ResultSet> res;
	};

}