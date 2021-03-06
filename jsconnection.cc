#include "jsconnection.h"
#include <iostream>
#include <sstream>
#include <time.h>
#define v8str(_str_) String::NewFromUtf8(isolate, _str_)

class SQLToV8Converter : public ISQLDataVisitor {
public:
	SQLToV8Converter(std::vector<Local<String>>& col_, Local<Object>& obj_, Isolate* isolate_)
		:col(col_), obj(obj_), isolate(isolate_), n_row(0) {}
	// void visit(SQLInt* i) override {
	// 
	// 	obj->
	// 		Set(col[n_row], Int32::New(isolate, i->Value()));
	// }

	void Visit(SQLBigInt* i) override {
		obj->Set(col[n_row], BigInt::New(isolate, i->Value()));
	}

	void Visit(SQLDouble* i) override {
		obj->Set(col[n_row], Number::New(isolate, i->Value()));
	}

	// void visit(SQLFloat* i) override {
	// 	obj->Set(col[n_row], Number::New(isolate, i->Value()));
	// }
	// 
	// void visit(SQLNumeric* i) override {
	// 	auto val = i->Value();
	// 	Local<Object> t = Object::New(isolate);
	// 	t->Set(v8str("integer"), BigInt::New(isolate, val.integer));
	// 	t->Set(v8str("decimal"), BigInt::New(isolate, val.decimal));
	// 	t->Set(v8str("scale"), Int32::New(isolate, val.scale));
	// 	obj->Set(col[n_row], t);
	// }

	void Visit(SQLString *i) override {
		obj->Set(col[n_row], String::NewFromUtf8(isolate, i->Value()));
	}

	// void visit(SQLTime *i) override {
	// 	Local<Object> t = Object::New(isolate);
	// 	buildTime(t, i->Value());
	// 	obj->Set(col[n_row], t);
	// }
	// 
	// void visit(SQLDate *i) override {
	// 	Local<Object> t = Object::New(isolate);
	// 	buildDate(t, i->Value());
	// 	obj->Set(col[n_row], t);
	// }

	void Visit(SQLTimeStamp *i) override {
		Local<Object> t = Object::New(isolate);
		buildDate(t, i->Value().date);
		buildTime(t, i->Value().time);
		t->Set(v8str("fraction"), Int32::New(isolate, i->Value().fraction));
		obj->Set(col[n_row], t);
	}
	void Visit(SQLNull*) override { obj->Set(col[n_row], Null(isolate)); }

	void buildDate(Local<Object>& t, const SQLDateStruct& d) {
		t->Set(v8str("year"), Int32::New(isolate, d.year));
		t->Set(v8str("month"), Int32::New(isolate, d.month));
		t->Set(v8str("date"), Int32::New(isolate, d.date));
	}

	void buildTime(Local<Object>& t, const SQLTimeStruct& d) {
		t->Set(v8str("hour"), Int32::New(isolate, d.hour));
		t->Set(v8str("minute"), Int32::New(isolate, d.minute));
		t->Set(v8str("second"), Int32::New(isolate, d.second));
	}

	void next() { ++n_row; }
	std::vector<Local<String>>& col;
	Local<Object>& obj;
	Isolate* isolate;
	size_t n_row;
};

Persistent<Function> JSConnection::constructor;
void JSConnection::Init(Isolate* isolate) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(v8str("SQLConnection"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	NODE_SET_PROTOTYPE_METHOD(tpl, "connect", connect);
	NODE_SET_PROTOTYPE_METHOD(tpl, "query", query);
	// NODE_SET_PROTOTYPE_METHOD(tpl, "prepare", prepare);

	NODE_SET_PROTOTYPE_METHOD(tpl, "c_internal_test", c_internal_test);
	constructor.Reset(isolate, tpl->GetFunction());
}

void JSConnection::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	if (args.Length() < 1 || !args[0]->IsObject()) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Error: One object expected")));
		return;
	}

	Local<Context> context = isolate->GetCurrentContext();
	Local<Object> config = args[0]->ToObject(context).ToLocalChecked();

	Local<Value> luser = config->Get(v8str("user"));
	Local<Value> lpassword = config->Get(v8str("password"));
	Local<Value> ldatabase = config->Get(v8str("database"));
	String::Utf8Value user(isolate, luser);
	String::Utf8Value password(isolate, lpassword);
	String::Utf8Value database(isolate, ldatabase);

	if (args.IsConstructCall()) {
		// 像构造函数一样调用：`new MyObject(...)`
		double value = args[0]->IsUndefined() ? 0 : Local<Number>::Cast(args[0])->Value();
		JSConnection* obj = new JSConnection(Connection(*database, *user, *password));
		obj->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	else {
		// 像普通方法 `MyObject(...)` 一样调用，转为构造调用。
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		Local<Context> context = isolate->GetCurrentContext();
		Local<Object> instance =
			cons->NewInstance(context, argc, argv).ToLocalChecked();
		args.GetReturnValue().Set(instance);
	}
}

