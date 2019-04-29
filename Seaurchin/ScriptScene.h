#pragma once

#include "Scene.h"
#include "ScriptSprite.h"
#include "ScriptFunction.h"

#define SU_IF_SCENE "Scene"
#define SU_IF_COSCENE "CoroutineScene"
#define SU_IF_COROUTINE "Coroutine"

class Coroutine {
private:
	asIScriptContext* context;
	asIScriptObject* object;
	asIScriptFunction* function;
	asITypeInfo* type;

public:
	std::string Name;
	CoroutineWait Wait;

public:
	Coroutine(const std::string& name, const asIScriptFunction* cofunc, asIScriptEngine* engine);
	~Coroutine();

	asIScriptContext* GetContext() const { return context; }

	void* SetUserData(void* data, asPWORD type) { return context->SetUserData(data, type); }

	int Execute() { return context->Execute(); }

private:
	int Prepare()
	{
		const auto r1 = context->Prepare(function);
		if (r1 != asSUCCESS) return r1;
		return context->SetObject(object);
	}
	int Unprepare() { return context->Unprepare(); }

public:
	bool Tick(double delta) { return Wait.Tick(delta); }
};

class MethodObject;
class CallbackObject;

class ScriptScene : public Scene {
	typedef Scene Base;
protected:
	asIScriptObject* const sceneObject;

	MethodObject* initMethod;
	MethodObject* mainMethod;
	MethodObject* eventMethod;

	std::multiset<SSprite*, SSprite::Comparator> sprites;
	std::vector<SSprite*> spritesPending;
	std::list<Coroutine*> coroutines;
	std::list<Coroutine*> coroutinesPending;
	std::vector<CallbackObject*> callbacks;
	bool finished;

	void TickCoroutine(double delta);
	void TickSprite(double delta);
	void DrawSprite();

public:
	ScriptScene(asIScriptObject* scene);
	virtual ~ScriptScene();

	const char* GetMainMethodDecl() const override { return "void Tick(double)"; }

	void Initialize() override;

	void AddSprite(SSprite* sprite);
	void AddCoroutine(Coroutine* co);
	void KillCoroutine(const std::string& name);
	void Tick(double delta) override;
	void OnEvent(const std::string& message) override;
	void Draw() override;
	bool IsDead() override;
	void Disappear() override;
	void Dispose() override;

	void RegisterDisposalCallback(CallbackObject* callback);
};

class ScriptCoroutineScene : public ScriptScene {
	typedef ScriptScene Base;
protected:
	CoroutineWait wait;

public:
	ScriptCoroutineScene(asIScriptObject* scene);
	virtual ~ScriptCoroutineScene();

	const char* GetMainMethodDecl() const override { return "void Run()"; }

	void Initialize() override;

	void Tick(double delta) override;
};

class ExecutionManager;
void RegisterScriptScene(ExecutionManager* exm);

int ScriptSceneGetIndex();
void ScriptSceneSetIndex(int index);
bool ScriptSceneIsKeyHeld(int keynum);
bool ScriptSceneIsKeyTriggered(int keynum);
void ScriptSceneAddSprite(SSprite* sprite);
void ScriptSceneRunCoroutine(asIScriptFunction* cofunc, const std::string& name);
void ScriptSceneKillCoroutine(const std::string& name);
void ScriptSceneDisappear();
