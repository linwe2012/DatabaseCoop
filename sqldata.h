#pragma once


#include <assert.h>
#include <memory>
#include <string>
#include <functional>
#include <sstream>

// https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types?view=sql-server-2017


// inheritance hierachy
// ISQLData
//   - SQLNull
//   - ISQLNumber 
//       - SQLInt
//       - SQLBigInt
//       - SQLDouble
//       - SQLFloat
//       - SQLNumeric
//   - SQLString
//   - ISQLTimeOrDate
//       - SQLDate
//       - SQLTime
//       - SQLTimeStamp

template<typename T>
using Shared = std::shared_ptr<T>;

struct SQLDateStruct {
	unsigned short year;
	unsigned short month;
	unsigned short date;
};

struct SQLTimeStruct {
	unsigned short hour;
	unsigned short minute;
	unsigned short second;
};

struct SQLNullData {};

struct SQLTimeStampStruct {
	SQLDateStruct date;
	SQLTimeStruct time;
	unsigned int fraction;
};

struct SQLNumericStruct {
	enum { kMaxNumericLen = SQL_MAX_NUMERIC_LEN };
	unsigned char precision;
	char scale;
	unsigned char sign; // The sign field is 1 if positive, 0 if negative.
	unsigned char val[kMaxNumericLen];
};

#define MAKE_SURE(cond, msg) assert(cond)

#define SQL_DATA_TYPE_LIST(V) \
	V(Int, int)               \
	V(BigInt, int64_t)        \
	V(Double, double)         \
	V(Float, float)           \
	V(String, std::string)    \
	V(Date, SQLDateStruct)    \
	V(Time, SQLTimeStruct)    \
	V(TimeStamp, SQLTimeStampStruct) \
	V(Numeric, SQLNumericStruct) \
	V(Null, void)

#define FWD_DECALRE(t, u) class SQL##t;
SQL_DATA_TYPE_LIST(FWD_DECALRE)
#undef FWD_DECALRE


class ISQLDataVisitor {
public:
#define VISITOR(t, u)  virtual void visit(SQL##t *) = 0;
	SQL_DATA_TYPE_LIST(VISITOR)
#undef VISITOR
};

class ISQLData {
public:
	virtual bool IsNumber() { return false; }
	virtual bool IsInt() { return false; }
	virtual bool IsBigInt() { return false; }
	virtual bool IsFloat() { return false; }
	virtual bool IsDouble() { return false; }
	virtual bool IsNumeric() { return false; }

	virtual bool IsString() { return false; }
	virtual bool IsBlob() { return false; }

	virtual bool IsTimeOrDate() { return false; }
	virtual bool IsDate() { return false; }
	virtual bool IsTime() { return false; }
	virtual bool IsTimeStamp() { return false; }
	virtual bool IsNull() { return false; }
	virtual ~ISQLData() = 0;
#define AS(t, u) virtual SQL##t * As##t() { return nullptr; } 
	SQL_DATA_TYPE_LIST(AS);
#undef AS
	virtual void Accept(ISQLDataVisitor*) = 0;
	// virtual ~ISQLData() = 0;
};

inline ISQLData::~ISQLData() {}



class SQLNull : public ISQLData {
public:
	SQLNull() {}
	bool IsNull() override { return true; }
	SQLNull* AsNull() override { return this; }
	void Accept(ISQLDataVisitor*) override;
	~SQLNull() override {}
};

class ISQLNumber : public ISQLData {
public:
	bool IsNumber() override { return true; }
	// virtual ~ISQLNumber() = 0;
};



#define SOLID_DATA(type, data_type)            \
bool Is##type () override { return true; }      \
data_type Value() { return val_; }               \
void Accept(ISQLDataVisitor*) override;            \
SQL##type* As##type() override { return this; }   \
SQL##type(data_type val) : val_(val) {}           \
~SQL##type() override {}


class SQLInt : public ISQLNumber {
public:
	SOLID_DATA(Int, int)
private:
	int val_;
};

class SQLBigInt : public ISQLNumber {
public:
	SOLID_DATA(BigInt, int64_t)
private:
	int64_t val_;
};

class SQLFloat : public ISQLNumber {
public:
	SOLID_DATA(Float, float)
	/*
	SQLFloat(float val) : val_(val) {};
	bool IsFloat() override { return true; }
	float Value() { return val_; }
	void Accept(ISQLDataVisitor*) override;
	SQLFloat* AsFloat() override { return this; }*/
private:
	float val_;
};

