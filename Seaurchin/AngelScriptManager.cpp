#include "AngelScriptManager.h"

using namespace std;
using namespace filesystem;

namespace {
	void ScriptMessageCallback(const asSMessageInfo* message)
	{
		auto log = spdlog::get("main");
		switch (message->type) {
		case asMSGTYPE_INFORMATION:
			log->info(u8"{0} ({1:d}行{2:d}列): {3}", message->section, message->row, message->col, message->message);
			break;
		case asMSGTYPE_WARNING:
			log->warn(u8"{0} ({1:d}行{2:d}列): {3}", message->section, message->row, message->col, message->message);
			break;
		case asMSGTYPE_ERROR:
			log->error(u8"{0} ({1:d}行{2:d}列): {3}", message->section, message->row, message->col, message->message);
			break;
		}
	}
}

AngelScript::AngelScript()
	: engine(asCreateScriptEngine())
{
	engine->SetMessageCallback(asFUNCTION(ScriptMessageCallback), this, asCALL_CDECL);
	RegisterScriptMath(engine);
	RegisterScriptArray(engine, true);
	RegisterStdString(engine);
	RegisterStdStringUtils(engine);
	RegisterScriptDictionary(engine);

	//Script Interface
	sharedContext = engine->CreateContext();
}

AngelScript::~AngelScript()
{
	sharedContext->Release();
	engine->ShutDownAndRelease();
}

asIScriptObject* AngelScript::InstantiateObject(asITypeInfo* type) const
{
	const auto factory = type->GetFactoryByIndex(0);
	sharedContext->Prepare(factory);
	sharedContext->Execute();
	const auto ptr = *static_cast<asIScriptObject * *>(sharedContext->GetAddressOfReturnValue());
	ptr->AddRef();
	sharedContext->Unprepare();

	return ptr;
}

asIScriptModule* AngelScript::GetModule(const path& root, const path& file, bool forceReload)
{
	const auto modulename = ConvertUnicodeToUTF8(file).c_str();

	if (!forceReload) {
		const auto mod = engine->GetModule(modulename);
		if (mod) return mod;
	}

	const auto path = root / file;
	if (!exists(path)) {
		spdlog::get("main")->error(u8"スクリプトファイル \"{0}\" が見つかりませんでした。", modulename);
		return nullptr;
	}

	builder.StartNewModule(engine, modulename);
	if (builder.AddSectionFromFile(path.c_str()) < 0) {
		spdlog::get("main")->error(u8"スクリプトファイル \"{0}\" の読み込みに失敗しました。", modulename);
		return nullptr;
	}
	if (builder.BuildModule() < 0) {
		builder.GetModule()->Discard();
		spdlog::get("main")->error(u8"スクリプトファイル \"{0}\" のビルドに失敗しました。", modulename);
		return nullptr;
	}

	return builder.GetModule();
}

asIScriptObject* AngelScript::ExecuteScriptAsObject(const path& root, const path& file, bool forceReload)
{
	const auto mod = GetModule(root, file, forceReload);
	if (!mod) return nullptr;

	asITypeInfo* type = nullptr;
	const auto cnt = mod->GetObjectTypeCount();
	for (auto i = 0u; i < cnt; i++) {
		const auto cti = mod->GetObjectTypeByIndex(i);

		if (cti->GetUserData(SU_UDTYPE_ENTRYPOINT)) {
			type = cti;
			break;
		}
		else if (strcmp(builder.GetMetadataStringForType(cti->GetTypeId()), "EntryPoint") == 0) {
			cti->SetUserData(reinterpret_cast<void*>(0xFFFFFFFF), SU_UDTYPE_ENTRYPOINT);
			type = cti;
			break;
		}

		cti->Release();
	}

	asIScriptObject* obj = nullptr;
	if (!type) {
		spdlog::get("main")->critical(u8"スクリプト \"{0}\" にEntryPointがありません。", ConvertUnicodeToUTF8(file));
	}
	else {
		type->AddRef();
		obj = InstantiateObject(type);
	}

	if (type) type->Release();
	mod->Discard();
	return obj;
}

