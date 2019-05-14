#pragma once

#define SU_IF_KEY "Key"
#define SU_IF_SEVERITY "Severity"

enum class ScriptLogSeverity {
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Critical,
};

class SettingTree;
class SettingManager;
class AngelScript;
class SoundManager;
class MusicsManager;
class CharacterManager;
class SkillManager;
class ExtensionManager;
class ControlState;
class Scene;
class SkinHolder;
class ScriptScene;
class ScenePlayer;
class SSettingItem;


class ExecutionManager final {
private:
	const std::unique_ptr<SettingManager> settingManager;
	const std::shared_ptr<AngelScript> scriptInterface;
	const std::shared_ptr<SoundManager> sound;
	const std::shared_ptr<MusicsManager> musics;
	const std::shared_ptr<CharacterManager> characters;
	const std::shared_ptr<SkillManager> skills;
	const std::unique_ptr<ExtensionManager> extensions;
	const std::shared_ptr<std::mt19937> random;
	const std::shared_ptr<ControlState> sharedControlState;

	std::vector<std::unique_ptr<Scene>> scenes;
	std::vector<std::unique_ptr<Scene>> scenesPending;
	std::unique_ptr<SkinHolder> skin;
	std::unordered_map<std::string, std::any> optionalData;
	HIMC hImc;
	HANDLE hCommunicationPipe;
	DWORD immConversion, immSentence;

	std::vector<std::wstring> skinNames;

public:
	explicit ExecutionManager(SettingTree* setting);
	~ExecutionManager();

	void Initialize();
	void Tick(double delta);
	void Draw();
	void Fire(const std::string& message);

	void EnumerateSkins();

	SettingManager* GetSettingManagerUnsafe() const { return settingManager.get(); }
	std::shared_ptr<MusicsManager> GetMusicsManagerSafe() const { return musics; }
	MusicsManager* GetMusicsManagerUnsafe() const { return musics.get(); }
	std::shared_ptr<ControlState> GetControlStateSafe() const { return sharedControlState; }
	ControlState* GetControlStateUnsafe() const { return sharedControlState.get(); }
	std::shared_ptr<AngelScript> GetScriptInterfaceSafe() const { return scriptInterface; }
	AngelScript* GetScriptInterfaceUnsafe() const { return scriptInterface.get(); }
	SoundManager* GetSoundManagerUnsafe() const { return sound.get(); }
	std::shared_ptr<CharacterManager> GetCharacterManagerSafe() const { return characters; }
	CharacterManager* GetCharacterManagerUnsafe() const { return characters.get(); }
	std::shared_ptr<SkillManager> GetSkillManagerSafe() const { return skills; }
	SkillManager* GetSkillManagerUnsafe() const { return skills.get(); }

	bool ExecuteSkin();
	bool ExecuteSkin(const std::string& file);
	bool ExecuteScene(asIScriptObject* sceneObject);

	void WriteLog(const std::string& message) { WriteLog(ScriptLogSeverity::Info, message); }
	void WriteLog(ScriptLogSeverity severity, const std::string& message);
	void ExitApplication() const { PostMessage(GetMainWindowHandle(), WM_CLOSE, 0, 0); }

	ScenePlayer* CreatePlayer();
	SSettingItem* GetSettingItem(const std::string& group, const std::string& key) const;

	bool CustomWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult) const;

	template<typename T>
	void SetData(const std::string& name, const T& data);
	template<typename T>
	T GetData(const std::string& name);
	template<typename T>
	T GetData(const std::string& name, const T& defaultValue);
	bool ExistsData(const std::string& name) { return optionalData.find(name) != optionalData.end(); }

private:
	void RegisterGlobalManagementFunction();
};

template<typename T>
void ExecutionManager::SetData(const std::string& name, const T& data)
{
	optionalData[name] = data;
}

template<typename T>
T ExecutionManager::GetData(const std::string& name)
{
	const auto it = optionalData.find(name);
	return it == optionalData.end() ? T() : std::any_cast<T>(it->second);
}

template<typename T>
T ExecutionManager::GetData(const std::string& name, const T& defaultValue)
{
	const auto it = optionalData.find(name);
	return it == optionalData.end() ? defaultValue : std::any_cast<T>(it->second);
}
