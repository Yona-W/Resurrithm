#include "ScriptScene.h"
#include "ScriptSprite.h"
#include "ScriptCoroutine.h"
#include "ExecutionManager.h"
#include "AngelScriptManager.h"
#include "Controller.h"

using namespace std;
using namespace std::filesystem;

ScriptScene::ScriptScene(asIScriptObject* scene)
	: sceneObject(scene)
	, initMethod(nullptr)
	, mainMethod(nullptr)
	, eventMethod(nullptr)
	, finished(false)
{}

ScriptScene::~ScriptScene()
{
	KillCoroutine("");

	if (mainMethod) mainMethod->Unprepare();

	delete initMethod;
	delete mainMethod;
	delete eventMethod;

	sceneObject->Release();

	for (auto& i : sprites) i->Release();
	sprites.clear();
	for (auto& i : spritesPending) i->Release();
	spritesPending.clear();
}

void ScriptScene::Initialize()
{
	const auto sceneType = sceneObject->GetObjectType();
	auto engine = sceneObject->GetEngine();

	sceneObject->AddRef();
	initMethod = MethodObject::Create(sceneObject, "void Initialize()");
	if (initMethod) initMethod->SetUserData(this, SU_UDTYPE_SCENE);

	sceneObject->AddRef();
	mainMethod = MethodObject::Create(sceneObject, this->GetMainMethodDecl());
	if (mainMethod) mainMethod->SetUserData(this, SU_UDTYPE_SCENE);

	sceneObject->AddRef();
	eventMethod = MethodObject::Create(sceneObject, "void OnEvent(const string &in)");
	if (eventMethod) eventMethod->SetUserData(this, SU_UDTYPE_SCENE);

	if (!initMethod) return;

	initMethod->Prepare();
	if (initMethod->Execute() != asEXECUTION_FINISHED) Disappear();
	initMethod->Unprepare();
}

void ScriptScene::AddSprite(SSprite * sprite)
{
	spritesPending.push_back(sprite);
}

void ScriptScene::AddCoroutine(Coroutine * co)
{
	coroutinesPending.push_back(co);
}

void ScriptScene::KillCoroutine(const string & name)
{
	if (name.empty()) {
		for (auto& i : coroutinesPending) delete i;
		coroutinesPending.clear();
		for (auto& i : coroutines) delete i;
		coroutines.clear();
	}
	else {
		auto i = coroutinesPending.begin();
		while (i != coroutinesPending.end()) {
			const auto c = *i;
			if (c->Name == name) {
				delete c;
				i = coroutinesPending.erase(i);
			}
			else {
				++i;
			}
		}
		i = coroutines.begin();
		while (i != coroutines.end()) {
			const auto c = *i;
			if (c->Name == name) {
				delete c;
				i = coroutines.erase(i);
			}
			else {
				++i;
			}
		}
	}
}

void ScriptScene::Tick(const double delta)
{
	TickSprite(delta);
	TickCoroutine(delta);

	if (!mainMethod) return;

	mainMethod->Prepare();
	if (mainMethod->SetArgument([&delta](auto p) { p->SetArgDouble(0, delta); return true; })) {
		if (mainMethod->Execute() != asEXECUTION_FINISHED) Disappear();
		mainMethod->Unprepare();
	}
}

void ScriptScene::OnEvent(const string & message)
{
	if (!eventMethod) return;

	auto msg = message;
	eventMethod->Prepare();
	if (eventMethod->SetArgument([&msg](auto p) { p->SetArgAddress(0, static_cast<void*>(&msg)); return true; })) {
		if (eventMethod->Execute() != asEXECUTION_FINISHED) Disappear();
		eventMethod->Unprepare();
	}
}

void ScriptScene::Draw()
{
	DrawSprite();
}

bool ScriptScene::IsDead()
{
	return finished;
}

void ScriptScene::Disappear()
{
	finished = true;
}

