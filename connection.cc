#include "connection.h"
#include <functional>
#include <iostream>
struct Defer {
	Defer(std::function<void()> ff) : f(ff) {}
	~Defer() { f(); }
	std::function<void()> f;
};

Connection::ErrorDetail Connection::getError(SQLHANDLE hstmt) {
	ErrorDetail ed;
	char state[6]; SQLINTEGER native_err = 0;
	SQLSMALLINT len = 0;
	BufferResult buf = getBuffer(SQL_CHAR);
	SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, (SQLCHAR*)state, &native_err, (SQLCHAR*)buf.ptr, buf.buf_size, &len);

	ed.description = buf.ptr;
	ed.errcode = state;
	ed.native_err = native_err;
	returnBuffer(buf);
	return ed;
}

bool Connection::checkSucceed(QueryResult& qr, SQLHANDLE hstmt) {
	if (qr.retcode == SQL_SUCCESS || qr.retcode == SQL_NO_DATA) return true;
	
	qr.errors.push_back(getError(hstmt));
	
	if (qr.retcode == SQL_SUCCESS_WITH_INFO) return true;
	return false;
}

#define CHECK_SUCCEED_RT(rt, qr, hstmt) do\
{ if(!checkSucceed(qr, hstmt)) return rt; \
}while(0)


RETCODE Connection::connect() {
	retcode_ = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &hEnv);

	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	retcode_ = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	retcode_ = SQLConnect(hDBC,
		(SQLCHAR*)database_.c_str(), SQL_NTS,
		(SQLCHAR*)user_.c_str(), SQL_NTS,
		(SQLCHAR*)password_.c_str(), SQL_NTS
	);
	return retcode_;
}

std::shared_ptr<Connection::QueryResult> Connection::query(const char* txt)
{
	std::shared_ptr<Connection::QueryResult> result(new QueryResult(0));

	// QueryResult result(0);
	SQLSMALLINT num_col;
	auto hstmt = getStatementHandle();
	// Defer hstmt_gard([this, &hstmt]() { returnStatementHandle(hstmt); });

	result->retcode = SQLExecDirect(hstmt, (SQLCHAR*)txt, SQL_NTS);
	result->num_rows = 0;
	CHECK_SUCCEED_RT(result, *result, hstmt);
	

	result->retcode = SQLNumResultCols(hstmt, &num_col);
	CHECK_SUCCEED_RT(result, *result, hstmt);

	std::vector< BufferResult> bufs;
	Defer buffer_guard([this, &bufs]() {
		for (auto& b : bufs)
			returnBuffer(b);
	});

	result->retcode = fetchColDesc(hstmt, &result->col_desc, &bufs);

	result->retcode = fetchData(hstmt, result, &bufs, num_col);

	return result;
}

RETCODE Connection::fetchData(SQLHANDLE hstmt, std::shared_ptr<QueryResult> res, std::vector<BufferResult>*pbufs, SQLSMALLINT num_col) {
	QueryResult& result = *res;
	std::vector<BufferResult>& bufs = *pbufs;
	// https://dev.mysql.com/doc/connector-odbc/en/connector-odbc-reference-api.html table 7.6
	result.retcode = SQLRowCount(hstmt, &result.num_rows);
	result.data.reserve(result.num_rows);
	for (SQLLEN j = 0; j < result.num_rows; ++j) {
		result.retcode = SQLFetch(hstmt);
		result.data.push_back(std::vector<ISQLData*>{});
		if (result.retcode == SQL_SUCCESS) {
			for (SQLSMALLINT i = 0; i < num_col; ++i) {
				ISQLData *data = nullptr;
				if (bufs[i].len == SQL_NULL_DATA) {
					data = new SQLNull;
				}
				else
					data = SQLDataConverter::NewSQLData(bufs[i].ptr, result.col_desc[i].type, bufs[i].len);
				result.data[j].push_back(data);
			}
		}
	}
	// SQLNumeric *n = result.data[0][7]->AsNumeric();
	// auto m = n->Value();
	return SQL_SUCCESS;
}


