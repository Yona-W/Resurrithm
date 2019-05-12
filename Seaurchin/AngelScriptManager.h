/*!
 * @file AngelScriptManager.h
 * @brief AngelScriptの管理を行うクラス AngelScriptManager の宣言
 * @author kb10uy
 * @date 2019/05/08
 * @details スクリプトのロード、実行を行うインターフェースを提供します。
 */

#pragma once

/*!
 * @brief AngelScriptのエンジンを保持し、スクリプトのロードなどを行います。
 */
class AngelScript {
private:
	asIScriptEngine* const engine;		//!< スクリプトを実行するエンジン
	asIScriptContext* sharedContext;	//!< アプリケーション全体で共有されるコンテキスト
	CWScriptBuilder builder;			//!< スクリプトファイルのロードなどを実際に行うビルダー

public:
	AngelScript();
	~AngelScript();

	//! @brief アプリケーション全体で共有されているスクリプトエンジンを返します。
	//! @return アプリケーション全体で共有されているスクリプトエンジン。
	asIScriptEngine* GetEngine() const { return engine; }

	//! @brief そのクラス(型)が他の型を実装しているかを確認します。
	//! @param[in] type nameで示される型を実装しているかを確認したい型の情報。
	//! @param[in] name 実装元となる型の名前。
	//! @return typeで示される型がnameで示される型を実装していればtrue。
	bool CheckImplementation(asITypeInfo* type, const char* name) const
	{
		return type->Implements(engine->GetTypeInfoByName(name));
	}

private:
	//! @brief 型情報からインスタンスを生成します。
	//! @param[in] type インスタンスを生成する型の情報。
	//! @return typeで示される型のインスタンス。
	asIScriptObject* InstantiateObject(asITypeInfo* type) const;

	//! @brief ファイル名をもとにスクリプトモジュールを取得します。
	//! @param[in] root スクリプトファイルの属するディレクトリ。
	//! @param[in] file スクリプトファイル名。
	//! @param[in] forceReload この引数がtrueの場合、スクリプトモジュールを強制的に再生成します。
	//! @return 取得したスクリプトモジュール。処理に失敗した場合、nullptrを返します。
	//! @note 内部でmoduleをfileで指定した名前をキーにキャッシュします。fileが重複しないよう注意してください。
	asIScriptModule* GetModule(const std::filesystem::path& root, const std::filesystem::path& file, bool forceReload);

public:
	//! @brief ファイル名をもとにスクリプトをオブジェクトとして実行します。
	//! @param[in] root スクリプトファイルの属するディレクトリ。
	//! @param[in] file スクリプトファイル名。
	//! @param[in] forceReload この引数がtrueの場合、スクリプトを強制的に再読み込みします。
	//! @return エントリポイントを持った型のインスタンス。処理に失敗した場合、nullptrを返します。
	//! @note 最初に見つけた該当する型が返るはずです(要確認)。
	asIScriptObject* AngelScript::ExecuteScriptAsObject(const std::filesystem::path& root, const std::filesystem::path& file, bool forceReload);

	//! @brief ファイル名をもとにスクリプトを関数として実行します。
	//! @param[in] root スクリプトファイルの属するディレクトリ。
	//! @param[in] file スクリプトファイル名。
	//! @param[in] forceReload この引数がtrueの場合、スクリプトを強制的に再読み込みします。
	//! @return エントリポイントを持った関数。処理に失敗した場合、nullptrを返します。
	//! @note 最初に見つけた該当する関数が返るはずです(要確認)。
	asIScriptFunction* AngelScript::ExecuteScriptAsFunction(const std::filesystem::path& root, const std::filesystem::path& file, bool forceReload);
};

/*!
 * @brief グローバル関数の保持、実行を行います。
 * @detail Createによりインスタンスを生成し、Prepare、SetArgument、Execute、Unprepareの順に実行します。
 */
