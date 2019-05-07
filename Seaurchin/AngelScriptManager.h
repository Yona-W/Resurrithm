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

// メンバ関数を管理する
class MethodObject {
private:
	asIScriptContext* context;
	asIScriptObject* object;
	asIScriptFunction* function;

public:
	static MethodObject* Create(asIScriptObject* obj, const char* decl);

private:
	explicit MethodObject(asIScriptEngine* engine, asIScriptObject* object, asIScriptFunction* method);
public:
	~MethodObject();

public:
	void* SetUserData(void* data, asPWORD type) { return context->SetUserData(data, type); }

	int Prepare();
	bool SetArgument(std::function<int(asIScriptContext*)> f);
	int Execute();
	int ExecuteAsSuspendable();
	int Unprepare() { return context->Unprepare(); }
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
	static CallbackObject* Create(asIScriptFunction* callback);

private:
	explicit CallbackObject(asIScriptEngine* engine, asIScriptFunction* callback);
public:
	~CallbackObject();

	void AddRef() { ++refcount; }
	void Release() { if (--refcount == 0) delete this; }
	int GetRefCount() const { return refcount; }

	// オブジェクトが所有しているデリゲート「のみ」を解放する、オブジェクトそのものが解放されるわけではない
	// これを呼び出すとContext等は解放されるので参照しては行けない状態になる
	void Dispose();

	// オブジェクトが所有しているデリゲートが有効かどうかを返す
	// trueを返している間は登録したデリゲートを呼び出してよい
	// falseを返したなら速やかにこのオブジェクトをReleaseすることが望まれる(二重開放しないよう注意)
	bool IsExists() const { return exists; }

	void* SetUserData(void* data, asPWORD type)
	{
		BOOST_ASSERT(IsExists());
		return context->SetUserData(data, type);
	}

	int Prepare();
	bool SetArgument(std::function<int(asIScriptContext*)> f);
	int Execute();
	int Unprepare()
	{
		BOOST_ASSERT(IsExists());
		return context->Unprepare();
	}
};