void ScriptScene::Dispose()
{
	for (auto it = callbacks.begin(); it != callbacks.end(); ++it) {
		(*it)->Dispose();
		(*it)->Release();
	}
	callbacks.clear();
}

void ScriptScene::RegisterDisposalCallback(CallbackObject * callback)
{
	callbacks.push_back(callback);
}

void ScriptScene::TickCoroutine(const double delta)
{
	if (!coroutinesPending.empty()) {
		for (auto& coroutine : coroutinesPending) {
			coroutine->SetUserData(this, SU_UDTYPE_SCENE);
			coroutines.push_back(coroutine);
		}
		coroutinesPending.clear();
	}

	auto i = coroutines.begin();
	while (i != coroutines.end()) {
		const auto result = (*i)->Tick(delta);

		if (result == asEXECUTION_SUSPENDED) {
			++i;
		}
		else {
			delete *i;
			i = coroutines.erase(i);
		}
	}
}

void ScriptScene::TickSprite(const double delta)
{
	if (!spritesPending.empty()) {
		for (auto& sprite : spritesPending) sprites.emplace(sprite);
		spritesPending.clear();
	}

	auto i = sprites.begin();
	while (i != sprites.end()) {
		(*i)->Tick(delta);
		if ((*i)->IsDead) {
			(*i)->Release();
			i = sprites.erase(i);
		}
		else {
			++i;
		}
	}
}

void ScriptScene::DrawSprite()
{
	for (auto& i : sprites) i->Draw();
}


ScriptCoroutineScene::ScriptCoroutineScene(asIScriptObject * scene)
	: Base(scene)
	, wait(new CoroutineWait(0ull))
{}

ScriptCoroutineScene::~ScriptCoroutineScene()
{
	delete wait;
}

void ScriptCoroutineScene::Initialize()
{
	Base::Initialize();

	if (mainMethod) {
		mainMethod->SetUserData(wait, SU_UDTYPE_WAIT);
		mainMethod->Prepare();
	}
}

void ScriptCoroutineScene::Tick(const double delta)
{
	TickSprite(delta);
	TickCoroutine(delta);

	//Run()
	if (wait->Tick(delta)) return;

	if (!mainMethod) {
		Disappear();
		return;
	}

	if (mainMethod->ExecuteAsSuspendable() != asEXECUTION_SUSPENDED) Disappear();
}

void RegisterScriptScene(ExecutionManager * exm)
{
	auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

	engine->RegisterFuncdef("void " SU_IF_COROUTINE "()");

	engine->RegisterInterface(SU_IF_SCENE);
	engine->RegisterInterfaceMethod(SU_IF_SCENE, "void Initialize()");
	engine->RegisterInterfaceMethod(SU_IF_SCENE, "void Tick(double)");

	engine->RegisterInterface(SU_IF_COSCENE);
	engine->RegisterInterfaceMethod(SU_IF_COSCENE, "void Initialize()");
	engine->RegisterInterfaceMethod(SU_IF_COSCENE, "void Run()");
}

// Scene用メソッド

void ScriptSceneSetIndex(const int index)
{
	const auto ctx = asGetActiveContext();
	const auto psc = static_cast<ScriptScene*>(ctx->GetUserData(SU_UDTYPE_SCENE));
	if (!psc) {
		ScriptSceneWarnOutOf("SetIndex", "Scene Class", ctx);
		return;
	}
	psc->SetIndex(index);
}

int ScriptSceneGetIndex()
{
	const auto ctx = asGetActiveContext();
	const auto psc = static_cast<ScriptScene*>(ctx->GetUserData(SU_UDTYPE_SCENE));
	if (!psc) {
		ScriptSceneWarnOutOf("GetIndex", "Scene Class", ctx);
		return 0;
	}
	return psc->GetIndex();
}

