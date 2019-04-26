#pragma once

typedef std::function<bool(std::wstring, std::wstring, CWScriptBuilder*)> IncludeCallback;

class AngelScript {
private:
	asIScriptEngine* const engine;
	asIScriptContext* sharedContext;
	CWScriptBuilder builder;
	IncludeCallback includeFunc;

	void ScriptMessageCallback(const asSMessageInfo* message) const;

public:
	AngelScript();
	~AngelScript();

	asIScriptEngine* GetEngine() const { return engine; }
	asIScriptContext* GetContext() const { return sharedContext; }

	//新しくModuleする
	void StartBuildModule(const std::string& name, IncludeCallback callback);

	asIScriptModule* GetExistModule(std::string name) const { return engine->GetModule(name.c_str()); }

	//ファイル読み込み
	void LoadFile(const std::wstring& filename);

	//外から使わないで
	bool IncludeFile(const std::wstring& include, const std::wstring& from);

	//ビルドする
	bool FinishBuildModule();

	//FinishしたModuleを取得
	asIScriptModule* GetLastModule() { return builder.GetModule(); }

	//特定クラスにメタデータが付与されてるか
	bool CheckMetaData(asITypeInfo* type, const std::string& meta);

	//特定グロ関に(ry
	bool CheckMetaData(asIScriptFunction* type, const std::string& meta);


	//実装をチェック
	bool CheckImplementation(asITypeInfo* type, std::string name) const
	{
		return type->Implements(engine->GetTypeInfoByName(name.c_str()));
	}

	//asITypeInfoからインスタンス作成
	asIScriptObject* InstantiateObject(asITypeInfo* type) const;
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

	int Execute() { return context->Execute(); }
	int Unprepare() { return context->Unprepare(); }

	SU_DEF_SETARG_BEGIN;
	SU_DEF_SETARG_ALL(context);
	SU_DEF_SETARG_END;
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
