#pragma once
#include <string>
#include <vector>
#include <deque>
#include "sqldata.h"
// #include "sqldata-converter.h"

class Connection {
public:
	struct ErrorDetail {
		ErrorDetail() : native_err(0) {}
		std::string description;
		int native_err;
		std::string errcode;
	};

	struct ColumnDesc {
		std::string name; // **
		short type; // this is sql type
		short nullable;
		short name_len;
		short decimal_digits;
		size_t col_size;
	};

	struct QueryResult {
		QueryResult(int rcode) : retcode(rcode), col_desc(), data(), num_rows(0){}
		std::vector<std::vector<std::shared_ptr<ISQLData>>> data; // **
		std::vector<ColumnDesc> col_desc; // **
		int retcode; //** return 0 if success
		size_t num_rows; // **
		std::vector<ErrorDetail> errors;
	};

	Connection(const char *database, const char *user, const char*password) 
		:database_(database), user_(user), password_(password)
	{}

	int connect();

	std::shared_ptr<QueryResult> query(const char* txt);

private:

	std::string database_;
	std::string user_;
	std::string password_;
};