bool ScriptSceneIsKeyHeld(const int keynum)
{
	const auto ctx = asGetActiveContext();
	const auto psc = static_cast<ScriptScene*>(ctx->GetUserData(SU_UDTYPE_SCENE));
	if (!psc) {
		ScriptSceneWarnOutOf("IsKeyHeld", "Scene Class", ctx);
		return false;
	}
	return psc->GetManager()->GetControlStateUnsafe()->GetCurrentState(ControllerSource::RawKeyboard, keynum);
}

bool ScriptSceneIsKeyTriggered(const int keynum)
{
	const auto ctx = asGetActiveContext();
	const auto psc = static_cast<ScriptScene*>(ctx->GetUserData(SU_UDTYPE_SCENE));
	if (!psc) {
		ScriptSceneWarnOutOf("IsKeyTriggered", "Scene Class", ctx);
		return false;
	}
	return  psc->GetManager()->GetControlStateUnsafe()->GetTriggerState(ControllerSource::RawKeyboard, keynum);
}

void ScriptSceneAddSprite(SSprite * sprite)
{
	if (!sprite) return;

	const auto ctx = asGetActiveContext();
	auto psc = static_cast<ScriptCoroutineScene*>(ctx->GetUserData(SU_UDTYPE_SCENE));
	if (!psc) {
		ScriptSceneWarnOutOf("AddSprite", "Scene Class", ctx);
		return;
	}
	{
		sprite->AddRef();
		psc->AddSprite(sprite);
	}

	sprite->Release();
}

void ScriptSceneRunCoroutine(asIScriptFunction * cofunc, const string & name)
{
	if (!cofunc) return;

	auto const ctx = asGetActiveContext();
	auto const psc = static_cast<ScriptScene*>(ctx->GetUserData(SU_UDTYPE_SCENE));
	if (!psc) {
		ScriptSceneWarnOutOf("RunCoroutine", "Scene Class", ctx);
		return;
	}

	if (cofunc->GetFuncType() != asFUNC_DELEGATE) {
		// TODO: エラー丁寧にだすべき
		ScriptSceneWarnOutOf("RunCoroutine", "Scene Class", ctx);
	}
	else {
		cofunc->AddRef();
		psc->AddCoroutine(new Coroutine(name, cofunc));
	}

	cofunc->Release();
}

void ScriptSceneKillCoroutine(const std::string & name)
{
	const auto ctx = asGetActiveContext();
	auto psc = static_cast<ScriptScene*>(ctx->GetUserData(SU_UDTYPE_SCENE));
	if (!psc) {
		ScriptSceneWarnOutOf("KillCoroutine", "Scene Class", ctx);
		return;
	}
	psc->KillCoroutine(name);
}

void ScriptSceneDisappear()
{
	const auto ctx = asGetActiveContext();
	auto psc = static_cast<ScriptCoroutineScene*>(ctx->GetUserData(SU_UDTYPE_SCENE));
	if (!psc) {
		ScriptSceneWarnOutOf("Disappear", "Scene Class", ctx);
		return;
	}
	psc->Disappear();
}

void ScriptSceneYieldFrame(uint64_t frames)
{
	auto ctx = asGetActiveContext();
	auto pcw = static_cast<CoroutineWait*>(ctx->GetUserData(SU_UDTYPE_WAIT));
	if (!pcw) {
		ScriptSceneWarnOutOf("YieldFrame", "Coroutine Scene Class or Coroutine", ctx);
		return;
	}
	pcw->WaitFrame(frames);
	ctx->Suspend();
}

void ScriptSceneYieldTime(double time)
{
	auto ctx = asGetActiveContext();
	auto pcw = static_cast<CoroutineWait*>(ctx->GetUserData(SU_UDTYPE_WAIT));
	if (!pcw) {
		ScriptSceneWarnOutOf("YieldTime", "Coroutine Scene Class or Coroutine", ctx);
		return;
	}
	pcw->WaitTime(time);
	ctx->Suspend();
}
