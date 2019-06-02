#pragma once

#include "Scene.h"
#include "ScriptSprite.h"

#define SU_IF_SCENE "Scene"
#define SU_IF_COSCENE "CoroutineScene"

class Coroutine;
class CoroutineWait;
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
	CoroutineWait* const wait;

public:
	ScriptCoroutineScene(asIScriptObject* scene);
	virtual ~ScriptCoroutineScene();

	const char* GetMainMethodDecl() const override { return "void Run()"; }

	void Initialize() override;

	void Tick(double delta) override;
};

void RegisterScriptScene(asIScriptEngine* engine);

int ScriptSceneGetIndex();
void ScriptSceneSetIndex(int index);
bool ScriptSceneIsKeyHeld(int keynum);
bool ScriptSceneIsKeyTriggered(int keynum);
void ScriptSceneAddSprite(SSprite* sprite);
void ScriptSceneRunCoroutine(asIScriptFunction* cofunc, const std::string& name);
void ScriptSceneKillCoroutine(const std::string& name);
void ScriptSceneDisappear();

//! @brief 指定フレーム数関数実行を待機します。
//! @param[in] frames 待機フレーム数
//! @note 実行中のコンテキストにSU_UDTYPE_WAITが紐づいていない場合(これを呼び出した関数がsuspendする前提の関数でない場合)、失敗します。
void ScriptSceneYieldFrame(uint64_t frames);

//! @brief 指定時間関数実行を待機します。
//! @param[in] time 待機時間
//! @note 実行中のコンテキストにSU_UDTYPE_WAITが紐づいていない場合(これを呼び出した関数がsuspendする前提の関数でない場合)、失敗します。
void ScriptSceneYieldTime(double time);
