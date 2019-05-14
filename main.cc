#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <assert.h>
#include <iostream>
#define ENABLE_V8
#ifdef ENABLE_V8
#include "jsconnection.h"

const char* return_code(RETCODE rc);

void MySQLretcodeToString(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	if (args.Length() < 1 || !args[0]->IsNumber()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Error: One number expected")));
		return;
	}
	auto retcode = Local<Number>::Cast(args[0])->Value();
	const char *name = return_code(retcode);
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, name));
}

void MySQLcreateConnection(const FunctionCallbackInfo<Value>& args) {
	JSConnection::NewInstance(args);
}

void MySQLfetchTypes(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();
	Local<Object> types = Object::New(isolate);
	SQLDataConverter::FetchAllName([&types, &isolate](int tid, const char* name) {
		types->Set(String::NewFromUtf8(isolate, name), Int32::New(isolate, tid));
	});
	args.GetReturnValue().Set(types);
}

void  MySQLfetchTypeNameArray(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();
	Local<Array> types = Array::New(isolate, SQLDataConverter::GetNumSolidTypes());
	SQLDataConverter::FetchAllName([&types, &isolate](int tid, const char* name) {
		types->Set(tid, String::NewFromUtf8(isolate, name));
	});
	args.GetReturnValue().Set(types);
}

// context-aware ³õÊ¼»¯
NODE_MODULE_INIT(/* exports, module, context */) {
	Isolate* isolate = context->GetIsolate();

	JSConnection::Init(exports->GetIsolate());

	exports->Set(context,
		String::NewFromUtf8(isolate, "createConnection", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, MySQLcreateConnection)->GetFunction(context).ToLocalChecked()
	).FromJust();

	exports->Set(context,
		String::NewFromUtf8(isolate, "retcodeToString", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, MySQLretcodeToString)->GetFunction(context).ToLocalChecked()
	).FromJust();

	exports->Set(context,
		String::NewFromUtf8(isolate, "fetchTypes", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, MySQLfetchTypes)->GetFunction(context).ToLocalChecked()
	).FromJust();

	exports->Set(context,
		String::NewFromUtf8(isolate, "fetchTypeNameArray", NewStringType::kNormal)
		.ToLocalChecked(),
		FunctionTemplate::New(isolate, MySQLfetchTypeNameArray)->GetFunction(context).ToLocalChecked()
	).FromJust();
}
#else
#include "build/connection.h"
#endif // ENABLE_V8

const char* return_code(RETCODE rc)
{
	switch (rc)
	{
	case SQL_SUCCESS:
		return "SQL_SUCCESS";
	case SQL_SUCCESS_WITH_INFO:
		return "SQL_SUCCESS_WITH_INFO";
	case SQL_ERROR:
		return "SQL_ERROR";
	case SQL_INVALID_HANDLE:
		return "SQL_INVALID_HANDLE";
	case SQL_NO_DATA:
		return "SQL_NO_DATA";
	case SQL_NEED_DATA:
		return "SQL_NEED_DATA";
	case SQL_STILL_EXECUTING:
		return "SQL_STILL_EXECUTING";
	}
	return "unknown";
}

// This is for debugging in c++
int main()
{
	Connection conn("library", "lib", "lib");
	auto ret = conn.connect();
	auto m = conn.query("select * from branch");
	getchar();
} 