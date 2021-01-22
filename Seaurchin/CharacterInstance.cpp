﻿#include "CharacterInstance.h"
#include "Misc.h"
#include "Setting.h"
#include "Config.h"
#include "ScriptScene.h"
#include <angelscript.h>

using namespace std;

CharacterInstance::CharacterInstance(const shared_ptr<CharacterParameter>& character, const shared_ptr<SkillParameter>& skill, const shared_ptr<AngelScript>& script, const shared_ptr<Result>& result)
    : scriptInterface(script)
    , characterSource(character)
    , skillSource(skill)
    , imageSet(nullptr)
    , indicators(new SkillIndicators())
    , targetResult(result)
    , context(script->GetEngine()->CreateContext())
    , judgeCallback(nullptr)
{}

CharacterInstance::~CharacterInstance()
{
    if (judgeCallback) judgeCallback->Release();
    context->Release();
    for (const auto &t : abilityTypes) t->Release();
    for (const auto &o : abilities) o->Release();
    if (imageSet) imageSet->Release();
}

shared_ptr<CharacterInstance> CharacterInstance::CreateInstance(shared_ptr<CharacterParameter> character, shared_ptr<SkillParameter> skill, shared_ptr<AngelScript> script, std::shared_ptr<Result> result)
{
    auto ci = make_shared<CharacterInstance>(character, skill, script, result);
    ci->LoadAbilities();
    ci->CreateImageSet();
    ci->AddRef();
    return ci;
}

void CharacterInstance::LoadAbilities()
{
    using namespace boost::algorithm;
    using namespace boost::filesystem;
    auto log = spdlog::get("main");
    const auto abroot = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_ABILITY_DIR;

    const auto &abilities = skillSource->GetDetail(skillSource->CurrentLevel).Abilities;
    for (const auto &def : abilities) {
        vector<string> params;
        auto scrpath = abroot / ConvertUTF8ToUnicode(def.Name + ".as");

        auto abo = LoadAbilityObject(scrpath);
        if (!abo) continue;
        auto abt = abo->GetObjectType();
        abt->AddRef();
        AbilityFunctions funcs;
        funcs.OnStart = abt->GetMethodByDecl("void OnStart(" SU_IF_RESULT "@)");
        funcs.OnFinish = abt->GetMethodByDecl("void OnFinish(" SU_IF_RESULT "@)");
        funcs.OnJusticeCritical = abt->GetMethodByDecl("void OnJusticeCritical(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");
        funcs.OnJustice = abt->GetMethodByDecl("void OnJustice(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");
        funcs.OnAttack = abt->GetMethodByDecl("void OnAttack(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");
        funcs.OnMiss = abt->GetMethodByDecl("void OnMiss(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");
        this->abilities.push_back(abo);
        abilityTypes.push_back(abt);
        abilityEvents.push_back(funcs);

        const auto init = abt->GetMethodByDecl("void Initialize(dictionary@, " SU_IF_SKILL_INDICATORS "@)");
        if (!init) continue;

        auto args = CScriptDictionary::Create(scriptInterface->GetEngine());
        for (const auto &arg : def.Arguments) {
            const auto key = arg.first;
            auto value = arg.second;
            auto &vid = value.type();
            if (vid == typeid(int)) {
                asINT64 avalue = boost::any_cast<int>(value);
                args->Set(key, avalue);
            } else if (vid == typeid(double)) {
                auto avalue = boost::any_cast<double>(value);
                args->Set(key, avalue);
            } else if (vid == typeid(string)) {
                auto avalue = boost::any_cast<string>(value);
                args->Set(key, &avalue, scriptInterface->GetEngine()->GetTypeIdByDecl("string"));
            }
        }

        context->Prepare(init);
        context->SetObject(abo);
        context->SetArgAddress(0, args);
        context->SetArgAddress(1, indicators.get());
        context->Execute();
        context->Unprepare();
    }
}

void CharacterInstance::CreateImageSet()
{
    imageSet = CharacterImageSet::CreateImageSet(characterSource);
}

asIScriptObject* CharacterInstance::LoadAbilityObject(const boost::filesystem::path& filepath)
{
    using namespace boost::filesystem;
    auto log = spdlog::get("main");
    auto abroot = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_ABILITY_DIR;

    const auto modulename = ConvertUnicodeToUTF8(filepath.wstring().c_str());
    auto mod = scriptInterface->GetExistModule(modulename);
    if (!mod) {
        scriptInterface->StartBuildModule(modulename, [=](string inc, string from, CScriptBuilder *b) {
            if (!exists(abroot / inc)) return false;
            b->AddSectionFromFile((abroot / inc).string().c_str());
            return true;
        });
        scriptInterface->LoadFile(filepath.string());
        if (!scriptInterface->FinishBuildModule()) {
            scriptInterface->GetLastModule()->Discard();
            return nullptr;
        }
        mod = scriptInterface->GetLastModule();
    }

    //エントリポイント検索
    const int cnt = mod->GetObjectTypeCount();
    asITypeInfo *type = nullptr;
    for (auto i = 0; i < cnt; i++) {
        const auto cti = mod->GetObjectTypeByIndex(i);
        if (!(scriptInterface->CheckMetaData(cti, "EntryPoint") || cti->GetUserData(SU_UDTYPE_ENTRYPOINT))) continue;
        type = cti;
        type->SetUserData(reinterpret_cast<void*>(0xFFFFFFFF), SU_UDTYPE_ENTRYPOINT);
        type->AddRef();
        break;
    }
    if (!type) {
        log->critical(u8"アビリティーにEntryPointがありません");
        return nullptr;
    }

    const auto obj = scriptInterface->InstantiateObject(type);
    type->Release();
    return obj;
}

