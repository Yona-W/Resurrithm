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

/*!
 * @details ロードされたdllの破棄も行います。
 */
ExtensionManager::~ExtensionManager()
{
	for (const auto& e : dllInstances) if (e) FreeLibrary(e);
}

/*!
 * @brief エクステンションを列挙しロードします。
 * @details 読み込む対象は Setting::GetRootDirectory() / SU_EXTENSION_DIR 内の *.dll です。
 */
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

/*!
 * @brief dllをロードします。
 * @param[in] path ロードするdllのパス。絶対パスを想定しています。
 * @return bool ロードに失敗した場合falseを返します。
 */
bool ExtensionManager::LoadDll(const std::filesystem::path& path)
{
	const auto h = LoadLibraryW(path.c_str());
	if (!h) return false;

	dllInstances.push_back(h);

	return true;
}

/*!
 * @brief エクステンションの初期化を行います。
 * @param[in] engine スクリプトエンジン。
 */
void ExtensionManager::Initialize(asIScriptEngine * engine)
{
	for (const auto& h : dllInstances) {
		const auto func = SE_InitializeExtension(GetProcAddress(h, "InitializeExtension"));
		if (!func) continue;
		func(engine);
	}
}

/*!
 * @brief エクステンションの登録を行います。
 * @details このメソッドの呼び出し以降、エクステンションで提供される機能が利用できるようになります。
 */
void ExtensionManager::RegisterInterfaces()
{
	for (const auto& h : dllInstances) {
		const auto func = SE_RegisterInterfaces(GetProcAddress(h, "RegisterInterfaces"));
		if (!func) continue;
		func();
	}
}