RETCODE Connection::fetchColDesc(SQLHANDLE hstmt, std::vector<Connection::ColumnDesc>* cols, std::vector<Connection::BufferResult>* pbufs) {
	SQLSMALLINT num_col;
	SQLNumResultCols(hstmt, &num_col);
	char tmp_buf[kLargeBufSize];
	RETCODE retcode = SQL_SUCCESS;
	std::vector<BufferResult>& bufs = *pbufs;
	bufs.reserve(num_col);
	for (SQLSMALLINT i = 1; i <= num_col; ++i) {
		ColumnDesc desc;
		retcode = SQLDescribeCol(hstmt, i, (SQLCHAR*)tmp_buf, kLargeBufSize, &desc.name_len, &desc.type, &desc.col_size, &desc.decimal_digits, &desc.nullable);

		if (desc.name_len == SQL_NTS || desc.name_len <= 0)
			desc.name = std::string(tmp_buf);
		else
			desc.name = std::string(tmp_buf, desc.name_len);

		bufs.push_back(getBuffer(desc.type));
		retcode = SQLBindCol(hstmt, i, SQLDataConverter::SQLIdToCType(desc.type), bufs.back().ptr, bufs.back().buf_size, &bufs.back().len);
		if (retcode != SQL_SUCCESS && SQL_SUCCESS_WITH_INFO)
			return retcode;
		cols->push_back(std::move(desc));
	}
	return retcode;
}

Connection::QueryHandle Connection::prepare(const char* stmt) {
	QueryHandle h(-1, 0, 0);
	SQLHANDLE hstmt;
	SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hstmt);
	
	h.retcode = SQLPrepare(hstmt, (SQLCHAR*)stmt, SQL_NTS);
	
	if (h.retcode != SQL_SUCCESS && h.retcode != SQL_SUCCESS_WITH_INFO) {
		char state[6]; SQLINTEGER native_err = 0;
		SQLSMALLINT len = 0;
		BufferResult buf = getBuffer(SQL_CHAR);
		SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, (SQLCHAR*)state, &native_err, (SQLCHAR*)buf.ptr, buf.buf_size, &len);
		h.errors.description = buf.ptr;
		h.errors.errcode = state;
		h.errors.native_err = native_err;
		return h;
	}
		
	
	SQLNumParams(hstmt, &h.num_params);

	QueryDetailed qd;
	qd.num_params = h.num_params;
	qd.hstmt = hstmt;
	qd.size_of_fetch.resize(h.num_params, 0);
	qd.retcode = h.retcode = fetchColDesc(hstmt, &qd.col_desc, &qd.buffers);
	h.idx = query_details_.size();
	query_details_.push_back(std::move(qd));
	return h;
}

template<typename T>
void Connection::bindNumberParamHelper(const QueryHandle h, unsigned short param_num, T*ptr, int c_type) {
	auto& qd = query_details_[h.idx];
	SQLSMALLINT param_type, decimal_digit, nullable;
	SQLULEN param_size;
	qd.size_of_fetch[param_num-1] = 0;
	auto retode = SQLDescribeParam(qd.hstmt, param_num, &param_type, &param_size, &decimal_digit, &nullable);
	retode = SQLBindParameter(qd.hstmt, param_num, SQL_PARAM_INPUT, c_type, param_type, param_size, decimal_digit, ptr, sizeof(T), &qd.size_of_fetch[param_num-1]);
}

void Connection::bindParam(QueryHandle h, unsigned short param_num, double* ptr)
{
	bindNumberParamHelper<double>(h, param_num, ptr, SQL_C_DOUBLE);
}

void Connection::bindParam(QueryHandle h, unsigned short param_num, int* ptr)
{
	bindNumberParamHelper<int>(h, param_num, ptr, SQL_C_LONG);
}

void Connection::bindParam(QueryHandle h, unsigned short param_num, int64_t*ptr)
{
	bindNumberParamHelper<int64_t>(h, param_num, ptr, SQL_C_SBIGINT);
}


