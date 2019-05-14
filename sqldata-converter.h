#pragma once
#include <sql.h>
#include "sqldata.h"

// https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types?view=sql-server-2017
#define ODBC_TO_SQLDATA(V /*plain pod*/, P /*pointer type*/, S) \
   /*QL type identifier |C type identifier | ODBC C typedef |  C type          | SQLData type  | Cast method */  \
	P(SQL_CHAR,           SQL_C_CHAR,      SQLCHAR *,       unsigned char *,    SQLString,      const char *, char) \
	P(SQL_VARCHAR,        SQL_C_CHAR,      SQLCHAR *,       unsigned char *,    SQLString,      const char *, varchar) \
	P(SQL_LONGVARCHAR,    SQL_C_CHAR,      SQLCHAR *,       unsigned char *,    SQLString,      const char *, longvarchar) \
	P(SQL_WCHAR,          SQL_C_CHAR,      SQLWCHAR*,       wchar_t*,           SQLString,      const char*, wchar) \
	P(SQL_WVARCHAR,       SQL_C_CHAR,      SQLWCHAR*,       wchar_t*,           SQLString,      const char*, wvarchar) \
	P(SQL_WLONGVARCHAR,   SQL_C_CHAR,      SQLWCHAR*,       wchar_t*,           SQLString,      const char*, wlongvarchar) \
	V(SQL_SMALLINT,       SQL_C_SHORT,      SQLSMALLINT,     short int,          SQLInt,         short int, smallint)  \
  /*V(SQL_C_SSHORT,     SQLSMALLINT,     short int,          SQLInt,         short int)  \
	V(SQL_C_USHORT,     SQLUSMALLINT,    unsigned short int, SQLInt,         unsigned short) */\
	V(SQL_INTEGER,        SQL_C_SLONG,      SQLINTEGER,      long,               SQLInt,      long, int)           \
	/*V(SQL_C_ULONG,      SQLUINTEGER,     unsigned long,      SQLBigInt,      unsigned long) */\
	V(SQL_REAL,           SQL_C_FLOAT,      SQLREAL,         float,              SQLFloat,       float, float)          \
	V(SQL_DOUBLE,         SQL_C_DOUBLE,     SQLDOUBLE,       double,             SQLDouble,      double, float)        \
	V(SQL_BIT,            SQL_C_BIT,        SQLCHAR,         unsigned char,      SQLInt,         unsigned char, int) \
	V(SQL_TINYINT,        SQL_C_STINYINT,   SQLSCHAR,        signed char,        SQLInt,         char, int)\
	/*V(SQL_C_UTINYINT,   SQLCHAR,         unsigned char,      SQLInt,         unsigned char)*/\
	V(SQL_BIGINT,         SQL_C_SBIGINT,    SQLBIGINT,       int64_t,            SQLBigInt,      int64_t, bigint) \
	/*V(SQL_C_UBIGINT,    SQLUBIGINT,      uint64_t,           SQLBigInt,      uint64_t) \
  /*P(SQL_C_BINARY,     SQLCHAR *,       unsigned char *,    SQLString,      const char *)*/ \
  /*V(SQL_C_BOOKMARK,   BOOKMARK, ...)\
  /*V(SQL_C_VARBOOKMARK, ...)*/\
	S(SQL_TYPE_DATE,      SQL_C_TYPE_DATE,  SQL_DATE_STRUCT, DATE_STRUCT,        SQLDate,        SQLDateStruct, date) \
	S(SQL_TYPE_TIME,      SQL_C_TYPE_TIME,  SQL_TIME_STRUCT, DATE_STRUCT,        SQLTime,        SQLTimeStruct, time) \
	S(SQL_TYPE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP_STRUCT, TIMESTAMP_STRUCT, SQLTimeStamp, SQLTimeStampStruct, timestamp)\
	S(SQL_NUMERIC,      SQL_CHAR,    SQL_NUMERIC_STRUCT, SQL_NUMERIC_STRUCT, SQLNumeric,  const char * /*1*/, numeric)\
	P(SQL_DECIMAL,      SQL_CHAR	,    SQLCHAR *,       unsigned char *,    SQLNumeric,      const char * /*1*/, numeric)\
	/*V(SQL_C_GUID, SQLGUID, ...)*/ 
// --- NOTE
// 1. SQL_DECIMAL and SQL_NUMERIC data types differ only in their precision

class SQLDataConverter {
public:
	static ISQLData* NewSQLData(void* data, SQLSMALLINT type, SQLULEN len) {
		switch (type)
		{
#define TYPE_CASE(id, ctm, tydef, cty, sqld, cast, name)\
		case id: \
				return new sqld((*static_cast<cast *>(data)));\

#define PTR_TYPE_CASE(id, ctm, tydef, cty, sqld, cast, name) \
		case id: \
			if(len == SQL_NTS) \
				return new sqld(static_cast<cast>(data));\
			else\
				return new sqld(static_cast<cast>(data), len);
			// return new sqld((static_cast<cast>(data)));
			ODBC_TO_SQLDATA(TYPE_CASE, PTR_TYPE_CASE, TYPE_CASE)
#undef PTR_TYPE_CASE
#undef TYPE_CASE
		default:
			return new SQLNull();
			break;
		}
	}

	static SQLSMALLINT SQLIdToCType(SQLSMALLINT type) {

		switch (type)
		{
#define TYPE_CASE(id, ctm, tydef, cty, sqld, cast, name)\
		case id: return ctm;
			ODBC_TO_SQLDATA(TYPE_CASE, TYPE_CASE, TYPE_CASE)
#undef TYPE_CASE
		default:
			break;
		}
		//TODO: What should I return ??
		return SQL_NULL_DATA;
	}

	static int SQLTypeDecay(SQLSMALLINT type) {
		switch (type)
		{
#define TYPE_CASE(id, ctm, tydef, cty, sqld, cast, name)\
		case id: return SQLTypeID<sqld>::value;
			ODBC_TO_SQLDATA(TYPE_CASE, TYPE_CASE, TYPE_CASE)
#undef TYPE_CASE
		default:
			break;
		}
		return -1;
	}

	static void FetchAllName(std::function<void(int, const char*)>callback) {
#define FETCH_TYPE(t, u) callback(SQLTypeID<SQL##t>::value, SQLTypeID<SQL##t>::name);
		SQL_DATA_TYPE_LIST(FETCH_TYPE)
#undef FETCH_TYPE
	}

	static int GetNumSolidTypes() {
		return SQLTypeID<SQLTimeStamp>::value + 1;
	}
};