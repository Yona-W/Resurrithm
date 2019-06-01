/*!
 * @file Setting.h
 * @brief 設定項目の操作を行うクラス SettingItem と関連クラスの宣言
 * @author kb10uy
 * @date 2019/04/29
 * @details 設定項目定義ファイルの読み込み、パース、設定項目取得を行うインターフェースを提供します。
 */

#pragma once

/*!
 * @brief 全設定項目の設定値を管理します
 * @details 全ての設定項目の設定値が[グループ名].[キー名]で参照可能な1階層の toml::Table に格納されます。
 * 設定値の型情報があらかじめ判明している場合はグループ名、キー名から直接読み書き可能ですが、
 * 原則として設定項目(SettingItem)を経由して参照することが期待されます。
 * @note デストラクタで自動的に保存されます。ログを吐く都合上、ロガーよりも先に破棄される必要があります。
 * @note 対象となる設定ファイルは rootDir/filename です。
 * @todo ルートディレクトリ管理は別のクラスにやらせる
 */
class SettingTree final {
private:
	const std::filesystem::path rootDir;	//!< アプリケーションのワーキングディレクトリへの絶対パス
	const std::filesystem::path fileName;	//!< 設定ファイル名
	toml::Value setting;					//!< 設定値のテーブル

public:
	//! @param[in] hModule アプリケーションのモジュールハンドル。
	//! @param[in] filename 設定ファイル名。
	explicit SettingTree(HMODULE hModule, const std::filesystem::path& filename);

	//! @note 自動的に保存します。
	~SettingTree() { Save(); }

	//! @brief 設定ファイルから設定値を読み込みます。
	//! @note 設定ファイルが存在しない場合、現在の設定をファイルへ書き出します。
	//! @return 読み込みに失敗するとfalseが返ります。
	bool Load();

	//! @brief 設定値を再読み込みします。
	//! @return 再読み込みに失敗するとfalseが返ります。
	bool Reload() { return Load(); }

	//! @brief 設定ファイルへ設定値を書きだします。
	void Save() const;

	//! @brief アプリケーションのワーキングディレクトリへの絶対パスを取得します。
	//! @return アプリケーションのワーキングディレクトリへの絶対パス。
	const std::filesystem::path GetRootDir() const { return rootDir; }

	//! @brief 全設定項目の設定値を格納したテーブルを取得します。
	//! @return 全設定項目の設定値を格納した1oml::Table。[グループ名].[キー名]で設定値を参照できます。
	const toml::Value& GetRoot() const { return setting; }

	//! @brief グループ名を指定して設定値集合を取得します。
	//!  @param[in] group 取得したい設定値の属するグループ名。
	//!  @return 設定値集合からなるtoml::Valueの生ポインタ。該当する設定値が存在しない場合nullが返ります。
	const toml::Value* GetGroup(const std::string& group) const;

	//! @brief グループ名、キー名を指定して設定値を取得します。
	//! @param[in] group 取得したい設定値の属するグループ名。
	//! @param[in] key 取得したい設定値の属するキー名。
	//! @return 設定値を格納したtoml::Valueの生ポインタ。該当する設定値が存在しない場合nullが返ります。
	const toml::Value* GetValues(const std::string& group, const std::string& key) const;

	//! @brief グループ名、キー名に対応する設定項目の設定値を取得します。
	//! @tparam T 設定値として期待される型。
	//! @param group 設定項目のグループ名。
	//! @param key 設定項目のキー名。
	//! @param defValue デフォルト値。設定値が存在しない/期待した型の値ではない場合に用いられます。
	//! @return 引数に対応する設定項目の設定値。設定値が存在しない/期待した型の値ではない場合、 defValue が返ります。
	//! @note  設定値が存在しない/期待した型の値ではない場合、設定値として defValue が設定されます。
	template<typename T>
	T ReadValue(const std::string& group, const std::string& key, T defValue)
	{
		const auto v = GetValues(group, key);
		if (v && v->is<T>()) return v->as<T>();
		WriteValue(group, key, defValue);
		return defValue;
	}

