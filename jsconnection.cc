#include "jsconnection.h"
#include <iostream>
#include <sstream>
#include <time.h>
#define v8str(_str_) String::NewFromUtf8(isolate, _str_)

class SQLToV8Converter : public ISQLDataVisitor {
public:
	SQLToV8Converter(std::vector<Local<String>>& col_, Local<Object>& obj_, Isolate* isolate_)
		:col(col_), obj(obj_), isolate(isolate_), n_row(0) {}
	void visit(SQLInt* i) override {

		obj->
			Set(col[n_row], Int32::New(isolate, i->Value()));
	}

	void visit(SQLBigInt* i) override {
		obj->Set(col[n_row], BigInt::New(isolate, i->Value()));
	}

	void visit(SQLDouble* i) override {
		obj->Set(col[n_row], Number::New(isolate, i->Value()));
	}

	void visit(SQLFloat* i) override {
		obj->Set(col[n_row], Number::New(isolate, i->Value()));
	}

	void visit(SQLNumeric* i) override {
		auto val = i->Value();
		Local<Object> t = Object::New(isolate);
		t->Set(v8str("integer"), BigInt::New(isolate, val.integer));
		t->Set(v8str("decimal"), BigInt::New(isolate, val.decimal));
		t->Set(v8str("scale"), Int32::New(isolate, val.scale));
		obj->Set(col[n_row], t);
	}

	void visit(SQLString *i) override {
		obj->Set(col[n_row], String::NewFromUtf8(isolate, i->Value()));
	}

	void visit(SQLTime *i) override {
		Local<Object> t = Object::New(isolate);
		buildTime(t, i->Value());
		obj->Set(col[n_row], t);
	}

	void visit(SQLDate *i) override {
		Local<Object> t = Object::New(isolate);
		buildDate(t, i->Value());
		obj->Set(col[n_row], t);
	}