asIScriptFunction* AngelScript::ExecuteScriptAsFunction(const path& root, const path& file, bool forceReload)
{
	const auto mod = GetModule(root, file, forceReload);
	if (!mod) return nullptr;

	asIScriptFunction* func = nullptr;
	const auto cnt = mod->GetFunctionCount();
	for (asUINT i = 0; i < cnt; i++) {
		const auto f = mod->GetFunctionByIndex(i);

		if (f->GetUserData(SU_UDTYPE_ENTRYPOINT)) {
			func = f;
			break;
		}
		else if (strcmp(builder.GetMetadataStringForFunc(f), "EntryPoint") == 0) {
			f->SetUserData(reinterpret_cast<void*>(0xFFFFFFFF), SU_UDTYPE_ENTRYPOINT);
			func = f;
			break;
		}
	}

	if (!func) {
		spdlog::get("main")->critical(u8"スクリプト \"{0}\" にEntryPointがありません。", ConvertUnicodeToUTF8(file));
	}
	else {
		func->AddRef();
	}

	mod->Discard();
	return func;
}


FunctionObject* FunctionObject::Create(asIScriptFunction* func)
{
	if (!func) return nullptr;

	auto engine = func->GetEngine();

	func->AddRef();
	const auto ptr = new FunctionObject(engine, func);

	func->Release();
	return ptr;
}

FunctionObject::FunctionObject(asIScriptEngine * engine, asIScriptFunction * func)
	: context(engine->CreateContext())
	, function(func)
{}

FunctionObject::~FunctionObject()
{
	context->Release();
	function->Release();
}

int FunctionObject::Prepare()
{
	return context->Prepare(function);
}

bool FunctionObject::SetArgument(std::function<int(asIScriptContext*)> f)
{
	/* TODO: ログ */
	return f(context);
}

int FunctionObject::Execute()
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


MethodObject* MethodObject::Create(asIScriptObject* obj, const char* decl)
{
	if (!obj || !decl) return nullptr;

	auto engine = obj->GetEngine();
	auto func = obj->GetObjectType()->GetMethodByDecl(decl);
	if (!func) return nullptr;

	obj->AddRef();
	func->AddRef();
	const auto ptr = new MethodObject(engine, obj, func);

	obj->Release();
	return ptr;
}

MethodObject::MethodObject(asIScriptEngine* engine, asIScriptObject* object, asIScriptFunction* method)
	: context(engine->CreateContext())
	, object(object)
	, function(method)
{}

MethodObject::~MethodObject()
{
	context->Release();
	object->Release();
	function->Release();
}

int MethodObject::Prepare()
{
	const auto r1 = context->Prepare(function);
	if (r1 != asSUCCESS) return r1;
	return context->SetObject(object);
}

bool MethodObject::SetArgument(std::function<int(asIScriptContext*)> f)
{
	/* TODO: ログ */
	return f(context);
}

int MethodObject::Execute()
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

int MethodObject::ExecuteAsSuspendable()
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



CallbackObject* CallbackObject::Create(asIScriptFunction* callback)
{
	if (!callback || callback->GetFuncType() != asFUNC_DELEGATE) return nullptr;

	callback->AddRef();
	const auto ptr = new CallbackObject(callback->GetEngine(), callback);

	callback->Release();
	return ptr;
}

CallbackObject::CallbackObject(asIScriptEngine* engine, asIScriptFunction* callback)
	: context(engine->CreateContext())
	, object(static_cast<asIScriptObject*>(callback->GetDelegateObject()))
	, function(callback->GetDelegateFunction())
	, type(callback->GetDelegateObjectType())
	, exists(true)
	, refcount(1)
{
	function->AddRef();
	object->AddRef();
	type->AddRef();

	callback->Release();
}

CallbackObject::~CallbackObject()
{
	Dispose();
}

void CallbackObject::Dispose()
{
	if (!exists) return;

	auto engine = context->GetEngine();
	context->Release();
	function->Release();
	engine->ReleaseScriptObject(object, type);
	type->Release();

	exists = false;
}

int CallbackObject::Prepare()
{
	SU_ASSERT(IsExists());
	const auto r1 = context->Prepare(function);
	if (r1 != asSUCCESS) return r1;
	return context->SetObject(object);
}

bool CallbackObject::SetArgument(std::function<int(asIScriptContext*)> f)
{
	/* TODO: ログ */
	return f(context);
}

int CallbackObject::Execute()
{
	SU_ASSERT(IsExists());

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
