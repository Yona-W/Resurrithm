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
	engine->SetMessageCallback(asFUNCTION(ScriptMessageCallback), this, asCALL_THISCALL);
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
	const auto modulename = ConvertUnicodeToUTF8(file.wstring()).c_str();

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

asITypeInfo* AngelScript::GetEntryPointAsTypeInfo(asIScriptModule* module)
{
	if (!module) return nullptr;

	const int cnt = module->GetObjectTypeCount();

	for (auto i = 0; i < cnt; i++) {
		const auto cti = module->GetObjectTypeByIndex(i);
		asITypeInfo* type;

		if (cti->GetUserData(SU_UDTYPE_ENTRYPOINT)) return cti;

		if (strcmp(builder.GetMetadataStringForType(cti->GetTypeId()), "EntryPoint") == 0) {
			cti->SetUserData(reinterpret_cast<void*>(0xFFFFFFFF), SU_UDTYPE_ENTRYPOINT);
			return cti;
		}
	}

	return nullptr;
}

asIScriptFunction* AngelScript::GetEntryPointAsFunction(asIScriptModule* module)
{
	if (!module) return nullptr;

	const auto cnt = module->GetFunctionCount();

	for (asUINT i = 0; i < cnt; i++) {
		const auto func = module->GetFunctionByIndex(i);

		if (func->GetUserData(SU_UDTYPE_ENTRYPOINT)) return func;

		if (strcmp(builder.GetMetadataStringForFunc(func), "EntryPoint") == 0) {
			func->SetUserData(reinterpret_cast<void*>(0xFFFFFFFF), SU_UDTYPE_ENTRYPOINT);
			return func;
		}
	}

	return nullptr;
}

asIScriptObject* AngelScript::ExecuteScript(const path& root, const path& file, bool forceReload)
{
	const auto mod = GetModule(root, file, forceReload);
	asITypeInfo* type = GetEntryPointAsTypeInfo(mod);
	if (!type) {
		spdlog::get("main")->critical(u8"スクリプト \"{0}\" にEntryPointがありません。", ConvertUnicodeToUTF8(file));
		return nullptr;
	}

	type->AddRef();
	auto obj = InstantiateObject(type);

	type->Release();
	mod->Discard();
	return obj;
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

CallbackObject::CallbackObject(asIScriptFunction* callback)
	: exists(true)
	, refcount(1)
{
	const auto ctx = asGetActiveContext();
	auto engine = ctx->GetEngine();
	context = engine->CreateContext();
	function = callback->GetDelegateFunction();
	function->AddRef();
	object = static_cast<asIScriptObject*>(callback->GetDelegateObject());
	object->AddRef();
	type = callback->GetDelegateObjectType();
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
