#include "Setting.h"
#include "SettingManager.h"

using namespace std;
using namespace filesystem;

namespace {
	/*!
	 * @brief アプリケーションのワーキングディレクトリを取得します。
	 * @param[in] hModule アプリケーションのモジュールハンドル。
	 * @return ワーキングディレクトリの絶対パス。
	 */
	path GetRootDirectory(HMODULE hModule)
	{
		wchar_t directory[MAX_PATH];
		GetModuleFileNameW(hModule, directory, MAX_PATH);
		PathRemoveFileSpecW(directory);
		return path(directory);
	}
}

/*!
 * @param[in] hModule アプリケーションのモジュールハンドル。
 * @param[in] filename 設定ファイル名。
 */
SettingTree::SettingTree(HMODULE hModule, const path& filename)
	: rootDir(GetRootDirectory(hModule))
	, fileName(filename)
{}

/*!
 * @brief 設定ファイルから設定値を読み込みます。
 * @note 設定ファイルが存在しない場合、現在の設定をファイルへ書き出します。
 * @return 読み込みに失敗するとfalseが返ります。
 */
bool SettingTree::Load()
{
	auto log = spdlog::get("main");

	if (fileName.empty()) {
		log->error(u8"設定ファイルパスが無効値です");
		return false;
	}

	const auto path = rootDir / fileName;
	if (!exists(path)) {
		log->info(u8"設定ファイルを作成します");
		Save();
	}

	std::ifstream ifs(path.wstring(), ios::in);
	auto pr = toml::parse(ifs);
	ifs.close();

	if (!pr.valid()) {
		log->error(u8"設定ファイルの記述が不正です: {0}", pr.errorReason);
		return false;
	}
	else {
		setting = pr.value;
		log->info(u8"設定ファイルを読み込みました");
	}

	return true;
}

/*!
 * @brief 設定ファイルへ設定値を書きだします。
 */
void SettingTree::Save() const
{
	auto log = spdlog::get("main");

	if (fileName.empty()) {
		log->error(u8"設定ファイルパスが無効値です");
		return;
	}

	if (!setting.valid()) {
		log->error(u8"設定値が不正です");
		return;
	}

	const auto path = rootDir / fileName;

	log->info(u8"設定ファイルに書き込みます");
	std::ofstream ofs(path.wstring(), ios::out);
	setting.write(&ofs);
	ofs.close();
	log->info(u8"設定ファイルを保存しました");
}

/*!
 * @brief グループ名を指定して設定値集合を取得します。
 * @param[in] group 取得したい設定値の属するグループ名。
 * @return 設定値集合からなるtoml::Valueの生ポインタ。該当する設定値が存在しない場合nullが返ります。
 */
const toml::Value* SettingTree::GetGroup(const std::string& group) const
{
	const auto g = setting.find(group);
	return (g && g->is<toml::Table>()) ? g : nullptr;
}

/*!
 * @brief グループ名、キー名を指定して設定値を取得します。
 * @param[in] group 取得したい設定値の属するグループ名。
 * @param[in] key 取得したい設定値の属するキー名。
 * @return 設定値を格納したtoml::Valueの生ポインタ。該当する設定値が存在しない場合nullが返ります。
 */
const toml::Value* SettingTree::GetValues(const string& group, const std::string& key) const
{
	const auto g = setting.find(group);
	return (g && g->is<toml::Table>()) ? g->find(key) : nullptr;
}


SettingItem::SettingItem(const shared_ptr<SettingTree> setting, const string& igroup, const string& ikey)
	: settingInstance(setting)
	, description(u8"説明はありません")
	, group(igroup)
	, key(ikey)
{}

void SettingItem::Build(const toml::Value& table)
{
	const auto d = table.find("Description");
	if (d && d->is<string>()) {
		description = d->as<string>();
	}
}


BooleanSettingItem::BooleanSettingItem(const std::shared_ptr<SettingTree> setting, const std::string & group, const std::string & key)
	: SettingItem(setting, group, key)
	, value(false)
	, defaultValue(false)
	, falsy("false")
	, truthy("true")
{}

void BooleanSettingItem::Build(const toml::Value& table)
{
	SettingItem::Build(table);

	auto log = spdlog::get("main");

	const auto r = table.find("Values");
	if (r && r->is<vector<string>>()) {
		auto v = r->as<vector<string>>();
		if (v.size() != 2) {
			log->warn(u8"真偽値型設定項目 {0}.{1} に対する値指定が不正です。", group, key);
		}
		else {
			truthy = v[0];
			falsy = v[1];
		}
	}
	const auto d = table.find("Default");
	if (d && d->is<bool>()) {
		defaultValue = d->as<bool>();
	}
}


StringSettingItem::StringSettingItem(const std::shared_ptr<SettingTree> setting, const std::string & group, const std::string & key)
	: SettingItem(setting, group, key)
	, value("")
	, defaultValue("")
{}

void StringSettingItem::Build(const toml::Value& table)
{
	SettingItem::Build(table);

	const auto d = table.find("Default");
	if (d && d->is<string>()) {
		defaultValue = d->as<string>();
	}
}
