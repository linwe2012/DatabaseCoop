#pragma once
#include "connection.h"
#pragma warning( push )
#pragma warning( disable : 26495)
#pragma warning( disable : 26451)
#include "node.h"
#include "node_object_wrap.h"
#pragma warning( pop )

using namespace v8;
class JSConnection : public node::ObjectWrap {
public:
	JSConnection(Connection conn) :conn_(conn) {};
	static void Init(v8::Isolate* isolate);
	static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
	static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void connect(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void query(const v8::FunctionCallbackInfo<v8::Value>& args);
	// static void prepare(const v8::FunctionCallbackInfo<v8::Value>& args);
	// static void queryDirect(const v8::FunctionCallbackInfo<v8::Value>& args, Local<String> lq, int func_id);
	// static void queryPrepared(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void c_internal_test(const v8::FunctionCallbackInfo<v8::Value>& args);
	static Persistent<Function> constructor;
	Connection conn_;
};