#pragma once

class AngelScript {
private:
	asIScriptEngine* const engine;
	asIScriptContext* sharedContext;
	CWScriptBuilder builder;

public:
	AngelScript();
	~AngelScript();

	asIScriptEngine* GetEngine() const { return engine; }
	asIScriptContext* GetContext() const { return sharedContext; }


	//実装をチェック
	bool CheckImplementation(asITypeInfo* type, const std::string& name) const
	{
		return CheckImplementation(type, name.c_str());
	}
	bool CheckImplementation(asITypeInfo* type, const char* name) const
	{
		return type->Implements(engine->GetTypeInfoByName(name));
	}

	//asITypeInfoからインスタンス作成
	asIScriptObject* InstantiateObject(asITypeInfo* type) const;


	asIScriptModule* GetModule(const std::filesystem::path& root, const std::filesystem::path& file, bool forceReload);
	asITypeInfo* GetEntryPointAsTypeInfo(asIScriptModule* module);
	asIScriptFunction* GetEntryPointAsFunction(asIScriptModule* module);
	asIScriptObject* AngelScript::ExecuteScript(const std::filesystem::path& root, const std::filesystem::path& file, bool forceReload);
};

#define SU_DEF_SETARG_ALL(CONTEXT) \
    SU_DEF_SETARG_BYTE(UINT8, CONTEXT)\
    SU_DEF_SETARG_BYTE(INT8, CONTEXT)\
    SU_DEF_SETARG_WORD(UINT16, CONTEXT)\
    SU_DEF_SETARG_WORD(INT16, CONTEXT)\
    SU_DEF_SETARG_DWORD(UINT32, CONTEXT)\
    SU_DEF_SETARG_DWORD(INT32, CONTEXT)\
    SU_DEF_SETARG_QWORD(UINT64, CONTEXT)\
    SU_DEF_SETARG_QWORD(INT64, CONTEXT)\
    SU_DEF_SETARG_FLOAT(float, CONTEXT)\
    SU_DEF_SETARG_DOUBLE(double, CONTEXT)\
    SU_DEF_SETARG_ADDRESS(void *, CONTEXT)\
    SU_DEF_SETARG_ADDRESS(std::string *, CONTEXT)\
    SU_DEF_SETARG_OBJECT(void *, CONTEXT)


#define SU_DEF_SETARG_BEGIN template<typename T> int SetArg(asUINT, T) { static_assert(false, "テンプレート特殊化を実装すること"); }
#define SU_DEF_SETARG_BYTE(TYPE, CONTEXT) template<> int SetArg<TYPE>(asUINT arg, TYPE value) { return CONTEXT->SetArgByte(arg, value); }
#define SU_DEF_SETARG_WORD(TYPE, CONTEXT) template<> int SetArg<TYPE>(asUINT arg, TYPE value) { return CONTEXT->SetArgWord(arg, value); }
#define SU_DEF_SETARG_DWORD(TYPE, CONTEXT) template<> int SetArg<TYPE>(asUINT arg, TYPE value) { return CONTEXT->SetArgDWord(arg, value); }
#define SU_DEF_SETARG_QWORD(TYPE, CONTEXT) template<> int SetArg<TYPE>(asUINT arg, TYPE value) { return CONTEXT->SetArgQWord(arg, value); }
#define SU_DEF_SETARG_FLOAT(TYPE, CONTEXT) template<> int SetArg<TYPE>(asUINT arg, TYPE value) { return CONTEXT->SetArgFloat(arg, value); }
#define SU_DEF_SETARG_DOUBLE(TYPE, CONTEXT) template<> int SetArg<TYPE>(asUINT arg, TYPE value) { return CONTEXT->SetArgDouble(arg, value); }
#define SU_DEF_SETARG_ADDRESS(TYPE, CONTEXT) template<> int SetArg<TYPE>(asUINT arg, TYPE value) { return CONTEXT->SetArgAddress(arg, static_cast<void *>(value)); }
#define SU_DEF_SETARG_OBJECT(TYPE, CONTEXT) int SetArgObject(asUINT arg, TYPE value) { return CONTEXT->SetArgObject(arg, static_cast<void *>(value)); }
#define SU_DEF_SETARG_END 