void Connection::bindParam(QueryHandle h, unsigned short param_num, std::string& str)
{
	auto& qd = query_details_[h.idx];
	SQLSMALLINT param_type, decimal_digit, nullable;
	SQLULEN param_size;
	qd.size_of_fetch[param_num-1] = SQL_NTS;
	auto retcode = SQLDescribeParam(qd.hstmt, param_num, &param_type, &param_size, &decimal_digit, &nullable);
	retcode = SQLBindParameter(qd.hstmt, param_num, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, param_size, 0, (SQLPOINTER)str.c_str(), str.length()+1, &qd.size_of_fetch[param_num-1]);
}

void Connection::bindParam(QueryHandle h, unsigned short param_num, SQLNumeric::Num&n, std::string& str_buffer)
{
	str_buffer = SQLNumeric::Stringnize(n);
	auto& qd = query_details_[h.idx];
	SQLSMALLINT param_type, decimal_digit, nullable;
	SQLULEN param_size;
	qd.size_of_fetch[param_num-1] = SQL_NTS;
	auto retcode = SQLDescribeParam(qd.hstmt, param_num, &param_type, &param_size, &decimal_digit, &nullable);
	retcode = SQLBindParameter(qd.hstmt, param_num, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_NUMERIC, param_size, 0, (SQLPOINTER)str_buffer.c_str(), str_buffer.length() + 1, &qd.size_of_fetch[param_num-1]);
}

void Connection::bindParam(QueryHandle& h, unsigned short param_num, SQLDateStruct*ptr)
{
	auto& qd = query_details_[h.idx];
	SQLSMALLINT param_type, decimal_digit, nullable;
	SQLULEN param_size;
	qd.size_of_fetch[param_num - 1] = 0;
	auto retcode = SQLDescribeParam(qd.hstmt, param_num, &param_type, &param_size, &decimal_digit, &nullable);
	retcode = SQLBindParameter(qd.hstmt, param_num, SQL_PARAM_INPUT, SQL_C_DATE, param_type, param_size, decimal_digit, ptr, sizeof(SQLDateStruct), &qd.size_of_fetch[param_num - 1]);
	if (retcode != 0) { h.errors = getError(qd.hstmt); h.retcode = retcode; };
}

void Connection::bindParam(QueryHandle& h, unsigned short param_num, SQLTimeStruct*ptr)
{
	auto& qd = query_details_[h.idx];
	SQLSMALLINT param_type, decimal_digit, nullable;
	SQLULEN param_size;
	qd.size_of_fetch[param_num-1] = 0;
	auto retcode = SQLDescribeParam(qd.hstmt, param_num, &param_type, &param_size, &decimal_digit, &nullable);
	retcode = SQLBindParameter(qd.hstmt, param_num, SQL_PARAM_INPUT, SQL_C_TIME, param_type, param_size, decimal_digit, ptr, sizeof(SQLTimeStruct), &qd.size_of_fetch[param_num-1]);
	if (retcode != 0) { h.errors = getError(qd.hstmt); h.retcode = retcode; };
}

void Connection::bindParam(QueryHandle& h, unsigned short param_num, SQLTimeStampStruct* ptr)
{
	auto& qd = query_details_[h.idx];
	SQLSMALLINT param_type, decimal_digit, nullable;
	SQLULEN param_size;
	qd.size_of_fetch[param_num-1] = 0;
	auto retcode = SQLDescribeParam(qd.hstmt, param_num, &param_type, &param_size, &decimal_digit, &nullable);
	retcode = SQLBindParameter(qd.hstmt, param_num, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, param_type, param_size, decimal_digit, ptr, sizeof(SQLTimeStampStruct), &qd.size_of_fetch[param_num-1]);
	if (retcode != 0) { h.errors = getError(qd.hstmt); h.retcode = retcode; };
}