	//! @brief グループ名、キー名に対応する設定項目の設定値を設定します。
	//! @tparam T 設定値の型。
	//! @param group 設定項目のグループ名。
	//! @param key 設定項目のキー名。
	//! @param value 設定する値。
	//! @note  現在の設定値の型/値に関係なく設定値を書き変えます。
	template<typename T>
	void WriteValue(const std::string& group, const std::string& key, T value)
	{
		setting.set(group + "." + key, value);
	}
};


/*!
 * @brief 設定項目として設定値を操作するインターフェースを提供します
 * @details 設定項目定義に従い設定値を操作する設定項目クラスの基底クラスです。
 */
class SettingItem {
protected:
	SettingTree* const settingInstance;	//!< 設定値管理クラスへの参照

	std::string description;	//!< 説明(概要)
	std::string group;			//!< グループ名
	std::string key;			//!< キー名

public:
	//! @param setting この設定項目で操作する設定値を保持する設定値管理クラス。
	//! @param group この設定項目で操作する設定値のグループ名。
	//! @param key この設定項目で操作する設定値のキー名。
	SettingItem(SettingTree* setting, const std::string& group, const std::string& key);
	virtual ~SettingItem() = default;

	//! @brief 設定項目に対する説明(概要)を取得します。
	//! @return 説明(概要)。
	std::string GetDescription() const { return description; }

	//! @brief 設定項目の設定値を文字列として取得します。
	//! @return 文字列化した設定値。
	virtual std::string GetItemString() = 0;

	//! @brief 設定値を次の値へ変更します。
	virtual void MoveNext() = 0;

	//! @brief 設定値を前の値へ変更します。
	virtual void MovePrevious() = 0;

	//! @brief 設定値を反映します。
	//! @note 設定ファイルへ保存するのではなく、設定値管理クラスへ反映するのみです。
	virtual void SaveValue() = 0;

	//! @brief 設定値を読み込みます。
	//! @note 設定ファイルから読み込むのではなく、設定値管理クラスに設定された値を読み込むのみです。
	virtual void RetrieveValue() = 0;

	//! @brief 設定項目定義をメンバへ反映します。
	//! @param table この設定項目に対する設定項目定義を格納したtoml::Table。
	virtual void Build(const toml::Value& table);
};

/*!
 * @brief 区間[最小値, 最大値]内でstep刻みで変更可能な設定項目を定義するテンプレートクラス。
 * @tparam T 設定値に用いる型。
 * @tparam min 最小値。
 * @tparam max 最大値。
 * @tparam step 刻み幅。
 * @note 非型テンプレートの制約上、Tはint型から変換可能な型に限られます。
 */
template<typename T, int min, int max, int step>
class StepSettingItem final : public SettingItem {
private:
	T value;		//!< 現在の値
	T minValue;		//!< 最小値
	T maxValue;		//!< 最大値
	T step;			//!< 変化幅
	T defaultValue;	//!< 初期値

public:
	//! @param setting この設定項目で操作する設定値を保持する設定値管理クラス。
	//! @param group この設定項目で操作する設定値のグループ名。
	//! @param key この設定項目で操作する設定値のキー名。
	StepSettingItem(SettingTree* setting, const std::string& group, const std::string& key)
		: SettingItem(setting, group, key)
		, value(T())
		, minValue(min)
		, maxValue(max)
		, step(step)
		, defaultValue(T())
	{}

	//! @brief 設定項目の設定値を文字列として取得します。
	//! @return 文字列化した設定値。
	std::string GetItemString() override { return fmt::format("{0}", value); }

	//! @brief 設定値を次の値へ変更します。
	void MoveNext() override { if ((value += step) > maxValue) value = maxValue; }

	//! @brief 設定値を前の値へ変更します。
	void MovePrevious() override { if ((value -= step) < minValue) value = minValue; }

