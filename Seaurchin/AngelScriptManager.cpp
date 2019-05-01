#include "AngelScriptManager.h"

using namespace std;

AngelScript::AngelScript()
	: engine(asCreateScriptEngine())
{
	engine->SetMessageCallback(asMETHOD(AngelScript, ScriptMessageCallback), this, asCALL_THISCALL);
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

void AngelScript::StartBuildModule(const string& name)
{
	builder.StartNewModule(engine, name.c_str());
}

void AngelScript::LoadFile(const filesystem::path& path)
{
	builder.AddSectionFromFile(path.wstring().c_str());
}

bool AngelScript::FinishBuildModule()
{
	return builder.BuildModule() >= 0;
}

bool AngelScript::CheckMetaData(asITypeInfo* type, const string& meta)
{
	const auto df = builder.GetMetadataStringForType(type->GetTypeId());
	return df == meta;
}

bool AngelScript::CheckMetaData(asIScriptFunction* func, const string& meta)
{
	const auto df = builder.GetMetadataStringForFunc(func);
	return df == meta;
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

// ReSharper disable once CppMemberFunctionMayBeStatic
void AngelScript::ScriptMessageCallback(const asSMessageInfo* message) const
{
	using namespace std;
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