void Connection::bindParam(QueryHandle h, unsigned short param_num, SQLNullData&)
{
	auto& qd = query_details_[h.idx];
	SQLSMALLINT param_type, decimal_digit, nullable;
	SQLULEN param_size;
	qd.size_of_fetch[param_num-1] = SQL_NULL_DATA;
	auto retcode = SQLDescribeParam(qd.hstmt, param_num, &param_type, &param_size, &decimal_digit, &nullable);
	retcode = SQLBindParameter(qd.hstmt, param_num, SQL_PARAM_INPUT, SQL_C_CHAR, param_type, param_size, 0, nullptr, 0, &qd.size_of_fetch[param_num-1]);
}

std::shared_ptr<Connection::QueryResult> Connection::queryPrepared(QueryHandle& h) {
	std::shared_ptr<Connection::QueryResult> qr(new QueryResult(0));

	auto& qd = query_details_[h.idx];
	
	Defer sanitize_hanlde([&qd] {
		// SQLFreeStmt(qd.hstmt, SQL_UNBIND);
		SQLFreeStmt(qd.hstmt, SQL_RESET_PARAMS);
		SQLCloseCursor(qd.hstmt);
	});
	
	qr->retcode = SQLExecute(qd.hstmt);
	// assert(qr->retcode == 0);
	CHECK_SUCCEED_RT(qr, *qr, qd.hstmt);
	qr->col_desc = qd.col_desc;
	qr->retcode = fetchData(qd.hstmt, qr, &qd.buffers, qd.col_desc.size());
	return qr;
}

Connection::BufferResult Connection::getBuffer(short type) {
#define SMALL(id, ctm, tydef, cty, sqld, cast, name) case id: /*fall through*/
#define UNKOWN_SIZE(id, ctm, tydef, cty, sqld, cast, name) case id:
#define DO_NOTHING(...)
	BufferResult r{ nullptr, 0, 0 };
	BufferResult *pr = &r;
	switch (type)
	{
		ODBC_TO_SQLDATA(SMALL, DO_NOTHING, DO_NOTHING)
			if (mini_buffer_pool_.empty()) {
				r.ptr = new char[kMiniBufSize];
				r.buf_size = kMiniBufSize;
				return r;
			}
			else {
				auto ptr = mini_buffer_pool_.back();
				mini_buffer_pool_.pop_back();
				return BufferResult{ ptr, kMiniBufSize, 0 };
			}
		ODBC_TO_SQLDATA(DO_NOTHING, UNKOWN_SIZE, UNKOWN_SIZE)
			if (large_buffer_pool_.empty()) {
				r.ptr = new char[kLargeBufSize];
				r.buf_size = kLargeBufSize;
				return r;
			}
		//return BufferResult{ new char[kLargeBufSize], kLargeBufSize, 0 };
			else {
				auto ptr = large_buffer_pool_.back();
				large_buffer_pool_.pop_back();
				return BufferResult{ ptr, kLargeBufSize, 0 };
			}
	};

	assert(0);
	return BufferResult{ nullptr, 0, 0 };
#undef SMALL
#undef UNKOWN_SIZE
#undef DO_NOTHING
}

inline void Connection::returnBuffer(Connection::BufferResult& r) {
	switch (r.buf_size)
	{
	case kMiniBufSize:
		return mini_buffer_pool_.push_back(r.ptr);
	case kLargeBufSize:
		return large_buffer_pool_.push_back(r.ptr);
	}
}

// It seemes that we can't recycle handle of different query

inline void Connection::returnStatementHandle(SQLHANDLE hstmt)
{
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	// sanitize handle
	/*
	SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
	SQLFreeStmt(hstmt, SQL_UNBIND);
	hstates_.push_back(hstmt);
	*/
}

inline SQLHANDLE Connection::getStatementHandle() {
	SQLHANDLE hstmt;
	SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hstmt);
	return hstmt;
	/*
	if (hstates_.empty()) {
		SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hstmt);
		return hstmt;
	}
	hstmt = hstates_.back();
	hstates_.pop_back();
	return hstmt;*/
}

Connection::QueryResult::~QueryResult()
{
	for (auto& row : data) {
		for (auto col : row) {
			delete col;
		}
	}
}