	void visit(SQLTimeStamp *i) override {
		Local<Object> t = Object::New(isolate);
		buildDate(t, i->Value().date);
		buildTime(t, i->Value().time);
		t->Set(v8str("fraction"), Int32::New(isolate, i->Value().fraction));
		obj->Set(col[n_row], t);
	}
	void visit(SQLNull*) override { obj->Set(col[n_row], Null(isolate)); }

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
	NODE_SET_PROTOTYPE_METHOD(tpl, "prepare", prepare);

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
				Exception::TypeError( v8str("Error: expected callback  for second parameter") ));
			return;
		}
		queryDirect(args, args[0]->ToString(), 1);
	}
	else if (args[0]->IsObject()) {
		if (args.Length() < 2 || !args[1]->IsArray()) {
			isolate->ThrowException(Exception::TypeError(
				v8str("Error: expected array of paramters to bind for 2nd parameter")
			));
			return;
		}

		if (args.Length() < 3 || !args[2]->IsFunction()) {
			isolate->ThrowException(Exception::TypeError(
				v8str("Error: expected callback to bind for 3rd parameter")
			));
			return;
		}
		queryPrepared(args);
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

void JSConnection::prepare(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();
	JSConnection* self = ObjectWrap::Unwrap<JSConnection>(args.Holder());
	if (args.Length() < 1 || !args[0]->IsString()) {
		isolate->ThrowException(Exception::TypeError(
			v8str("Error: expected string for first parameter")
		));
		return;
	}
	Local<String> lstmt = args[0]->ToString();
	String::Utf8Value stmt(isolate, lstmt);
	Connection::QueryHandle qh = self->conn_.prepare(*stmt);
	Local<Object> res = Object::New(isolate);
	if (qh.idx < 0) {
		res->Set(v8str("err"), errorsToV8Object(qh.errors, isolate));
	}
	res->Set(v8str("index"), Int32::New(isolate, qh.idx));
	res->Set(v8str("num_params"), Int32::New(isolate, qh.num_params));
	res->Set(v8str("retcode"), Int32::New(isolate, qh.retcode));
	args.GetReturnValue().Set(res);
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
		field->Set(v8str("type"), Int32::New(isolate, SQLDataConverter::SQLTypeDecay(col.type)));
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

void JSConnection::queryPrepared(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	JSConnection* self = ObjectWrap::Unwrap<JSConnection>(args.Holder());
	Connection::QueryHandle handle;
	Local<Object> h = args[0]->ToObject(context).ToLocalChecked();
	auto hidx = h->Get(v8str("index"));
	if (!hidx->IsInt32()) {
		isolate->ThrowException(Exception::TypeError(
			v8str("Error: 1st param is not a valid handle")
		));
		return;
	}

	handle.idx = hidx->ToInt32(context).ToLocalChecked()->Value();
	handle.num_params = h->Get(v8str("num_params"))->ToInt32(context).ToLocalChecked()->Value();
	handle.retcode = h->Get(v8str("retcode"))->ToInt32(context).ToLocalChecked()->Value();

	Local<Array> params = Local<Array>::Cast(args[1]);
	if (params->Length() != handle.num_params) {
		std::stringstream ss;
		ss << "Error: Expected " << handle.num_params << " to bind, yet " << params->Length() << " provided";
		std::string str = ss.str();
		isolate->ThrowException(Exception::TypeError(
			String::NewFromUtf8(isolate, str.c_str())
		));
		return;
	}

	auto& conn = self->conn_;
	Local<Int32> uu;
	std::vector<double> nums;
	std::vector<int64_t> int_nums;
	std::vector<std::string> u8;
	std::vector<SQLTimeStampStruct> time_stamp;
	SQLNullData null_data;
	nums.reserve(params->Length());
	u8.reserve(params->Length());
	int_nums.reserve(params->Length());
	time_stamp.reserve(params->Length());
	for (size_t i = 0; i < params->Length(); ++i) {
		auto r = params->Get(i);
		// String
		if (r->IsString()) {
			u8.push_back(*String::Utf8Value(isolate, r));
			conn.bindParam(handle, i + 1, u8.back());
		}
		// NUll
		else if (r->IsNull()) {
			conn.bindParam(handle, i + 1, null_data);
		}
		//Big Int
		else if (r->IsBigInt()) {
			int_nums.push_back(r->ToBigInt(context).ToLocalChecked()->Int64Value());
			conn.bindParam(handle, i + 1, &int_nums.back());
		}
		// Int, float, double, bigint, float, numeric
		else if (r->IsNumber()) {
			nums.push_back(r->NumberValue(context).ToChecked());
			conn.bindParam(handle, i + 1, &nums.back());
		}
		else if (r->IsObject()) {
			
			Local<Object> o = r->ToObject(context).ToLocalChecked();
#define GET_D(n) \
				auto maybe = o->Get(context, v8str(n)); \
				if (maybe.IsEmpty()) return false; \
				auto int32_maybe = maybe.ToLocalChecked()->ToInt32(context); \
				if (int32_maybe.IsEmpty()) return false; \
				int res = int32_maybe.ToLocalChecked()->Value()

			auto date_get = [&context, &o, &isolate](SQLDateStruct& date) -> bool {
				{
					GET_D("year");
					date.year = res;
				}
				{
					GET_D("date");
					date.date = res;
				}
				{
					GET_D("month");
					date.month = res;
				}
				return true;
			};
			auto time_get = [&context, &o, &isolate](SQLTimeStruct & date) -> bool {
				{
					GET_D("hour");
					date.hour = res;
				}
				{
					GET_D("minute");
					date.minute = res;
				}
				{
					GET_D("second");
					date.second = res;
				}
				return true;
			};
#undef GET_D
			// numeric
			if (o->Has(v8str("scale"))) {
				
			}
			// timestmp
			else if (o->Has(v8str("fraction"))) {
				time_stamp.push_back(SQLTimeStampStruct{});
				if (!time_get(*&time_stamp.back().time) || !date_get(*&time_stamp.back().date)) {
					isolate->ThrowException(Exception::TypeError(
						v8str("Error: Expected timestamp with year, month, date, hour, minute, second")
					));
					return;
				}
			}
			// date
			else if (o->Has(v8str("date"))) {
				time_stamp.push_back(SQLTimeStampStruct{});
				if (!date_get(*&time_stamp.back().date)) {
					isolate->ThrowException(Exception::TypeError(
						v8str("Error: Expected date with year, month, date")
					));
					return;
				}
				conn.bindParam(handle, i + 1, &time_stamp.back().date);
			}
			// time or else
			else {
				time_stamp.push_back(SQLTimeStampStruct{});
				if (!time_get(*&time_stamp.back().time)) {
					isolate->ThrowException(Exception::TypeError(
						v8str("Error: Expected time with hour, minute, second")
					));
					return;
				}
				conn.bindParam(handle, i + 1, &time_stamp.back().date);
			}
		}
		if (handle.retcode != 0) {
			Local<Object> err = Object::New(isolate);
			Local<Array> arr = Array::New(isolate);
			err->Set(v8str("retcode"), Int32::New(isolate, handle.retcode));
			err->Set(v8str("err"), errorsToV8Object(handle.errors, isolate));
			Local<Function> cb = Local<Function>::Cast(args[2]);
			Local<Value> argv[3] = { err, arr, arr };
			auto maybe_local = cb->Call(context, Null(isolate), 3, argv);
			if (!maybe_local.IsEmpty()) maybe_local.ToLocalChecked();
			return;
		}
	}
	
	auto res = self->conn_.queryPrepared(handle);

	queryExecuteHelper(args, *res, 2);

}



void JSConnection::c_internal_test(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Isolate* isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	JSConnection* self = ObjectWrap::Unwrap<JSConnection>(args.Holder());
	auto res = self->conn_.query("show tables");
	int a = 0;
}


#undef v8str