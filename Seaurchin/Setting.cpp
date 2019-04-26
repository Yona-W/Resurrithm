#include "Setting.h"
#include "Misc.h"

using namespace std;
using namespace std::filesystem;

std::filesystem::path Setting::rootDirectory = L"";

Setting::Setting(const HMODULE hModule)
{
	if (rootDirectory != "") return;
	wchar_t directory[MAX_PATH];
	GetModuleFileNameW(hModule, directory, MAX_PATH);
	PathRemoveFileSpecW(directory);
	rootDirectory = directory;
}

Setting::Setting()
{

}

void Setting::Load(const std::filesystem::path & filename)
{
	auto log = spdlog::get("main");
	file = filename;
	if (!exists(rootDirectory / file)) {
		log->info(u8"設定ファイルを作成しました");
		Save();
	}
	std::ifstream ifs((rootDirectory / file).wstring(), ios::in);
	auto pr = toml::parse(ifs);
	if (!pr.valid()) {
		log->error(u8"設定ファイルの記述が不正です: {0}", pr.errorReason);
	}
	else {
		settingTree = pr.value;
		log->info(u8"設定ファイルを読み込みました");
	}
	ifs.close();
}

std::filesystem::path Setting::GetRootDirectory()
{
	return rootDirectory;
}

void Setting::Save() const
{
	auto log = spdlog::get("main");
	std::ofstream ofs((rootDirectory / file).wstring(), ios::out);
	if (settingTree.valid()) settingTree.write(&ofs);
	log->info(u8"設定ファイルを保存しました");
	ofs.close();
}

namespace setting2
{

	// SettingItemManager

	SettingItemManager::SettingItemManager(const shared_ptr<Setting>& setting)
	{
		settingInstance = setting;
	}