	//! @brief 設定値を反映します。
	//! @note 設定ファイルへ保存するのではなく、設定値管理クラスへ反映するのみです。
	void SaveValue() override { settingInstance->WriteValue<T>(group, key, value); }

	//! @brief 設定値を読み込みます。
	//! @note 設定ファイルから読み込むのではなく、設定値管理クラスに設定された値を読み込むのみです。
	void RetrieveValue() override { value = settingInstance->ReadValue(group, key, defaultValue); }

	//! @brief 設定項目定義をメンバへ反映します。
	//! @param table この設定項目に対する設定項目定義を格納したtoml::Table。
	void Build(const toml::Value& table) override
	{
		SettingItem::Build(table);

		auto log = spdlog::get("main");

		const auto r = table.find("Range");
		if (r && r->is<vector<T>>()) {
			const auto v = r->as<vector<T>>();
			if (v.size() != 2) {
				log->warn(u8"設定項目 {0}.{1} に対する範囲指定が不正です。", group, key);
			}
			else {
				minValue = v[0];
				maxValue = v[1];
			}
		}
		const auto s = table.find("Step");
		if (s && s->is<T>()) {
			step = s->as<T>();
		}
		const auto d = table.find("Default");
		if (d && d->is<T>()) {
			defaultValue = d->as<T>();
		}
	}
};

/*!
 * @brief 区間[最小値, 最大値]内でstep刻みで変更可能なint型の設定値を持つ設定項目。
 * @note デフォルトの区間は[0, 0]、刻み幅は1です。
 */
typedef StepSettingItem<int64_t, 0, 0, 1> IntegerStepSettingItem;

/*!
 * @brief 区間[最小値, 最大値]内でstep刻みで変更可能なdouble型の設定値を持つ設定項目。
 * @note デフォルトの区間は[0, 0]、刻み幅は1です。
 */
typedef StepSettingItem<double, 0, 0, 1> FloatStepSettingItem;

/*!
 * @brief 真偽値を設定値にもつ設定項目。
 * @note 取得できる値はtrue/falseそれぞれに紐づけられた文字列です。
 */
class BooleanSettingItem final : public SettingItem {
private:
	bool value;			//!< 現在の値
	std::string truthy;	//!< trueの時の表示文字列
	std::string falsy;	//!< falseの時の表示文字列
	bool defaultValue;	//!< 初期値

public:
	//! @param setting この設定項目で操作する設定値を保持する設定値管理クラス。
	//! @param group この設定項目で操作する設定値のグループ名。
	//! @param key この設定項目で操作する設定値のキー名。
	BooleanSettingItem(SettingTree* setting, const std::string& group, const std::string& key);

	//! @brief 設定項目の設定値を文字列として取得します。
	//! @return 文字列化した設定値。
	std::string GetItemString() override { return value ? truthy : falsy; }

	//! @brief 設定値を次の値へ変更します。
	void MoveNext() override { value = !value; }

	//! @brief 設定値を前の値へ変更します。
	void MovePrevious() override { value = !value; }

	//! @brief 設定値を反映します。
	//! @note 設定ファイルへ保存するのではなく、設定値管理クラスへ反映するのみです。
	void SaveValue() override { settingInstance->WriteValue<bool>(group, key, value); }

	//! @brief 設定値を読み込みます。
	//! @note 設定ファイルから読み込むのではなく、設定値管理クラスに設定された値を読み込むのみです。
	void RetrieveValue() override { value = settingInstance->ReadValue(group, key, defaultValue); }

	//! @brief 設定項目定義をメンバへ反映します。
	//! @param table この設定項目に対する設定項目定義を格納したtoml::Table。
	void Build(const toml::Value& table) override;
};