class FunctionObject {
private:
	asIScriptContext* context;		//!< 関数を実行するコンテキスト
	asIScriptFunction* function;	//!< 実行する関数

public:
	//! @brief FunctionObjectのインスタンスを生成します。
	//! @param[in] function FunctionObjectで管理する関数。
	//! @return FunctionObjectのインスタンス。処理に失敗した場合、nullptrを返します。
	static FunctionObject* Create(asIScriptFunction* function);

private:
	explicit FunctionObject(asIScriptEngine* engine, asIScriptFunction* method);
public:
	~FunctionObject();

public:
	//! @brief 関数の実行コンテキストにユーザー指定データを登録します。
	//! @param[in] data 登録するユーザー指定データ。
	//! @param[in] type 登録するデータのタイプID。
	//! @return typeに今まで登録されていたデータ。
	void* SetUserData(void* data, asPWORD type) { return context->SetUserData(data, type); }

	//! @brief 関数の実行準備を行います。
	//! @return 実行結果。asSUCCESS以外なら失敗しています。
	int Prepare();

	//! @brief 関数に引数を設定します。
	//! @param[in] f 引数を設定する関数。引数の設定に成功したらtrueを返すような関数を渡してください。ラムダ式を書くと便利です。
	//! @return 全ての引数の設定に成功した場合true。
	//! @todo パフォーマンス的に心許ないので何か適切な代替手段を考えたい
	bool SetArgument(std::function<int(asIScriptContext*)> f);

	//! @brief 関数を実行します。
	//! @return 実行結果。asEXECUTION_FINISHED以外ならおそらく失敗しています。
	//! @note 実行結果が意図しないものだった場合は関数内部でログを吐きます。
	int Execute();

	//! @brief 関数実行を終了します。
	//! @return 実行結果。asSUCCESS以外ならたぶん失敗しています。
	int Unprepare() { return context->Unprepare(); }
};

/*!
 * @brief メンバ関数の保持、実行を行います。
 * @detail Createによりインスタンスを生成し、Prepare、SetArgument、Execute/ExecuteAsSuspendable、Unprepareの順に実行します。
 */
class MethodObject {
private:
	asIScriptContext* context;		//!< メンバ関数を実行するコンテキスト
	asIScriptObject* object;		//!< メンバ関数をもつオブジェクトのインスタンス
	asIScriptFunction* function;	//!< 実行するメンバ関数

public:
	//! @brief MethodObjectのインスタンスを生成します。
	//! @param[in] obj MethodObjectで管理するメンバ関数をもつオブジェクトのインスタンス。
	//! @param[in] decl MethodObjectで管理するメンバ関数の定義文字列。
	//! @return MethodObjectのインスタンス。処理に失敗した場合、nullptrを返します。
	static MethodObject* Create(asIScriptObject* obj, const char* decl);

private:
	explicit MethodObject(asIScriptEngine* engine, asIScriptObject* object, asIScriptFunction* method);
public:
	~MethodObject();

public:
	//! @brief 関数の実行コンテキストにユーザー指定データを登録します。
	//! @param[in] data 登録するユーザー指定データ。
	//! @param[in] type 登録するデータのタイプID。
	//! @return typeに今まで登録されていたデータ。
	void* SetUserData(void* data, asPWORD type) { return context->SetUserData(data, type); }

	//! @brief 関数の実行準備を行います。
	//! @return 実行結果。asSUCCESS以外なら失敗しています。
	int Prepare();

	//! @brief 関数に引数を設定します。
	//! @param[in] f 引数を設定する関数。引数の設定に成功したらtrueを返すような関数を渡してください。ラムダ式を書くと便利です。
	//! @return 全ての引数の設定に成功した場合true。
	//! @todo パフォーマンス的に心許ないので何か適切な代替手段を考えたい
	bool SetArgument(std::function<int(asIScriptContext*)> f);

	//! @brief 関数を実行します。
	//! @return 実行結果。asEXECUTION_FINISHED以外ならおそらく失敗しています。
	//! @note 実行結果が意図しないものだった場合は関数内部でログを吐きます。
	int Execute();

	//! @brief 関数を中断可能形式で実行します。
	//! @return 実行結果。asEXECUTION_FINISHED/asEXECUTION_SUSPENDED以外ならおそらく失敗しています。
	//! @note 実行結果が意図しないものだった場合は関数内部でログを吐きます。
	int ExecuteAsSuspendable();

