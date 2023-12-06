#include "DbMysql.hpp"

using namespace db;

DbMysql::DbMysql() {
	initDriver();
}

DbMysql::DbMysql(const std::string& url, const std::string& username, const std::string& password) {
	initDriver();
	connect(url, username, password);
}

DbMysql::~DbMysql() {
	// needed for thread-safe usage (?)
	driver->threadEnd();
}

int DbMysql::connect(const std::string& url, const std::string& username, const std::string& password) {
	// needed for thread-safe usage (?)
	driver->threadInit();	
	con = std::move(std::unique_ptr<sql::Connection>(driver->connect(url, username, password)));
	if (!con) {
		throw std::runtime_error("Can't create mysql connection");
	}
	stmt = std::move(std::unique_ptr<sql::Statement>(con->createStatement()));
	if (!stmt) {
		throw std::runtime_error("Can't create mysql statement");
	}
	return 0;
}

int DbMysql::setSchema(const std::string& schema) {
	con->setSchema(schema);
	return 0;
}

int DbMysql::query(const std::string& request) {
	res = std::move(std::unique_ptr<sql::ResultSet>(stmt->executeQuery(request)));
	return 0;
}

int DbMysql::modify(const std::string& request) {
	return stmt->executeUpdate(request);
}

size_t DbMysql::resultRowsSize() {
	return res->rowsCount();
}

bool DbMysql::resultEmpty() {
	return !res->rowsCount();
}

void DbMysql::initDriver() {
	driver = get_driver_instance();
	if (!driver) {
		throw std::runtime_error("Can't create mysql driver");
	}
}

bool DbMysql::next() {
	return res->next();
}

int64_t DbMysql::getColInt64(const std::string& field) {
	return res->getInt64(field);
}

int64_t DbMysql::getColInt64(size_t field) {
	return res->getInt64(field);
}

uint64_t DbMysql::getColUint64(const std::string& field) {
	return res->getUInt64(field);
}

uint64_t DbMysql::getColUint64(size_t field) {
	return res->getUInt64(field);
}

std::string DbMysql::getColString(const std::string& field) {
	return res->getString(field);
}

std::string DbMysql::getColString(size_t field) {
	return res->getString(field);
}

double DbMysql::getColDouble(const std::string& field) {
	return res->getDouble(field);
}

double DbMysql::getColDouble(size_t field) {
	return res->getDouble(field);
}