/*!
 * @brief 文字列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
class StringSettingItem final : public SettingItem {
private:
	std::string value;			//!< 現在の値
	std::string defaultValue;	//!< 初期値

public:
	//! @param setting この設定項目で操作する設定値を保持する設定値管理クラス。
	//! @param group この設定項目で操作する設定値のグループ名。
	//! @param key この設定項目で操作する設定値のキー名。
	StringSettingItem(SettingTree* setting, const std::string& group, const std::string& key);

	//! @brief 設定項目の設定値を文字列として取得します。
	//! @return 文字列化した設定値。
	std::string GetItemString() override { return value; }

	//! @brief 設定値を次の値へ変更します。
	void MoveNext() override {}

	//! @brief 設定値を前の値へ変更します。
	void MovePrevious() override {}

	//! @brief 設定値を反映します。
	//! @note 設定ファイルへ保存するのではなく、設定値管理クラスへ反映するのみです。
	void SaveValue() override { settingInstance->WriteValue<std::string>(group, key, value); }

	//! @brief 設定値を読み込みます。
	//! @note 設定ファイルから読み込むのではなく、設定値管理クラスに設定された値を読み込むのみです。
	void RetrieveValue() override { value = settingInstance->ReadValue(group, key, defaultValue); }

	//! @brief 設定項目定義をメンバへ反映します。
	//! @param table この設定項目に対する設定項目定義を格納したtoml::Table。
	void Build(const toml::Value& table) override;
};

/*!
 * @brief 設定値をリストに含まれる値から選択可能な設定項目を定義するテンプレートクラス。
 * @tparam T 設定値に用いる型。
 * @todo Retriveした際のインデックス修正を丁寧に行いたい
 */
template<typename T>
class SelectSettingItem final : public SettingItem {
public:
	static constexpr size_t MaxItemCount = 10000000;	//!< 保持可能な最大項目数
private:
	T value;				//!< 現在の値
	T defaultValue;			//!< 初期値
	int selected;			//!< 選択中の値のインデックス
	std::vector<T> values;	//!< 選択肢のリスト

public:
	//! @param setting この設定項目で操作する設定値を保持する設定値管理クラス。
	//! @param group この設定項目で操作する設定値のグループ名。
	//! @param key この設定項目で操作する設定値のキー名。
	SelectSettingItem(SettingTree* setting, const std::string& group, const std::string& key)
		: SettingItem(setting, group, key)
		, value(T())
		, defaultValue(T())
		, selected(-1)
	{}

	//! @brief 設定項目の設定値を文字列として取得します。
	//! @return 文字列化した設定値。
	std::string GetItemString() override { return fmt::format("{0}", value); }

	//! @brief 設定値を次の値へ変更します。
	void MoveNext() override { value = values[selected = (selected + 1) % SU_TO_INT(values.size())]; }

	//! @brief 設定値を前の値へ変更します。
	void MovePrevious() override { value = values[selected = (selected + SU_TO_INT(values.size()) - 1) % SU_TO_INT(values.size())]; }

	//! @brief 設定値を反映します。
	//! @note 設定ファイルへ保存するのではなく、設定値管理クラスへ反映するのみです。
	void SaveValue() override { settingInstance->WriteValue<T>(group, key, value); }

	//! @brief 設定値を読み込みます。
	//! @note 設定ファイルから読み込むのではなく、設定値管理クラスに設定された値を読み込むのみです。
	void RetrieveValue() override { value = settingInstance->ReadValue(group, key, defaultValue); }

	//! @brief 設定項目定義をメンバへ反映します。
	//! @param table この設定項目に対する設定項目定義を格納したtoml::Table。
	void Build(const toml::Value& table) override
	{
		SettingItem::Build(table);

		const auto r = table.find("Values");
		if (r && r->is<vector<T>>()) {
			auto v = r->as<vector<T>>();
			for (const auto& val : v) {
				values.push_back(val);
				if (values.size() >= MaxItemCount) {
					spdlog::get("main")->warn(u8"保持可能最大項目数に達したため、解析を終了します。");
					break;
				}
			}
		}
		const auto d = table.find("Default");
		if (d && d->is<T>()) {
			defaultValue = d->as<T>();
		}
		selected = 0;
		if (values.size() == 0) values.push_back(defaultValue);
	}
};

