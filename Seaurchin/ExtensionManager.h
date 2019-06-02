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

public:
	ExtensionManager();

	//! @note ロードされたdllの破棄も行います。
	~ExtensionManager();

private:
	//! @brief dllをロードします。
	//! @param[in] path ロードするdllのパス。絶対パスを想定しています。
	//! @return bool ロードに失敗した場合falseを返します。
	bool LoadDll(const std::filesystem::path& path);

public:
	//! @brief エクステンションを列挙しロードします。
	//! @note 読み込む対象は Setting::GetRootDirectory() / SU_EXTENSION_DIR 内の *.dll です。
	void LoadExtensions();

	//! @brief エクステンションの初期化を行います。
	//! @param[in] engine スクリプトエンジン。
	void Initialize(asIScriptEngine* engine);

	//! @brief エクステンションの登録を行います。
	//! @details このメソッドの呼び出し以降、エクステンションで提供される機能が利用できるようになります。
	void RegisterInterfaces();
};
