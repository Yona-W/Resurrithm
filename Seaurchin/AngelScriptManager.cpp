#include "AngelScriptManager.h"

using namespace std;
static int ScriptIncludeCallback(const char *include, const char *from, CScriptBuilder *builder, void *userParam);

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
    builder.SetIncludeCallback(ScriptIncludeCallback, this);
}

AngelScript::~AngelScript()
{
    sharedContext->Release();
    engine->ShutDownAndRelease();
}

void AngelScript::StartBuildModule(const string &name, const IncludeCallback callback)
{
    includeFunc = callback;
    builder.StartNewModule(engine, name.c_str());
}

void AngelScript::LoadFile(const string &filename)
{
    builder.AddSectionFromFile(filename.c_str());
}

bool AngelScript::IncludeFile(const string &include, const string &from)
{
    return includeFunc(include, from, &builder);
}

bool AngelScript::FinishBuildModule()
{
    return builder.BuildModule() >= 0;
}

bool AngelScript::CheckMetaData(asITypeInfo *type, const string &meta)
{
    const auto df = builder.GetMetadataForType(type->GetTypeId());
    for(auto specified : df){
        if (specified == meta) return true;
    }
    return false;
}

bool AngelScript::CheckMetaData(asIScriptFunction *func, const string &meta)
{
    const auto df = builder.GetMetadataForFunc(func);
    for(auto specified : df){
        if (specified == meta) return true;
    }
    return false;
}

asIScriptObject *AngelScript::InstantiateObject(asITypeInfo * type) const
{
    const auto factory = type->GetFactoryByIndex(0);
    sharedContext->Prepare(factory);
    sharedContext->Execute();
    const auto ptr = *static_cast<asIScriptObject**>(sharedContext->GetAddressOfReturnValue());
    ptr->AddRef();
    sharedContext->Unprepare();

    return ptr;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void AngelScript::ScriptMessageCallback(const asSMessageInfo * message) const
{
    using namespace std;
    
    switch (message->type) {
        case asMSGTYPE_INFORMATION:
            spdlog::info(u8"{0} ({1:d}行{2:d}列): {3}", message->section, message->row, message->col, message->message);
            break;
        case asMSGTYPE_WARNING:
            spdlog::warn(u8"{0} ({1:d}行{2:d}列): {3}", message->section, message->row, message->col, message->message);
            break;
        case asMSGTYPE_ERROR:
            spdlog::error(u8"{0} ({1:d}行{2:d}列): {3}", message->section, message->row, message->col, message->message);
            break;
    }
}

int ScriptIncludeCallback(const char *include, const char *from, CScriptBuilder *builder, void *userParam)
{
    auto as = reinterpret_cast<AngelScript*>(userParam);
    return as->IncludeFile(include, from) ? 1 : -1;
}

MethodObject::MethodObject(asIScriptEngine *engine, asIScriptObject *object, asIScriptFunction *method)
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

CallbackObject::CallbackObject(asIScriptFunction *callback)
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
