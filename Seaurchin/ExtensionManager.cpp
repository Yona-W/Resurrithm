#include "ExtensionManager.h"
#include "SeaurchinExtension.h"
#include "Config.h"
#include "Setting.h"

/*
using namespace std;

ExtensionManager::ExtensionManager()
= default;

ExtensionManager::~ExtensionManager()
{
    for (const auto &e : dllInstances) if (e) FreeLibrary(e);
}

void ExtensionManager::LoadExtensions()
{
    using namespace boost;
    using namespace filesystem;
    const auto root = Setting::GetRootDirectory() / SU_DATA_DIR / SU_EXTENSION_DIR;
    for (const auto& fdata : make_iterator_range(directory_iterator(root), {})) {
        if (is_directory(fdata)) continue;
        const auto filename = fdata.path().wstring();
        if (!ends_with(filename, L".dll")) continue;
        LoadDll(filename);
    }

    spdlog::info(u8"エクステンション総数: {0}", dllInstances.size());
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
*/