	//! @brief 関数実行を終了します。
	//! @return 実行結果。asSUCCESS以外ならたぶん失敗しています。
	int Unprepare() { return context->Unprepare(); }
};

/*!
 * @brief コールバック関数の保持、実行を行います。
 * @detail Createによりインスタンスを生成し、Prepare、SetArgument、Execute、Unprepareの順に実行します。
 * 保持するコールバックはデリゲートになるため、循環参照を発生させる場合があります。
 * これを回避するために、このオブジェクトを介してコールバックを実行するスレッドで適切なタイミングで
 * Dispose、Releaseを呼び出すことでデリゲートを解放し、循環参照状態を解消することが望まれます。
 * Disposeを呼び出すとPrepareメソッド等は呼び出し禁止状態になります。
 * オブジェクトが呼び出し禁止状態になっているときはIsExistsがfalseを返します。
 */
class CallbackObject {
private:
	asIScriptContext* context;		//!< コールバックを実行するコンテキスト
	asIScriptObject* object;		//!< コールバックをもつオブジェクトのインスタンス
	asIScriptFunction* function;	//!< 実行するコールバック
	asITypeInfo* type;				//!< コールバックを持つオブジェクトの型情報
	bool exists;					//!< コールバックのデリゲートが存在しているか
	int refcount;					//!< 参照カウント

public:
	//! @brief CallbackObjectのインスタンスを生成します。
	//! @param[in] obj CallbackObjectで管理するコールバック関数。
	//! @return CallbackObjectのインスタンス。処理に失敗した場合、nullptrを返します。
	static CallbackObject* Create(asIScriptFunction* callback);

private:
	explicit CallbackObject(asIScriptEngine* engine, asIScriptFunction* callback);
public:
	~CallbackObject();

	//! @brief 参照カウントを増やします。
	void AddRef() { ++refcount; }

	//! @brief 参照カウントを減らします。
	void Release() { if (--refcount == 0) delete this; }

	//! @brief 参照カウントを返します。
	//! @return 現在の参照カウント。
	int GetRefCount() const { return refcount; }

	//! @brief オブジェクトが所有しているデリゲート「のみ」を解放します。
	//! @note これを呼び出すとコンテキスト等が解放されます。これの呼び出し以降はPrepare、Executeなどを呼び出さないでください。
	void Dispose();

	//! @brief オブジェクトが所有しているデリゲートが有効かどうかを返します。
	//! @return 登録したデリゲートを呼び出してよいときはtrue。
	//! @note falseを返したなら速やかにこのオブジェクトをReleaseすることが望まれる。(ただし二重開放しないよう注意。)
	bool IsExists() const { return exists; }

	//! @brief 関数の実行コンテキストにユーザー指定データを登録します。
	//! @param[in] data 登録するユーザー指定データ。
	//! @param[in] type 登録するデータのタイプID。
	//! @return typeに今まで登録されていたデータ。
	void* SetUserData(void* data, asPWORD type)
	{
		SU_ASSERT(IsExists());
		return context->SetUserData(data, type);
	}

	//! @brief 関数の実行準備を行います。
	//! @return 実行結果。asSUCCESS以外なら失敗しています。
	int Prepare();

	//! @brief 関数に引数を設定します。
	//! @param[in] f 引数を設定する関数。引数の設定に成功したらtrueを返すような関数を渡してください。ラムダ式を書くと便利です。
	//! @return 全ての引数の設定に成功した場合true。
	//! @todo パフォーマンス的に心許ないので何か適切な代替手段を考えたい
	bool SetArgument(std::function<int(asIScriptContext*)> f);

	//! @brief 関数を実行します。
	//! @return 実行結果。asEXECUTION_FINISHED以外ならおそらく失敗しています。
	//! @note 実行結果が意図しないものだった場合は関数内部でログを吐きます。
	int Execute();

	//! @brief 関数実行を終了します。
	//! @return 実行結果。asSUCCESS以外ならたぶん失敗しています。
	int Unprepare()
	{
		SU_ASSERT(IsExists());
		return context->Unprepare();
	}
};