void CharacterInstance::CallEventFunction(asIScriptObject *obj, asIScriptFunction *func) const
{
    context->Prepare(func);
    context->SetObject(obj);
    context->SetArgAddress(0, targetResult.get());
    context->Execute();
    context->Unprepare();
}

void CharacterInstance::CallEventFunction(asIScriptObject *obj, asIScriptFunction *func, const JudgeInformation &info) const
{
    auto infoClone = info;
    context->Prepare(func);
    context->SetObject(obj);
    context->SetArgAddress(0, targetResult.get());
    context->SetArgObject(1, static_cast<void*>(&infoClone));
    context->Execute();
    context->Unprepare();
}

void CharacterInstance::CallJudgeCallback(const AbilityJudgeType judge, const JudgeInformation &info, const string& extra) const
{
    if (!judgeCallback) return;
    if (!judgeCallback->IsExists()) {
        judgeCallback->Release();
        judgeCallback = nullptr;
        return;
    }

    auto message = extra;
    auto infoClone = info;

    judgeCallback->Prepare();
    judgeCallback->SetArgDWord(0, SU_TO_INT32(judge));
    judgeCallback->SetArgObject(1, &infoClone);
    judgeCallback->SetArgObject(2, &message);
    judgeCallback->Execute();
    judgeCallback->Unprepare();
}

void CharacterInstance::OnStart()
{
    for (auto i = 0u; i < abilities.size(); ++i) {
        const auto func = abilityEvents[i].OnStart;
        const auto obj = abilities[i];
        CallEventFunction(obj, func);
    }
}

void CharacterInstance::OnFinish()
{
    for (auto i = 0u; i < abilities.size(); ++i) {
        const auto func = abilityEvents[i].OnFinish;
        const auto obj = abilities[i];
        CallEventFunction(obj, func);
    }
}

void CharacterInstance::OnJusticeCritical(const JudgeInformation &info, const string& extra)
{
    for (auto i = 0u; i < abilities.size(); ++i) {
        const auto func = abilityEvents[i].OnJusticeCritical;
        const auto obj = abilities[i];
        CallEventFunction(obj, func, info);
    }
    CallJudgeCallback(AbilityJudgeType::JusticeCritical, info, extra);
}

void CharacterInstance::OnJustice(const JudgeInformation &info, const string& extra)
{
    for (auto i = 0u; i < abilities.size(); ++i) {
        const auto func = abilityEvents[i].OnJustice;
        const auto obj = abilities[i];
        CallEventFunction(obj, func, info);
    }
    CallJudgeCallback(AbilityJudgeType::Justice, info, extra);
}

void CharacterInstance::OnAttack(const JudgeInformation &info, const string& extra)
{
    for (auto i = 0u; i < abilities.size(); ++i) {
        const auto func = abilityEvents[i].OnAttack;
        const auto obj = abilities[i];
        CallEventFunction(obj, func, info);
    }
    CallJudgeCallback(AbilityJudgeType::Attack, info, extra);
}

void CharacterInstance::OnMiss(const JudgeInformation &info, const string& extra)
{
    for (auto i = 0u; i < abilities.size(); ++i) {
        const auto func = abilityEvents[i].OnMiss;
        const auto obj = abilities[i];
        CallEventFunction(obj, func, info);
    }
    CallJudgeCallback(AbilityJudgeType::Miss, info, extra);
}

void CharacterInstance::SetCallback(asIScriptFunction *func, ScriptScene *sceneObj)
{
    if (!func || func->GetFuncType() != asFUNC_DELEGATE) return;
    if (judgeCallback) judgeCallback->Release();

    func->AddRef();
    judgeCallback = new CallbackObject(func);
    judgeCallback->SetUserData(sceneObj, SU_UDTYPE_SCENE);

    judgeCallback->AddRef();
    sceneObj->RegisterDisposalCallback(judgeCallback);

    func->Release();
}

CharacterParameter* CharacterInstance::GetCharacterParameter() const
{
    return characterSource.get();
}

SkillParameter* CharacterInstance::GetSkillParameter() const
{
    return skillSource.get();
}

SkillIndicators* CharacterInstance::GetSkillIndicators() const
{
    return indicators.get();
}

CharacterImageSet* CharacterInstance::GetCharacterImages() const
{
    imageSet->AddRef();
    return imageSet;
}

void RegisterCharacterSkillTypes(asIScriptEngine *engine)
{
    RegisterResultTypes(engine);
    RegisterSkillTypes(engine);
    RegisterCharacterTypes(engine);

    engine->RegisterFuncdef("void " SU_IF_JUDGE_CALLBACK "(" SU_IF_JUDGETYPE ", " SU_IF_JUDGE_DATA ", const string &in)");
    engine->RegisterObjectType(SU_IF_CHARACTER_INSTANCE, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_CHARACTER_INSTANCE, asBEHAVE_ADDREF, "void f()", asMETHOD(CharacterInstance, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_CHARACTER_INSTANCE, asBEHAVE_RELEASE, "void f()", asMETHOD(CharacterInstance, Release), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_INSTANCE, SU_IF_CHARACTER_PARAM "@ GetCharacter()", asMETHOD(CharacterInstance, GetCharacterParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_INSTANCE, SU_IF_CHARACTER_IMAGES "@ GetCharacterImages()", asMETHOD(CharacterInstance, GetCharacterImages), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_INSTANCE, SU_IF_SKILL "@ GetSkill()", asMETHOD(CharacterInstance, GetSkillParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_INSTANCE, SU_IF_SKILL_INDICATORS "@ GetSkillIndicators()", asMETHOD(CharacterInstance, GetSkillIndicators), asCALL_THISCALL);
}
