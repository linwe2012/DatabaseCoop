#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <deque>
#include "sqldata-converter.h"

class Connection {

	struct BufferResult {
		char* ptr;
		SQLLEN buf_size;
		SQLLEN len;
	};
public:
	struct ErrorDetail {
		ErrorDetail() : native_err(0) {}
		std::string description;
		SQLINTEGER native_err;
		std::string errcode;
	};

	struct ColumnDesc {
		std::string name;
		SQLSMALLINT type; // this is sql type
		SQLSMALLINT nullable;
		SQLSMALLINT name_len;
		SQLSMALLINT decimal_digits;
		SQLULEN col_size;
	};

	struct QueryDetailed {
		RETCODE retcode;
		SQLSMALLINT num_params;
		std::vector<ColumnDesc> col_desc;
		std::vector<BufferResult> buffers;
		std::vector<SQLLEN> size_of_fetch;
		SQLHANDLE hstmt;
		// SQLLEN size_of_fetch;
	};

	struct QueryHandle {
		QueryHandle() :idx(0), retcode(0), num_params(0) {}
		QueryHandle(int midx, RETCODE mretcode, SQLSMALLINT mnum_params) : idx(midx), retcode(mretcode), num_params(mnum_params){}
		int idx;
		RETCODE retcode;
		SQLSMALLINT num_params;
		ErrorDetail errors;
	};

	struct QueryResult {
		QueryResult(RETCODE rcode) : retcode(rcode), col_desc(), data(), num_rows(0){}
		RETCODE retcode;
		std::vector<ColumnDesc> col_desc;
		std::vector<std::vector<ISQLData*>> data;
		SQLLEN num_rows;
		std::vector<ErrorDetail> errors;
	};

	Connection(const char *database, const char *user, const char*password) 
		:database_(database), user_(user), password_(password),
		hEnv(nullptr), hDBC(nullptr), retcode_(0)
	{}

	RETCODE connect();

	std::shared_ptr<QueryResult> query(const char* txt);
	// std::shared_ptr<QueryResult> queryPrepared(QueryHandle& h);
	// QueryHandle prepare(const char* stmt);
	// void bindParam(QueryHandle h, unsigned short param_num, double*);
	// void bindParam(QueryHandle h, unsigned short param_num, int*);
	// void bindParam(QueryHandle h, unsigned short param_num, int64_t*);
	// void bindParam(QueryHandle h, unsigned short param_num, std::string& str);
	// void bindParam(QueryHandle h, unsigned short param_num, SQLNumeric::Num&, std::string& str_buffer);
	// void bindParam(QueryHandle& h, unsigned short param_num, SQLDateStruct*);
	// void bindParam(QueryHandle& h, unsigned short param_num, SQLTimeStruct*);
	// void bindParam(QueryHandle& h, unsigned short param_num, SQLTimeStampStruct*);
	// void bindParam(QueryHandle h, unsigned short param_num, SQLNullData&);
private:

	template<typename T>
	void bindNumberParamHelper(const QueryHandle h, unsigned short param_num, T*, int c_type);

	Connection::ErrorDetail getError(SQLHANDLE hstmt);

	bool checkSucceed(QueryResult& qd, SQLHANDLE hstmt);

	SQLHANDLE getStatementHandle();
	void returnStatementHandle(SQLHANDLE hstmt);

	BufferResult getBuffer(short type);
	void returnBuffer(BufferResult& r);
	RETCODE fetchColDesc(SQLHANDLE hstmt, std::vector<ColumnDesc>* cols, std::vector<BufferResult>* pbufs);
	RETCODE fetchData(SQLHANDLE hstmt, std::shared_ptr<QueryResult> res, std::vector<BufferResult>* pbufs, SQLSMALLINT num_col);
private:

	friend struct HandleGuard;

	SQLHANDLE hEnv, hDBC;
	std::string database_;
	std::string user_;
	std::string password_;
	RETCODE retcode_;

	std::vector<SQLHANDLE> hstates_;
	
	std::vector<char*>mini_buffer_pool_; // 64 bits, aka 8 bytes
	std::vector<char*>large_buffer_pool_; // 8192 bites
	std::vector<QueryDetailed> query_details_;
	enum {
		kMiniBufSize = 8,
		kLargeBufSize = 8192,
	};
};