void JSConnection::NewInstance(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	const unsigned argc = 1;
	Local<Value> argv[argc] = { args[0] };
	Local<Function> cons = Local<Function>::New(isolate, constructor);
	Local<Context> context = isolate->GetCurrentContext();
	Local<Object> instance =
		cons->NewInstance(context, argc, argv).ToLocalChecked();

	args.GetReturnValue().Set(instance);
}

void JSConnection::connect(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();

	JSConnection* self = ObjectWrap::Unwrap<JSConnection>(args.Holder());
	auto retcode = self->conn_.connect();
	args.GetReturnValue().Set(retcode);
}

void JSConnection::query(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	JSConnection* self = ObjectWrap::Unwrap<JSConnection>(args.Holder());

	if (args.Length() < 1) {
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, "Error: expected string, handle or array for first parameter")
		));
		return;
	}

	if (args[0]->IsString()) {
		if (args.Length() < 2 || !args[1]->IsFunction()) {
			isolate->ThrowException(
				Exception::TypeError(v8str("Error: expected callback  for second parameter")));
			return;
		}
		queryDirect(args, args[0]->ToString(), 1);
	}
	else {
		isolate->ThrowException(Exception::TypeError(
			v8str("Error: expected string or handle for first parameter")
		));
		return;
	}
}

Local<Object> errorsToV8Object(const Connection::ErrorDetail& e, Isolate* isolate) {
	Local<Object> eo = Object::New(isolate);
	eo->Set(v8str("native_err"), Int32::New(isolate, e.native_err));
	eo->Set(v8str("errcode"), v8str(e.errcode.c_str()));
	eo->Set(v8str("description"), v8str(e.description.c_str()));
	return eo;
}


void queryExecuteHelper(const v8::FunctionCallbackInfo<v8::Value>& args, Connection::QueryResult&res, int func_id) {
	Isolate* isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();

	Local<Function> cb = Local<Function>::Cast(args[func_id]);
	Local<Object> err = Object::New(isolate);
	err->Set(v8str("retcode"), Int32::New(isolate, res.retcode));

	if (res.errors.size()) {
		Local<Array> details = Array::New(isolate, res.errors.size());
		int i = 0;
		for (auto& e : res.errors) {
			details->Set(i, errorsToV8Object(e, isolate));
			++i;
		}
		err->Set(v8str("details"), details);
	}
	

	Local<Array> rows = Array::New(isolate, res.num_rows);
	std::vector<Local<String>>  col_desc;
	Local<Array> fields = Array::New(isolate, res.col_desc.size());
	size_t i = 0;
	for (auto& col : res.col_desc) {
		col_desc.push_back(String::NewFromUtf8(isolate, col.name.c_str()));
		Local<Object> field = Object::New(isolate);
		field->Set(v8str("columnname"), col_desc[i]);
		// field->Set(v8str("type"), Int32::New(isolate, SQLDataConverter::SQLTypeDecay(col.type)));
		field->Set(v8str("nullable"), Boolean::New(isolate, col.nullable));
		fields->Set(i, field);
		++i;
	}
	i = 0;
	for (auto& row : res.data) {
		HandleScope scope(isolate);
		Local<Object> obj = Object::New(isolate);
		SQLToV8Converter cvt(col_desc, obj, isolate);
		for (auto& col : row) {
			col->Accept(&cvt);
			cvt.next();
		}
		rows->Set(i, obj);
		++i;
	}

	Local<Value> argv[3] = { err, rows, fields };
	auto maybe_local = cb->Call(context, Null(isolate), 3, argv);
	if (!maybe_local.IsEmpty()) maybe_local.ToLocalChecked();
}


void JSConnection::queryDirect(const v8::FunctionCallbackInfo<v8::Value>& args, Local<String> lq, int func_id)
{
	Isolate* isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	JSConnection* self = ObjectWrap::Unwrap<JSConnection>(args.Holder());

	String::Utf8Value q(isolate, lq);
	auto res = self->conn_.query(*q);

	queryExecuteHelper(args, *res, func_id);
}





void JSConnection::c_internal_test(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	JSConnection* self = ObjectWrap::Unwrap<JSConnection>(args.Holder());
	auto res = self->conn_.query("show tables");
	int a = 0;
}


#undef v8str