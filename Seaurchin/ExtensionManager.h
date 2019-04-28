/*!
 * @file ExtensionManager.h
 * @brief エクステンション管理を行うクラス ExtensionManager の宣言
 * @author kb10uy
 * @date 2019/04/27
 * @details エクステンションの列挙、初期化、登録を行うインターフェースを提供します。
 */

#pragma once

/*!
 * @brief エクステンションの管理を行います
 * @details 列挙(LoadExtensions)、初期化(Initialize)、登録(RegisterInterfaces)の順に、それぞれ一度だけ実行してください。
 * 読み込んだエクステンションはデストラクタで解放されます。
 */
class ExtensionManager final {
private:
	std::vector<HINSTANCE> dllInstances;	//!< ロードしたdllの実体の配列

	bool LoadDll(const std::filesystem::path& path);

public:
	ExtensionManager();
	~ExtensionManager();

	void LoadExtensions();
	void Initialize(asIScriptEngine* engine);
	void RegisterInterfaces();
};
