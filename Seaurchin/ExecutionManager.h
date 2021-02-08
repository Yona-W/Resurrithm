#pragma once

#include "Setting.h"
#include "AngelScriptManager.h"
#include "Scene.h"
#include "ScriptScene.h"
#include "SkinHolder.h"
#include "MusicsManager.h"
#include "ExtensionManager.h"
#include "SoundManager.h"
#include "ScenePlayer.h"
#include "Controller.h"
#include "Character.h"
#include "Skill.h"

#include <random>
#include <map>
#include <spdlog/spdlog.h>

#include <SDL2/SDL_events.h>
#include <boost/filesystem.hpp>

class ExecutionManager final {
    friend class ScenePlayer;

private:
    const std::shared_ptr<Setting> sharedSetting;
    const std::unique_ptr<setting2::SettingItemManager> settingManager;
    const std::shared_ptr<AngelScript> scriptInterface;
    const std::shared_ptr<SoundManager> sound;
    const std::shared_ptr<MusicsManager> musics;
    const std::shared_ptr<CharacterManager> characters;
    const std::shared_ptr<SkillManager> skills;
    //const std::unique_ptr<ExtensionManager> extensions;
    const std::shared_ptr<std::mt19937> random;
    const std::shared_ptr<ControlState> sharedControlState;

    std::vector<std::shared_ptr<Scene>> scenes;
    std::vector<std::shared_ptr<Scene>> scenesPending;
    std::vector<std::string> skinNames;
    std::unique_ptr<SkinHolder> skin;
    std::unordered_map<std::string, boost::any> optionalData;
    DrawableResult lastResult;
    SSoundMixer *mixerBgm, *mixerSe;

    #ifdef _WIN32

    HIMC hImc;
    HANDLE hCommunicationPipe;
    DWORD immConversion, immSentence;

    #endif

public:
    explicit ExecutionManager(const std::shared_ptr<Setting>& setting);

    void EnumerateSkins();
    void Tick(double delta);
    void Draw();
    void Initialize();
    void Shutdown();
    void AddScene(const std::shared_ptr<Scene>& scene);
    std::shared_ptr<ScriptScene> CreateSceneFromScriptType(asITypeInfo *type) const;
    std::shared_ptr<ScriptScene> CreateSceneFromScriptObject(asIScriptObject *obj) const;
    int GetSceneCount() const { return scenes.size(); }

    std::shared_ptr<MusicsManager> GetMusicsManager() const { return musics; }
    std::shared_ptr<ControlState> GetControlStateSafe() const { return sharedControlState; }
    std::shared_ptr<Setting> GetSettingInstanceSafe() const { return sharedSetting; }
    std::shared_ptr<AngelScript> GetScriptInterfaceSafe() const { return scriptInterface; }
    ControlState* GetControlStateUnsafe() const { return sharedControlState.get(); }
    AngelScript* GetScriptInterfaceUnsafe() const { return scriptInterface.get(); }
    SoundManager* GetSoundManagerUnsafe() const { return sound.get(); }
    std::shared_ptr<CharacterManager> GetCharacterManagerSafe() const { return characters; }
    std::shared_ptr<SkillManager> GetSkillManagerSafe() const { return skills; }
    CharacterManager* GetCharacterManagerUnsafe() const { return characters.get(); }
    SkillManager* GetSkillManagerUnsafe() const { return skills.get(); }

    #ifdef _WIN32
    std::tuple<bool, LRESULT> CustomWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) const;
    #endif

    void ExecuteSkin();
    bool ExecuteSkin(const std::string &file);
    bool ExecuteScene(asIScriptObject *sceneObject);
    void ExecuteSystemMenu();
    void Fire(const std::string &message);
    void ExitApplication();
    void WriteLog(const std::string &message) const;
    ScenePlayer *CreatePlayer();
    SSoundMixer *GetDefaultMixer(const std::string &name) const;
    SSettingItem *GetSettingItem(const std::string &group, const std::string &key) const;
    void GetStoredResult(DrawableResult *result) const;

    template<typename T>
    void SetData(const std::string &name, const T& data);
    void SetBoolData(const std::string &name, const bool &data) { SetData(name, data); }
    void SetIntData(const std::string &name, const int &data) { SetData(name, data); }
    void SetDoubleData(const std::string &name, const double &data) { SetData(name, data); }
    void SetStringData(const std::string &name, const std::string &data) { SetData(name, data); }
    template<typename T>
    T GetData(const std::string &name);
    template<typename T>
    T GetData(const std::string &name, const T& defaultValue);
    bool ExistsData(const std::string &name) { return optionalData.find(name) != optionalData.end(); }

    bool shouldExit;

private:
    bool CheckSkinStructure(const boost::filesystem::path& name) const;
    void RegisterGlobalManagementFunction();
};

template<typename T>
void ExecutionManager::SetData(const std::string &name, const T & data)
{
    optionalData[name] = data;
}

template<typename T>
T ExecutionManager::GetData(const std::string &name)
{
    auto it = optionalData.find(name);
    return it == optionalData.end() ? T() : boost::any_cast<T>(it->second);
}

template<typename T>
T ExecutionManager::GetData(const std::string &name, const T& defaultValue)
{
    auto it = optionalData.find(name);
    return it == optionalData.end() ? defaultValue : boost::any_cast<T>(it->second);
}