//! @brief 設定値をリストに含まれる値から選択可能なint型設定値を持つ設定項目。
typedef SelectSettingItem<int64_t> IntegerSelectSettingItem;

//! @brief 設定値をリストに含まれる値から選択可能なdouble型設定値を持つ設定項目。
typedef SelectSettingItem<double> FloatSelectSettingItem;

//! @brief 設定値をリストに含まれる値から選択可能な文字列型設定値を持つ設定項目。
typedef SelectSettingItem<std::string> StringSelectSettingItem;

/*!
 * @brief 一次元配列を設定値にもつ設定項目定義するテンプレートクラス。
 * @tparam T 設定値に用いる型。
 * @note 現状外部から設定値を変更できません。
 */
template<typename T>
class ListSettingItem final : public SettingItem {
private:
	std::vector<T> values;			//!< 現在の値
	std::vector<T> defaultValues;	//!< 初期値
	std::string separator;			//!< 文字列化に用いる区切り文字

public:
	//! @param setting この設定項目で操作する設定値を保持する設定値管理クラス。
	//! @param group この設定項目で操作する設定値のグループ名。
	//! @param key この設定項目で操作する設定値のキー名。
	ListSettingItem(SettingTree* setting, const std::string& group, const std::string& key)
		: SettingItem(setting, group, key)
		, values()
		, defaultValues()
		, separator(",")
	{}

	//! @brief 設定項目の設定値を文字列として取得します。
	//! @return 文字列化した設定値。
	std::string GetItemString() override { return fmt::format("{0}", fmt::join(values.begin(), values.end(), separator)); }

	//! @brief 設定値を次の値へ変更します。
	void MoveNext() override {}

	//! @brief 設定値を前の値へ変更します。
	void MovePrevious() override {}

	//! @brief 設定値を反映します。
	//! @note 設定ファイルへ保存するのではなく、設定値管理クラスへ反映するのみです。
	void SaveValue() override
	{
		toml::Array arr;
		for (const T& val : values) arr.push_back(val);
		settingInstance->WriteValue(group, key, arr);
	}

	//! @brief 設定値を読み込みます。
	//! @note 設定ファイルから読み込むのではなく、設定値管理クラスに設定された値を読み込むのみです。
	void RetrieveValue() override
	{
		toml::Array arr;
		for (const T& val : defaultValues) arr.push_back(val);
		arr = settingInstance->ReadValue(group, key, arr);

		auto log = spdlog::get("main");

		values.clear();
		uint64_t cnt = 0;
		for (const auto& val : arr) {
			++cnt;
			if (!val.is<T>()) {
				log->warn(u8"リスト設定項目 {0}.{1} の第{2}要素の型が一致しません。", group, key, cnt);
				continue;
			}
			values.push_back(val.as<T>());
		}
	}

	//! @brief 設定項目定義をメンバへ反映します。
	//! @param table この設定項目に対する設定項目定義を格納したtoml::Table。
	void Build(const toml::Value& table) override
	{
		SettingItem::Build(table);

		const auto d = table.find("Default");
		if (d && d->is<std::vector<T>>()) {
			defaultValues = d->as<std::vector<T>>();
		}
		const auto s = table.find("Separator");
		if (s && s->is<std::string>()) {
			separator = s->as<std::string>();
		}
	}
};

/*!
 * @brief int型一次元配列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
typedef ListSettingItem<int64_t> IntegerListSettingItem;

/*!
 * @brief double型一次元配列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
typedef ListSettingItem<double> FloatListSettingItem;

/*!
 * @brief 真偽値の一次元配列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
typedef ListSettingItem<bool> BooleanListSettingItem;

/*!
 * @brief 文字列型一次元配列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
typedef ListSettingItem<std::string> StringListSettingItem;

/*!
 * @brief 二次元配列を設定値にもつ設定項目定義するテンプレートクラス。
 * @tparam T 設定値に用いる型。
 * @note 現状外部から設定値を変更できません。
 */
