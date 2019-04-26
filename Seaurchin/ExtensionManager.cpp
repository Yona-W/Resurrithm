#include "ExtensionManager.h"
#include "SeaurchinExtension.h"
#include "Config.h"
#include "Setting.h"

using namespace std;

ExtensionManager::ExtensionManager()
= default;

ExtensionManager::~ExtensionManager()
{
    for (const auto &e : dllInstances) if (e) FreeLibrary(e);
}

void ExtensionManager::LoadExtensions()
{
    using namespace std;
    using namespace filesystem;
    const auto root = Setting::GetRootDirectory() / SU_DATA_DIR / SU_EXTENSION_DIR;
    for (const auto& fdata : directory_iterator(root)) {
        if (is_directory(fdata)) continue;
        const auto filename = fdata.path().wstring();
        const std::wstring ext(L"dll");
        size_t len1 = filename.size();
        size_t len2 = ext.size();
        if (!(len1 >= len2 && filename.compare(len1 - len2, len2, ext) == 0)) continue;
        LoadDll(filename);
    }

    spdlog::get("main")->info(u8"エクステンション総数: {0}", dllInstances.size());
}

void ExtensionManager::LoadDll(wstring path)
{
    const auto h = LoadLibraryW(path.c_str());
    if (!h) return;

    dllInstances.push_back(h);
}

void ExtensionManager::Initialize(asIScriptEngine *engine)
{
    for (const auto &h : dllInstances) {
        const auto func = SE_InitializeExtension(GetProcAddress(h, "InitializeExtension"));
        if (!func) continue;
        func(engine);
    }
}

void ExtensionManager::RegisterInterfaces()
{
    for (const auto &h : dllInstances) {
        const auto func = SE_RegisterInterfaces(GetProcAddress(h, "RegisterInterfaces"));
        if (!func) continue;
        func();
    }
}
