#include "SettingManager.h"
#include "Setting.h"

using namespace std;
using namespace filesystem;

path SettingManager::rootDirectory;

SettingManager::SettingManager(shared_ptr<SettingTree> setting)
	: setting(setting)
{
	BOOST_ASSERT(setting);

	if (rootDirectory.empty()) rootDirectory = setting->GetRootDir();
}

/*!
 * @brief 指定された設定項目定義ファイルをもとに設定項目一覧を生成します。
 * @param[in] file 設定項目定義ファイルへのパス。絶対パスを想定しています。
 * @details この関数の呼び出しでは設定項目は上書きではなく追加されます。
 * @todo 非同期動作ができた方がベター
 */
void SettingManager::LoadItemsFromToml(const path& file)
{
	using namespace std::filesystem;
	using namespace crc32_constexpr;

	auto log = spdlog::get("main");

	std::ifstream ifs(file.wstring(), ios::in);
	auto pr = toml::parse(ifs);
	ifs.close();
	if (!pr.valid()) {
		log->error(u8"設定定義 {0} は不正なファイルです", ConvertUnicodeToUTF8(file.wstring()));
		log->error(pr.errorReason);
		return;
	}

	const auto items = pr.value.find("SettingItems");
	if (!items || !items->is<toml::Array>()) {
		log->warn(u8"設定定義 {0} に設定項目がありません", ConvertUnicodeToUTF8(file.wstring()));
		return;
	}
	for (const auto& item : items->as<vector<toml::Value>>()) {
		if (item.type() != toml::Value::TABLE_TYPE) continue;
		if (!item.has("Group")) {
			log->error(u8"設定項目にグループ指定が存在しません。");
			continue;
		}
		if (!item.has("Key")) {
			log->error(u8"設定項目にキー指定が存在しません。");
			continue;
		}
		if (!item.has("Type")) {
			log->error(u8"設定項目に種別指定が存在しません。");
			continue;
		}

		auto group = item.get<string>("Group");
		auto key = item.get<string>("Key");
		auto type = item.get<string>("Type");

		shared_ptr<SettingItem> si;
		switch (Crc32Rec(0xffffffff, type.c_str())) {
		case "Integer"_crc32:
			si = make_shared<IntegerStepSettingItem>(setting, group, key);
			break;
		case "Float"_crc32:
			si = make_shared<FloatStepSettingItem>(setting, group, key);
			break;
		case "Boolean"_crc32:
			si = make_shared<BooleanSettingItem>(setting, group, key);
			break;
		case "String"_crc32:
			si = make_shared<StringSettingItem>(setting, group, key);
			break;
		case "IntegerSelect"_crc32:
			si = make_shared<IntegerSelectSettingItem>(setting, group, key);
			break;
		case "FloatSelect"_crc32:
			si = make_shared<FloatSelectSettingItem>(setting, group, key);
			break;
		case "StringSelect"_crc32:
			si = make_shared<StringSelectSettingItem>(setting, group, key);
			break;
		case "IntegerList"_crc32:
			si = make_shared<IntegerListSettingItem>(setting, group, key);
			break;
		case "FloatList"_crc32:
			si = make_shared<FloatListSettingItem>(setting, group, key);
			break;
		case "BooleanList"_crc32:
			si = make_shared<BooleanListSettingItem>(setting, group, key);
			break;
		case "IntegerTable"_crc32:
			si = make_shared<IntegerTableSettingItem>(setting, group, key);
			break;
		case "FloatTable"_crc32:
			si = make_shared<FloatTableSettingItem>(setting, group, key);
			break;
		case "BooleanTable"_crc32:
			si = make_shared<BooleanTableSettingItem>(setting, group, key);
			break;
		case "StringTable"_crc32:
			si = make_shared<StringTableSettingItem>(setting, group, key);
			break;
		default:
			log->warn(u8"不明な設定タイプです: {0}", type);
			continue;
		}

		si->Build(item);
		this->items.emplace(fmt::format("{0}.{1}", group, key), si);
	}
	RetrieveAllValues();
}

/*!
 * @brief 現在保持しているすべての設定項目について、値の再読み込みを行います。
 */
void SettingManager::RetrieveAllValues()
{
	for (auto& si : items) si.second->RetrieveValue();
}

/*!
 * @brief 現在保持しているすべての設定項目について、値の保存を行います。
 * @note この関数を呼び出してもファイルへの書き出しは行われません。
 */
void SettingManager::SaveAllValues()
{
	for (auto& si : items) si.second->SaveValue();
}

/*!
 * @brief グループ名とキー名をもとに設定項目を取得します。
 * @param[in] group 設定項目のグループ名
 * @param[in] key 設定項目のキー名
 */
shared_ptr<SettingItem> SettingManager::GetSettingItem(const string & group, const string & key)
{
	const auto it = items.find(fmt::format("{0}.{1}", group, key));
	return (it != items.end()) ? it->second : nullptr;
}
