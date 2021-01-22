#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <format.hpp>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/algorithm/algorithm.hpp>
#include <toml/toml.h>
#include <spdlog/spdlog.h>
#include "Crc32.h"

#define SU_SETTING_GENERAL "General"
#define SU_SETTING_SKIN "Skin"

class Setting final {
private:
    static std::string rootDirectory;
    std::string file;
    toml::Value settingTree;

public:
    Setting();
    void Load(const std::string &filename);
    void Reload() { Load(file); }
    void Save() const;
    static std::string GetRootDirectory();

    template<typename T>
    T ReadValue(const std::string &group, const std::string &key, T defValue)
    {
        auto v = settingTree.find(group + "." + key);
        if (v && v->is<T>()) {
            return v->as<T>();
        }
        WriteValue(group, key, defValue);
        return defValue;
    }

    template<typename T>
    void WriteValue(const std::string &group, const std::string &key, T value)
    {
        settingTree.set(group + "." + key, value);
    }
};

namespace setting2
{

enum class SettingItemType {
    Integer,
    Float,
    Boolean,
    String,
    IntegerSelect,
    FloatSelect,
    StringSelect,
    IntegerList,
    FloatList,
    BooleanList,
    IntegerListVector,
    FloatListVector,
    BooleanListVector,
};

class SettingItem {
protected:
    std::shared_ptr<Setting> settingInstance;
    SettingItemType type;
    std::string description;
    std::string group;
    std::string key;
    std::string findName;

public:
    virtual ~SettingItem() = default;
    SettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetSettingName() const;
    std::string GetDescription() const;

    virtual std::string GetItemString() = 0;
    virtual void MoveNext() = 0;
    virtual void MovePrevious() = 0;
    virtual void SaveValue() = 0;
    virtual void RetrieveValue() = 0;
    virtual void Build(const toml::Value &table);
};

class IntegerSettingItem final : public SettingItem {
private:
    int64_t value;
    int64_t minValue;
    int64_t maxValue;
    int64_t step;
    int64_t defaultValue;

public:
    IntegerSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class FloatSettingItem final : public SettingItem {
private:
    double value;
    double minValue;
    double maxValue;
    double step;
    double defaultValue;

public:
    FloatSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class BooleanSettingItem final : public SettingItem {
private:
    bool value;
    std::string truthy;
    std::string falsy;
    bool defaultValue;

public:
    BooleanSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class StringSettingItem final : public SettingItem {
private:
    std::string value;
    std::string defaultValue;

public:
    StringSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class IntegerSelectSettingItem final : public SettingItem {
private:
    int64_t value;
    int64_t defaultValue;
    int selected = -1;
    std::vector<int64_t> values;

public:
    IntegerSelectSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class FloatSelectSettingItem final : public SettingItem {
private:
    double value;
    double defaultValue;
    int selected = -1;
    std::vector<double> values;

public:
    FloatSelectSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class StringSelectSettingItem final : public SettingItem {
private:
    std::string value;
    std::string defaultValue;
    int selected = -1;
    std::vector<std::string> values;

public:
    StringSelectSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class IntegerListSettingItem final : public SettingItem {
private:
    std::vector<int64_t> values;
    std::vector<int64_t> defaultValues;
    std::string separator;

public:
    IntegerListSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class FloatListSettingItem final : public SettingItem {
private:
    std::vector<double> values;
    std::vector<double> defaultValues;
    std::string separator;

public:
    FloatListSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class BooleanListSettingItem final : public SettingItem {
private:
    std::vector<bool> values;
    std::vector<bool> defaultValues;
    std::string separator;

public:
    BooleanListSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class IntegerListVectorSettingItem final : public SettingItem {
private:
    std::vector<std::vector<int64_t>> values;
    std::vector<std::vector<int64_t>> defaultValues;
    std::string valueSeparator;
    std::string listSeparator;

public:
    IntegerListVectorSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class FloatListVectorSettingItem final : public SettingItem {
private:
    std::vector<std::vector<double>> values;
    std::vector<std::vector<double>> defaultValues;
    std::string valueSeparator;
    std::string listSeparator;

public:
    FloatListVectorSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};

class BooleanListVectorSettingItem final : public SettingItem {
private:
    std::vector<std::vector<bool>> values;
    std::vector<std::vector<bool>> defaultValues;
    std::string valueSeparator;
    std::string listSeparator;

public:
    BooleanListVectorSettingItem(std::shared_ptr<Setting> setting, const std::string &group, const std::string &key);
    std::string GetItemString() override;
    void MoveNext() override;
    void MovePrevious() override;
    void SaveValue() override;
    void RetrieveValue() override;
    void Build(const toml::Value &table) override;
};


class SettingItemManager final {
private:
    std::shared_ptr<Setting> settingInstance;
    std::unordered_map<std::string, std::shared_ptr<SettingItem>> items;

public:
    explicit SettingItemManager(const std::shared_ptr<Setting>& setting);
    void LoadItemsFromToml(const boost::filesystem::path& file);
    void RetrieveAllValues();
    void SaveAllValues();

    std::shared_ptr<SettingItem> GetSettingItem(const std::string &group, const std::string &key);
    std::shared_ptr<SettingItem> GetSettingItem(const std::string &name);
};

}