// メンバ関数を管理する
class MethodObject {
private:
	asIScriptContext* context;
	asIScriptObject* object;
	asIScriptFunction* function;

public:
	explicit MethodObject(asIScriptEngine* engine, asIScriptObject* object, asIScriptFunction* method);
	~MethodObject();

	void* SetUserData(void* data, asPWORD type) { return context->SetUserData(data, type); }

	int Prepare()
	{
		const auto r1 = context->Prepare(function);
		if (r1 != asSUCCESS) return r1;
		return context->SetObject(object);
	}

	bool SetArgument(std::function<int(asIScriptContext*)> f) { /* TODO: ログ */ return f(context); }
	int Execute()
	{
		const auto result = context->Execute();

		switch (result) {
		case asEXECUTION_FINISHED: // 正常終了
			break;
		case asEXECUTION_SUSPENDED: // 中断
			spdlog::get("main")->error(u8"期待しない動作 : 関数実行が終了しませんでした。");
			break;
		case asEXECUTION_ABORTED: // Abort 現状起こりえない
			spdlog::get("main")->error(u8"期待しない動作 : 関数実行が強制終了しました。");
			break;
		case asEXECUTION_EXCEPTION:
		{
			int col;
			const char* at;
			const auto row = context->GetExceptionLineNumber(&col, &at);
			spdlog::get("main")->error(u8"{0} ({1:d}行{2:d}列) にて例外を検出しました : {3}", at, row, col, context->GetExceptionString());
			break;
		}
		default:
			spdlog::get("main")->error(u8"期待しない動作 : result as {0}.", result);
			break;
		}

		return result;
	}
	int ExecuteAsSuspendable()
	{
		const auto result = context->Execute();

		switch (result) {
		case asEXECUTION_FINISHED: // 正常終了
		case asEXECUTION_SUSPENDED: // 中断
			break;
		case asEXECUTION_ABORTED: // Abort 現状起こりえない
			spdlog::get("main")->error(u8"期待しない動作 : 関数実行が強制終了しました。");
			break;
		case asEXECUTION_EXCEPTION:
		{
			int col;
			const char* at;
			const auto row = context->GetExceptionLineNumber(&col, &at);
			spdlog::get("main")->error(u8"{0} ({1:d}行{2:d}列) にて例外を検出しました : {3}", at, row, col, context->GetExceptionString());
			break;
		}
		default:
			spdlog::get("main")->error(u8"期待しない動作 : result as {0}.", result);
			break;
		}

		return result;
	}

	int Unprepare() { return context->Unprepare(); }
	asIScriptContext* GetContext() const { return context; }
};

// コールバックを管理する
class CallbackObject {
private:
	asIScriptContext* context;
	asIScriptObject* object;
	asIScriptFunction* function;
	asITypeInfo* type;
	bool exists;
	int refcount;

public:
	explicit CallbackObject(asIScriptFunction* callback);
	~CallbackObject();

	void AddRef() { refcount++; }
	void Release() { if (--refcount == 0) delete this; }
	int GetRefCount() const { return refcount; }

	void* SetUserData(void* data, asPWORD type) {
		BOOST_ASSERT(IsExists());
		return context->SetUserData(data, type);
	}

	int Prepare()
	{
		BOOST_ASSERT(IsExists());
		const auto r1 = context->Prepare(function);
		if (r1 != asSUCCESS) return r1;
		return context->SetObject(object);
	}

	int Execute()
	{
		BOOST_ASSERT(IsExists());
		return context->Execute();
	}

	int Unprepare() {
		BOOST_ASSERT(IsExists());
		return context->Unprepare();
	}

	// オブジェクトが所有しているデリゲート「のみ」を解放する、オブジェクトそのものが解放されるわけではない
	// これを呼び出すとContext等は解放されるので参照しては行けない状態になる
	void Dispose();

	// オブジェクトが所有しているデリゲートが有効かどうかを返す
	// trueを返している間は登録したデリゲートを呼び出してよい
	// falseを返したなら速やかにこのオブジェクトをReleaseすることが望まれる(二重開放しないよう注意)
	bool IsExists() const { return exists; }

	SU_DEF_SETARG_BEGIN;
	SU_DEF_SETARG_ALL(context);
	SU_DEF_SETARG_END;
};
