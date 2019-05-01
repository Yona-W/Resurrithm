/*!
 * @file SettingManager.h
 * @brief 設定項目の管理を行うクラス SettingManager の宣言
 * @author kb10uy
 * @date 2019/04/29
 * @details 設定項目定義ファイルの読み込み、パース、設定項目取得を行うインターフェースを提供します。
 */

#pragma once

class SettingTree;
class SettingItem;

/*!
 * @brief 設定項目の管理を行います
 * @details 設定値集合(SettingTree)を内部に持ち、
 * それらの設定値を操作する設定項目を設定項目定義ファイルからロードして生成します。
 * @todo ルートディレクトリ管理は別のクラスにやらせる
 */
class SettingManager final {
	typedef std::unordered_map<std::string, std::shared_ptr<SettingItem>> SettingItemMap;
private:
	static std::filesystem::path rootDirectory;	//!< アプリケーションのワーキングディレクトリへの絶対パス

	const std::unique_ptr<SettingTree> setting;	//!< 設定値集合
	SettingItemMap items;						//!< 設定項目名をキーとした設定項目の集合

public:
	explicit SettingManager(SettingTree* setting);

	//! @brief アプリケーションのワーキングディレクトリへの絶対パスを取得します。
	static std::filesystem::path GetRootDirectory() { return rootDirectory; }

	void LoadItemsFromToml(const std::filesystem::path& file);
	void RetrieveAllValues();
	void SaveAllValues();

	//! @brief 設定値集合のスマートポインタを取得します。
	SettingTree* GetSettingInstanceUnsafe() { return setting.get(); }

	std::shared_ptr<SettingItem> GetSettingItem(const std::string& group, const std::string& key);
};