	void SettingItemManager::LoadItemsFromToml(const path& file)
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
		auto& root = pr.value;
		const auto items = root.find("SettingItems");
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
				si = make_shared<IntegerSettingItem>(settingInstance, group, key);
				break;
			case "Float"_crc32:
				si = make_shared<FloatSettingItem>(settingInstance, group, key);
				break;
			case "Boolean"_crc32:
				si = make_shared<BooleanSettingItem>(settingInstance, group, key);
				break;
			case "String"_crc32:
				si = make_shared<StringSettingItem>(settingInstance, group, key);
				break;
			case "IntegerSelect"_crc32:
				si = make_shared<IntegerSelectSettingItem>(settingInstance, group, key);
				break;
			case "FloatSelect"_crc32:
				si = make_shared<FloatSelectSettingItem>(settingInstance, group, key);
				break;
			case "StringSelect"_crc32:
				si = make_shared<StringSelectSettingItem>(settingInstance, group, key);
				break;
			case "IntegerList"_crc32:
				si = make_shared<IntegerListSettingItem>(settingInstance, group, key);
				break;
			case "FloatList"_crc32:
				si = make_shared<FloatListSettingItem>(settingInstance, group, key);
				break;
			case "BooleanList"_crc32:
				si = make_shared<BooleanListSettingItem>(settingInstance, group, key);
				break;
			case "IntegerListVector"_crc32:
				si = make_shared<IntegerListVectorSettingItem>(settingInstance, group, key);
				break;
			case "FloatListVector"_crc32:
				si = make_shared<FloatListVectorSettingItem>(settingInstance, group, key);
				break;
			case "BooleanListVector"_crc32:
				si = make_shared<BooleanListVectorSettingItem>(settingInstance, group, key);
				break;
			default:
				log->warn(u8"不明な設定タイプです: {0}", type);
				continue;
			}
			si->Build(item);
			const auto name = si->GetSettingName();
			this->items[name] = si;
		}
	}

	void SettingItemManager::RetrieveAllValues()
	{
		for (auto& si : items) si.second->RetrieveValue();
	}

	void SettingItemManager::SaveAllValues()
	{
		for (auto& si : items) si.second->SaveValue();
	}

	shared_ptr<SettingItem> SettingItemManager::GetSettingItem(const string & group, const string & key)
	{
		const auto skey = fmt::format("{0}.{1}", group, key);
		if (items.find(skey) != items.end()) return items[skey];
		return nullptr;
	}

	shared_ptr<SettingItem> SettingItemManager::GetSettingItem(const string & name)
	{
		if (items.find(name) != items.end()) return items[name];
		return nullptr;
	}

	// SettingItem

	SettingItem::SettingItem(const shared_ptr<Setting> setting, const string & igroup, const string & ikey) : type(SettingItemType::Integer)
	{
		settingInstance = setting;
		group = igroup;
		key = ikey;
		description = u8"説明はありません";
		findName = "";
	}

	void SettingItem::Build(const toml::Value & table)
	{
		const auto d = table.find("Description");
		if (d && d->is<string>()) {
			description = d->as<string>();
		}
		const auto n = table.find("Name");
		if (n && n->is<string>()) {
			findName = n->as<string>();
		}
	}

	string SettingItem::GetSettingName() const
	{
		if (findName != "") return findName;
		return fmt::format("{0}.{1}", group, key);
	}

	string SettingItem::GetDescription() const
	{
		return description;
	}

	// IntegerSettingItem

	IntegerSettingItem::IntegerSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		value = defaultValue = 0;
		minValue = maxValue = 0;
		step = 1;
		type = SettingItemType::Integer;
	}

	string IntegerSettingItem::GetItemString()
	{
		return fmt::format("{0}", value);
	}

	void IntegerSettingItem::MoveNext()
	{
		value += step;
		if (value > maxValue) value = maxValue;
	}

	void IntegerSettingItem::MovePrevious()
	{
		value -= step;
		if (value < minValue) value = minValue;
	}

	void IntegerSettingItem::SaveValue()
	{
		settingInstance->WriteValue<int64_t>(group, key, value);
	}

	void IntegerSettingItem::RetrieveValue()
	{
		value = settingInstance->ReadValue(group, key, defaultValue);
	}

	void IntegerSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);

		auto log = spdlog::get("main");

		const auto r = table.find("Range");
		if (r && r->is<vector<int64_t>>()) {
			auto v = r->as<vector<int64_t>>();
			if (v.size() != 2) {
				log->warn(u8"整数型設定項目 {0}.{1} に対する範囲指定が不正です。", group, key);
			}
			else {
				minValue = v[0];
				maxValue = v[1];
			}
		}
		const auto s = table.find("Step");
		if (s && s->is<int64_t>()) {
			step = s->as<int64_t>();
		}
		const auto d = table.find("Default");
		if (d && d->is<int64_t>()) {
			defaultValue = d->as<int64_t>();
		}
	}

	// FloatSettingItem

	FloatSettingItem::FloatSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		value = defaultValue = 0;
		minValue = maxValue = 0;
		step = 1;
		type = SettingItemType::Float;
	}

	string FloatSettingItem::GetItemString()
	{
		return fmt::format("{0}", value);
	}

	void FloatSettingItem::MoveNext()
	{
		value += step;
		if (value > maxValue) value = maxValue;
	}

	void FloatSettingItem::MovePrevious()
	{
		value -= step;
		if (value < minValue) value = minValue;
	}

	void FloatSettingItem::SaveValue()
	{
		settingInstance->WriteValue<double>(group, key, value);
	}

	void FloatSettingItem::RetrieveValue()
	{
		value = settingInstance->ReadValue(group, key, defaultValue);
	}

	void FloatSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);

		auto log = spdlog::get("main");

		const auto r = table.find("Range");
		if (r && r->is<vector<double>>()) {
			auto v = r->as<vector<double>>();
			if (v.size() != 2) {
				log->warn(u8"実数型設定項目 {0}.{1} に対する範囲指定が不正です。", group, key);
			}
			else {
				minValue = v[0];
				maxValue = v[1];
			}
		}
		const auto s = table.find("Step");
		if (s && s->is<double>()) {
			step = s->as<double>();
		}
		const auto d = table.find("Default");
		if (d && d->is<double>()) {
			defaultValue = d->as<double>();
		}
	}

	// BooleanSettingItem

	BooleanSettingItem::BooleanSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		value = defaultValue = false;
		falsy = "false";
		truthy = "true";
		type = SettingItemType::Boolean;
	}

	string BooleanSettingItem::GetItemString()
	{
		return fmt::format("{0}", value ? truthy : falsy);
	}

	void BooleanSettingItem::MoveNext()
	{
		value = !value;
	}

	void BooleanSettingItem::MovePrevious()
	{
		value = !value;
	}

	void BooleanSettingItem::SaveValue()
	{
		settingInstance->WriteValue<bool>(group, key, value);
	}

	void BooleanSettingItem::RetrieveValue()
	{
		value = settingInstance->ReadValue(group, key, defaultValue);
	}

	void BooleanSettingItem::Build(const toml::Value & table)
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

	// StringSettingItem

	StringSettingItem::StringSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		value = defaultValue = "";
		type = SettingItemType::String;
	}

	string StringSettingItem::GetItemString()
	{
		return fmt::format("{0}", value);
	}

	void StringSettingItem::MoveNext()
	{}

	void StringSettingItem::MovePrevious()
	{}

	void StringSettingItem::SaveValue()
	{
		settingInstance->WriteValue<string>(group, key, value);
	}

	void StringSettingItem::RetrieveValue()
	{
		value = settingInstance->ReadValue(group, key, defaultValue);
	}

	void StringSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto d = table.find("Default");
		if (d && d->is<string>()) {
			defaultValue = d->as<string>();
		}
	}

	// IntegerSelectSettingItem

	IntegerSelectSettingItem::IntegerSelectSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		value = defaultValue = 0;
		type = SettingItemType::IntegerSelect;
	}

	string IntegerSelectSettingItem::GetItemString()
	{
		return fmt::format("{0}", value);
	}

	void IntegerSelectSettingItem::MoveNext()
	{
		selected = (selected + values.size() + 1) % values.size();
		value = values[selected];
	}

	void IntegerSelectSettingItem::MovePrevious()
	{
		selected = (selected + values.size() - 1) % values.size();
		value = values[selected];
	}

	void IntegerSelectSettingItem::SaveValue()
	{
		settingInstance->WriteValue<int64_t>(group, key, value);
	}

	void IntegerSelectSettingItem::RetrieveValue()
	{
		value = settingInstance->ReadValue(group, key, defaultValue);
	}

	void IntegerSelectSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto r = table.find("Values");
		if (r && r->is<vector<int64_t>>()) {
			auto v = r->as<vector<int64_t>>();
			for (const auto& val : v) values.push_back(val);
		}
		const auto d = table.find("Default");
		if (d && d->is<int64_t>()) {
			defaultValue = d->as<int64_t>();
		}
		selected = 0;
		if (values.size() == 0) values.push_back(defaultValue);
	}

	// FloatSelectSettingItem

	FloatSelectSettingItem::FloatSelectSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		value = defaultValue = 0;
		type = SettingItemType::FloatSelect;
	}

	string FloatSelectSettingItem::GetItemString()
	{
		return fmt::format("{0}", value);
	}

	void FloatSelectSettingItem::MoveNext()
	{
		selected = (selected + values.size() + 1) % values.size();
		value = values[selected];
	}

	void FloatSelectSettingItem::MovePrevious()
	{
		selected = (selected + values.size() - 1) % values.size();
		value = values[selected];
	}

	void FloatSelectSettingItem::SaveValue()
	{
		settingInstance->WriteValue<double>(group, key, value);
	}

	void FloatSelectSettingItem::RetrieveValue()
	{
		value = settingInstance->ReadValue(group, key, defaultValue);
	}

	void FloatSelectSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto r = table.find("Values");
		if (r && r->is<vector<double>>()) {
			auto v = r->as<vector<double>>();
			for (const auto& val : v) values.push_back(val);
		}
		const auto d = table.find("Default");
		if (d && d->is<double>()) {
			defaultValue = d->as<double>();
		}
		selected = 0;
		if (values.size() == 0) values.push_back(defaultValue);
	}

	// StringSelectSettingItem

	StringSelectSettingItem::StringSelectSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		value = defaultValue = "";
		type = SettingItemType::StringSelect;
	}

	string StringSelectSettingItem::GetItemString()
	{
		return fmt::format("{0}", value);
	}

	void StringSelectSettingItem::MoveNext()
	{
		selected = (selected + values.size() + 1) % values.size();
		value = values[selected];
	}

	void StringSelectSettingItem::MovePrevious()
	{
		selected = (selected + values.size() - 1) % values.size();
		value = values[selected];
	}

	void StringSelectSettingItem::SaveValue()
	{
		settingInstance->WriteValue<string>(group, key, value);
	}

	void StringSelectSettingItem::RetrieveValue()
	{
		value = settingInstance->ReadValue(group, key, defaultValue);
	}

	void StringSelectSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto r = table.find("Values");
		if (r && r->is<vector<string>>()) {
			auto v = r->as<vector<string>>();
			for (const auto& val : v) values.push_back(val);
		}
		const auto d = table.find("Default");
		if (d && d->is<string>()) {
			defaultValue = d->as<string>();
		}
		selected = 0;
		if (values.size() == 0) values.push_back(defaultValue);
	}

	// IntegerListSettingItem

	IntegerListSettingItem::IntegerListSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		defaultValues = std::vector<int64_t>(0);
		separator = ",";
		type = SettingItemType::IntegerList;
	}

	string IntegerListSettingItem::GetItemString()
	{
		return fmt::format("{0}", fmt::join(values.begin(), values.end(), separator));
	}

	void IntegerListSettingItem::MoveNext()
	{}

	void IntegerListSettingItem::MovePrevious()
	{}

	void IntegerListSettingItem::SaveValue()
	{
		toml::Array arr;
		for (const auto& val : values) arr.push_back(val);
		settingInstance->WriteValue<toml::Array>(group, key, arr);
	}

	void IntegerListSettingItem::RetrieveValue()
	{
		toml::Array arr;
		for (const auto& val : defaultValues) arr.push_back(val);
		arr = settingInstance->ReadValue(group, key, arr);

		auto log = spdlog::get("main");

		uint64_t cnt = 0;
		for (const auto& val : arr) {
			if (!val.is<int64_t>()) {
				log->warn(u8"整数型リスト設定項目 {0}.{1} の第{2}要素が整数型ではありません。", group, key, cnt);
				continue;
			}
			values.push_back(val.as<int64_t>());
			++cnt;
		}
	}

	void IntegerListSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto d = table.find("Default");
		if (d && d->is<std::vector<int64_t>>()) {
			defaultValues = d->as<std::vector<int64_t>>();
		}
		const auto s = table.find("Separator");
		if (s && s->is<std::string>()) {
			separator = s->as<std::string>();
		}
	}

	// FloatListSettingItem

	FloatListSettingItem::FloatListSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		defaultValues = std::vector<double>(0);
		separator = ",";
		type = SettingItemType::FloatList;
	}

	string FloatListSettingItem::GetItemString()
	{
		return fmt::format("{0}", fmt::join(values.begin(), values.end(), separator));
	}

	void FloatListSettingItem::MoveNext()
	{}

	void FloatListSettingItem::MovePrevious()
	{}

	void FloatListSettingItem::SaveValue()
	{
		toml::Array arr;
		for (const auto& val : values) arr.push_back(val);
		settingInstance->WriteValue<toml::Array>(group, key, arr);
	}

	void FloatListSettingItem::RetrieveValue()
	{
		toml::Array arr;
		for (const auto& val : defaultValues) arr.push_back(val);
		arr = settingInstance->ReadValue(group, key, arr);

		auto log = spdlog::get("main");

		uint64_t cnt = 0;
		for (const auto& val : arr) {
			if (!val.is<double>()) {
				log->warn(u8"実数型リスト設定項目 {0}.{1} の第{2}要素が実数型ではありません。", group, key, cnt);
				continue;
			}
			values.push_back(val.as<double>());
			++cnt;
		}
	}

	void FloatListSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto d = table.find("Default");
		if (d && d->is<std::vector<double>>()) {
			defaultValues = d->as<std::vector<double>>();
		}
		const auto s = table.find("Separator");
		if (s && s->is<std::string>()) {
			separator = s->as<std::string>();
		}
	}

	// BooleanListSettingItem

	BooleanListSettingItem::BooleanListSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		defaultValues = std::vector<bool>(0);
		separator = ",";
		type = SettingItemType::BooleanList;
	}

	string BooleanListSettingItem::GetItemString()
	{
		return fmt::format("{0}", fmt::join(values.begin(), values.end(), separator));
	}

	void BooleanListSettingItem::MoveNext()
	{}

	void BooleanListSettingItem::MovePrevious()
	{}

	void BooleanListSettingItem::SaveValue()
	{
		toml::Array arr;
		for (const auto& val : values) arr.push_back(toml::Value(val));
		settingInstance->WriteValue<toml::Array>(group, key, arr);
	}

	void BooleanListSettingItem::RetrieveValue()
	{
		toml::Array arr;
		for (const auto& val : defaultValues) arr.push_back(toml::Value(val));
		arr = settingInstance->ReadValue(group, key, arr);

		auto log = spdlog::get("main");

		uint64_t cnt = 0;
		for (const auto& val : arr) {
			if (!val.is<bool>()) {
				log->warn(u8"真偽値型リスト設定項目 {0}.{1} の第{2}要素が真偽値型ではありません。", group, key, cnt);
				continue;
			}
			values.push_back(val.as<bool>());
			++cnt;
		}
	}

	void BooleanListSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto d = table.find("Default");
		if (d && d->is<std::vector<bool>>()) {
			defaultValues = d->as<std::vector<bool>>();
		}
		const auto s = table.find("Separator");
		if (s && s->is<std::string>()) {
			separator = s->as<std::string>();
		}
	}

	// IntegerListVectorSettingItem

	IntegerListVectorSettingItem::IntegerListVectorSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		defaultValues = std::vector<std::vector<int64_t>>(0);
		valueSeparator = ",";
		listSeparator = "\r\n";
		type = SettingItemType::IntegerListVector;
	}

	string IntegerListVectorSettingItem::GetItemString()
	{
		std::vector<fmt::ArgJoin<char, std::vector<int64_t>::const_iterator>> joinedList;
		for (const auto& list : values) joinedList.push_back(fmt::join(list.begin(), list.end(), valueSeparator));
		return fmt::format("{0}", fmt::join(joinedList.begin(), joinedList.end(), listSeparator));
	}

	void IntegerListVectorSettingItem::MoveNext()
	{}

	void IntegerListVectorSettingItem::MovePrevious()
	{}

	void IntegerListVectorSettingItem::SaveValue()
	{
		toml::Array arr;
		for (const auto& list : values) {
			toml::Array tmp;
			for (const auto& val : list) tmp.push_back(val);
			arr.push_back(tmp);
		}
		settingInstance->WriteValue<toml::Array>(group, key, arr);
	}

	void IntegerListVectorSettingItem::RetrieveValue()
	{
		toml::Array arr;
		for (const auto& list : defaultValues) {
			toml::Array tmp;
			for (const auto& val : list) tmp.push_back(val);
			arr.push_back(tmp);
		}
		arr = settingInstance->ReadValue(group, key, arr);

		auto log = spdlog::get("main");

		uint64_t cnt = 0;
		for (const auto& list : arr) {
			if (!list.is<std::vector<int64_t>>()) {
				log->warn(u8"整数型2重リスト設定項目 {0}.{1} の第{2}要素が整数型リストではありません。", group, key, cnt);
				continue;
			}

			values.push_back(list.as<std::vector<int64_t>>());
			++cnt;
		}
	}

	void IntegerListVectorSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto d = table.find("Default");
		if (d && d->type() == toml::Value::Type::ARRAY_TYPE) {
			const auto arr = d->as<toml::Array>();
			for (auto i = 0u; i < arr.size(); ++i) {
				if (arr[i].is<std::vector<int64_t>>()) {
					defaultValues.push_back(arr[i].as<std::vector<int64_t>>());
				}
			}
		}
		const auto vs = table.find("ValueSeparator");
		if (vs && vs->is<std::string>()) {
			valueSeparator = vs->as<std::string>();
		}
		const auto ls = table.find("ListSeparator");
		if (ls && ls->is<std::string>()) {
			listSeparator = ls->as<std::string>();
		}
	}

	// FloatListVectorSettingItem

	FloatListVectorSettingItem::FloatListVectorSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		defaultValues = std::vector<std::vector<double>>(0);
		valueSeparator = ",";
		listSeparator = "\r\n";
		type = SettingItemType::FloatListVector;
	}

	string FloatListVectorSettingItem::GetItemString()
	{
		std::vector<fmt::ArgJoin<char, std::vector<double>::const_iterator>> joinedList;
		for (const auto& list : values) joinedList.push_back(fmt::join(list.begin(), list.end(), valueSeparator));
		return fmt::format("{0}", fmt::join(joinedList.begin(), joinedList.end(), listSeparator));
	}

	void FloatListVectorSettingItem::MoveNext()
	{}

	void FloatListVectorSettingItem::MovePrevious()
	{}

	void FloatListVectorSettingItem::SaveValue()
	{
		toml::Array arr;
		for (const auto& list : values) {
			toml::Array tmp;
			for (const auto& val : list) tmp.push_back(val);
			arr.push_back(tmp);
		}
		settingInstance->WriteValue<toml::Array>(group, key, arr);
	}

	void FloatListVectorSettingItem::RetrieveValue()
	{
		toml::Array arr;
		for (const auto& list : defaultValues) {
			toml::Array tmp;
			for (const auto& val : list) tmp.push_back(val);
			arr.push_back(tmp);
		}
		arr = settingInstance->ReadValue(group, key, arr);

		auto log = spdlog::get("main");

		uint64_t cnt = 0;
		for (const auto& list : arr) {
			if (!list.is<std::vector<double>>()) {
				log->warn(u8"実数型2重リスト設定項目 {0}.{1} の第{2}要素が実数型リストではありません。", group, key, cnt);
				continue;
			}

			values.push_back(list.as<std::vector<double>>());
			++cnt;
		}
	}

	void FloatListVectorSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto d = table.find("Default");
		if (d && d->type() == toml::Value::Type::ARRAY_TYPE) {
			const auto arr = d->as<toml::Array>();
			for (auto i = 0u; i < arr.size(); ++i) {
				if (arr[i].is<std::vector<double>>()) {
					defaultValues.push_back(arr[i].as<std::vector<double>>());
				}
			}
		}
		const auto vs = table.find("ValueSeparator");
		if (vs && vs->is<std::string>()) {
			valueSeparator = vs->as<std::string>();
		}
		const auto ls = table.find("ListSeparator");
		if (ls && ls->is<std::string>()) {
			listSeparator = ls->as<std::string>();
		}
	}

	// BooleanListVectorSettingItem

	BooleanListVectorSettingItem::BooleanListVectorSettingItem(const std::shared_ptr<Setting> setting, const std::string & group, const std::string & key) : SettingItem(setting, group, key)
	{
		defaultValues = std::vector<std::vector<bool>>(0);
		valueSeparator = ",";
		listSeparator = "\r\n";
		type = SettingItemType::BooleanListVector;
	}

	string BooleanListVectorSettingItem::GetItemString()
	{
		std::vector<fmt::ArgJoin<char, std::vector<bool>::const_iterator>> joinedList;
		for (const auto& list : values) joinedList.push_back(fmt::join(list.begin(), list.end(), valueSeparator));
		return fmt::format("{0}", fmt::join(joinedList.begin(), joinedList.end(), listSeparator));
	}

	void BooleanListVectorSettingItem::MoveNext()
	{}

	void BooleanListVectorSettingItem::MovePrevious()
	{}

	void BooleanListVectorSettingItem::SaveValue()
	{
		toml::Array arr;
		for (const auto& list : values) {
			toml::Array tmp;
			for (const auto& val : list) tmp.push_back(toml::Value(val));
			arr.push_back(tmp);
		}
		settingInstance->WriteValue<toml::Array>(group, key, arr);
	}

	void BooleanListVectorSettingItem::RetrieveValue()
	{
		toml::Array arr;
		for (const auto& list : defaultValues) {
			toml::Array tmp;
			for (const auto& val : list) tmp.push_back(toml::Value(val));
			arr.push_back(tmp);
		}
		arr = settingInstance->ReadValue(group, key, arr);

		auto log = spdlog::get("main");

		uint64_t cnt = 0;
		for (const auto& list : arr) {
			if (!list.is<std::vector<bool>>()) {
				log->warn(u8"真偽値型2重リスト設定項目 {0}.{1} の第{2}要素が真偽値型リストではありません。", group, key, cnt);
				continue;
			}

			values.push_back(list.as<std::vector<bool>>());
			++cnt;
		}
	}

	void BooleanListVectorSettingItem::Build(const toml::Value & table)
	{
		SettingItem::Build(table);
		const auto d = table.find("Default");
		if (d && d->type() == toml::Value::Type::ARRAY_TYPE) {
			const auto arr = d->as<toml::Array>();
			for (auto i = 0u; i < arr.size(); ++i) {
				if (arr[i].is<std::vector<bool>>()) {
					defaultValues.push_back(arr[i].as<std::vector<bool>>());
				}
			}
		}
		const auto vs = table.find("ValueSeparator");
		if (vs && vs->is<std::string>()) {
			valueSeparator = vs->as<std::string>();
		}
		const auto ls = table.find("ListSeparator");
		if (ls && ls->is<std::string>()) {
			listSeparator = ls->as<std::string>();
		}
	}

}