template<typename T>
class TableSettingItem final : public SettingItem {
private:
	std::vector<std::vector<T>> values;			//!< 現在の値
	std::vector<std::vector<T>> defaultValues;	//!< 初期値
	std::string valueSeparator;					//!< 内側のリストの文字列化に用いる区切り文字
	std::string listSeparator;					//!< 外側のリストの文字列化に用いる区切り文字

public:
	//! @param setting この設定項目で操作する設定値を保持する設定値管理クラス。
	//! @param group この設定項目で操作する設定値のグループ名。
	//! @param key この設定項目で操作する設定値のキー名。
	TableSettingItem(SettingTree* setting, const std::string& group, const std::string& key)
		: SettingItem(setting, group, key)
		, values()
		, defaultValues()
		, valueSeparator(",")
		, listSeparator("\r\n")
	{}

	//! @brief 設定項目の設定値を文字列として取得します。
	//! @return 文字列化した設定値。
	std::string GetItemString() override
	{
		std::vector<fmt::ArgJoin<char, std::vector<T>::const_iterator>> joinedList;
		for (const auto& list : values) joinedList.push_back(fmt::join(list.begin(), list.end(), valueSeparator));
		return fmt::format("{0}", fmt::join(joinedList.begin(), joinedList.end(), listSeparator));
	}

	//! @brief 設定値を次の値へ変更します。
	void MoveNext() override {}

	//! @brief 設定値を前の値へ変更します。
	void MovePrevious() override {}

	//! @brief 設定値を反映します。
	//! @note 設定ファイルへ保存するのではなく、設定値管理クラスへ反映するのみです。
	void SaveValue() override
	{
		toml::Array arr;
		for (const std::vector<T>& list : values) {
			toml::Array tmp;
			for (const T& val : list) tmp.push_back(val);
			arr.push_back(tmp);
		}
		settingInstance->WriteValue(group, key, arr);
	}

	//! @brief 設定値を読み込みます。
	//! @note 設定ファイルから読み込むのではなく、設定値管理クラスに設定された値を読み込むのみです。
	void RetrieveValue() override
	{
		toml::Array arr;
		for (const std::vector<T>& list : defaultValues) {
			toml::Array tmp;
			for (const T& val : list) tmp.push_back(val);
			arr.push_back(tmp);
		}
		arr = settingInstance->ReadValue(group, key, arr);

		auto log = spdlog::get("main");

		values.clear();
		uint64_t cnt = 0;
		for (const auto& list : arr) {
			++cnt;
			if (!list.is<std::vector<T>>()) {
				log->warn(u8"2重リスト設定項目 {0}.{1} の第{2}リストの方が一致しません。", group, key, cnt);
				continue;
			}
			values.push_back(list.as<std::vector<T>>());
		}
	}

	//! @brief 設定項目定義をメンバへ反映します。
	//! @param table この設定項目に対する設定項目定義を格納したtoml::Table。
	void Build(const toml::Value& table) override
	{
		SettingItem::Build(table);

		const auto d = table.find("Default");
		if (d && d->type() == toml::Value::Type::ARRAY_TYPE) {
			const auto arr = d->as<toml::Array>();
			for (auto i = 0u; i < arr.size(); ++i) {
				if (arr[i].is<std::vector<T>>()) {
					defaultValues.push_back(arr[i].as<std::vector<T>>());
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
};

/*!
 * @brief int型二次元配列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
typedef TableSettingItem<int64_t> IntegerTableSettingItem;

/*!
 * @brief double型二次元配列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
typedef TableSettingItem<double> FloatTableSettingItem;

/*!
 * @brief 真偽値の二次元配列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
typedef TableSettingItem<bool> BooleanTableSettingItem;

/*!
 * @brief 文字列型二次元配列を設定値にもつ設定項目。
 * @note 現状外部から設定値を変更できません。
 */
typedef TableSettingItem<std::string> StringTableSettingItem;
