/*!
 * @file ExtensionManager.cpp
 * @brief エクステンション管理を行うクラス ExtensionManager の定義
 * @author kb10uy
 * @date 2019/04/29
 * @details エクステンションの列挙、初期化、登録を行うインターフェースを提供します。
 */

#include "ExtensionManager.h"
#include "SeaurchinExtension.h"
#include "SettingManager.h"

ExtensionManager::ExtensionManager()
= default;

ExtensionManager::~ExtensionManager()
{
	for (const auto& e : dllInstances) if (e) FreeLibrary(e);
}

bool ExtensionManager::LoadDll(const std::filesystem::path& path)
{
	const auto h = LoadLibraryW(path.c_str());
	if (!h) return false;

	dllInstances.push_back(h);

	return true;
}

void ExtensionManager::LoadExtensions()
{
	using namespace std::filesystem;
	const auto root = SettingManager::GetRootDirectory() / SU_EXTENSION_DIR;
	for (const auto& fdata : directory_iterator(root)) {
		if (is_directory(fdata)) continue;
		if (fdata.path().extension() != ".dll") continue;
		LoadDll(fdata.path());
	}

	spdlog::get("main")->info(u8"エクステンション総数: {0}", dllInstances.size());
}

void ExtensionManager::Initialize(asIScriptEngine * engine)
{
	for (const auto& h : dllInstances) {
		const auto func = SE_InitializeExtension(GetProcAddress(h, "InitializeExtension"));
		if (!func) continue;
		func(engine);
	}
}

void ExtensionManager::RegisterInterfaces()
{
	for (const auto& h : dllInstances) {
		const auto func = SE_RegisterInterfaces(GetProcAddress(h, "RegisterInterfaces"));
		if (!func) continue;
		func();
	}
}