class SQLDouble : public ISQLNumber {
public:
	SOLID_DATA(Double, double)
	/*
	SQLDouble(double val) : val_(val) {}
	bool IsDouble() override { return true; }
	double Value() { return val_; }
	void Accept(ISQLDataVisitor*) override;
	SQLDouble* AsDouble() override { return this; }*/
private:
	double val_;
};

class SQLNumeric : public ISQLNumber {
public:
	struct Num {
		int64_t integer;
		int64_t decimal;
		int scale;
	};
	SQLNumeric(const char *val, int cnt) : val_(val, cnt) {}
	SQLNumeric(const char* val) : val_(val) {}
	bool IsNumeric() override { return true; }
	const char* Raw() { return val_.c_str(); }
	void Accept(ISQLDataVisitor*) override;
	SQLNumeric* AsNumeric() override { return this; }
	
	Num Value() {
		Num num{ 0, 0 , 0};
		int toggle = 0;
		char dump = '\0';
		std::stringstream ss(val_);
		ss >> num.integer;
		ss >> dump; // eat '.'
		int64_t pos = ss.tellg();
		if (pos < 0) return num;

		ss >> num.decimal;
		num.scale = val_.size() - pos;
		return num;
	}

	static std::string Stringnize(const Num& n) {
		std::stringstream ss;
		ss << n.integer;
		if (n.decimal == 0) return ss.str();
		ss << '.';
		ss << n.decimal;
		return ss.str();
	}
	~SQLNumeric() override {}
	// SOLID_DATA(Numeric, SQLNumericStruct)
private:
	std::string val_;
};


class SQLString : public ISQLData{
public:
	SQLString(const char * val, size_t n) : val_(val, n) {}
	SQLString(const char *val) : val_(val) {}
	bool IsString() override { return true; }
	const char* Value() { return val_.c_str(); }
	void Accept(ISQLDataVisitor*) override;
	SQLString* AsString() { return this; }
	~SQLString() override {}
private:
	std::string val_;
};


class ISQLTimeOrDate : public ISQLData {
public:
	bool IsTimeOrDate() override { return true; }
	// virtual ~ISQLTimeOrDate() = 0;
};

class SQLDate : public ISQLTimeOrDate {
public:
	SOLID_DATA(Date, SQLDateStruct)
private:
	SQLDateStruct val_;
};

class SQLTime : public ISQLTimeOrDate {
public:
	SOLID_DATA(Time, SQLTimeStruct)
private:
	SQLTimeStruct val_;
};


class SQLTimeStamp : public ISQLTimeOrDate {
public:
	SOLID_DATA(TimeStamp, SQLTimeStampStruct)
private:
	SQLTimeStampStruct val_;
};

template <typename T>
struct SQLTypeID {
	constexpr static int value = -1;
	constexpr static char* name = "invalid_type";
};


template<>
struct SQLTypeID<SQLNull> {
	constexpr static int value = 0;
	constexpr static char* name = "sqlnull";
};

template<>
struct SQLTypeID<SQLInt> {
	constexpr static int value = 1;
	constexpr static char* name = "int";
};

template<>
struct SQLTypeID<SQLBigInt> {
	constexpr static int value = 2;
	constexpr static char* name = "bigint";
};

template<>
struct SQLTypeID<SQLDouble> {
	constexpr static int value = 3;
	constexpr static char* name = "double";
};

template<>
struct SQLTypeID<SQLFloat> {
	constexpr static int value = 4;
	constexpr static char* name = "float";
};

template<>
struct SQLTypeID<SQLNumeric> {
	constexpr static int value = 5;
	constexpr static char* name = "numeric";
};

template<>
struct SQLTypeID<SQLString> {
	constexpr static int value = 6;
	constexpr static char* name = "string";
};

template<>
struct SQLTypeID<SQLDate> {
	constexpr static int value = 7;
	constexpr static char* name = "date";
};

template<>
struct SQLTypeID<SQLTime> {
	constexpr static int value = 8;
	constexpr static char* name = "time";
};

template<>
struct SQLTypeID<SQLTimeStamp> {
	constexpr static int value = 9;
	constexpr static char* name = "timestamp";
};


#define ACCEPT(t, u) inline void SQL##t::Accept(ISQLDataVisitor* visitor) { visitor->visit(this); } 
SQL_DATA_TYPE_LIST(ACCEPT)
#undef ACCEPT