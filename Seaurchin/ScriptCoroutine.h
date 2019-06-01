/*!
 * @file ScriptCoroutine.h
 * @brief スキンで用いるコルーチン関係のクラスの宣言
 * @author kb10uy
 * @date 2019/05/30
 * @details コルーチン処理を行うためのインターフェースを提供します。
 */

#pragma once

#define SU_IF_COROUTINE "Coroutine"

/*!
 * @brief コルーチン処理のための時刻管理を行います。
 * @details 実行中の処理をsuspendする際にWaitFrmaeまたはWaitTimeを呼び出すことで、どれだけの間実行を中断するかを指定します。
 * 実際に処理を実行するクラスでは毎フレームごとにTickを呼び出し、戻り値がtrueの間は処理を中断し続けて(実行しないで)ください。
 */
class CoroutineWait {
private:
	//! @brief 待機方法
	enum class WaitType {
		Frame,	//!< フレーム数指定で待機
		Time	//!< 時間指定で待機
	};

private:
	WaitType type;		//!< 待機方法
	union {
		double time;	//!< 待機時間
		uint64_t frames;	//!< 待機フレーム数
	};

public:
	explicit CoroutineWait()
		: type(WaitType::Frame)
		, frames(0)
	{};
	//! @breif 待機フレーム数を指定して初期化
	//! @param[in] frames 待機フレーム数
	explicit CoroutineWait(const uint64_t frames)
		: type(WaitType::Frame)
		, frames(frames)
	{};
	//! @breif 待機時間を指定して初期化
	//! @param[in] tiime 待機時間
	explicit CoroutineWait(const double tiime)
		: type(WaitType::Frame)
		, time(time)
	{};

	//! @breif 待機フレーム数を設定します。
	//! @param[in] frames 待機フレーム数
	void WaitFrame(const uint64_t frames)
	{
		type = WaitType::Frame;
		this->frames = frames;
	}
	//! @breif 待機時間を設定します。
	//! @param[in] tiime 待機時間
	//! @note 非正数値を与えた場合は無効です。(パラメータの更新を行わない)
	void WaitTime(const double time)
	{
		if (!(time > 0.0)) return;

		type = WaitType::Time;
		this->time = time;
	}

	//! @breif 待機フレーム数/待機時間を更新し、実行を中断する必要があるかを返します。
	//! @param[in] delta 前フレームからの経過時間
	//! @return 実行を中断する必要があればtrueを返します。
	bool Tick(const double delta)
	{
		switch (type) {
		case WaitType::Frame:
			if (frames > 0) --frames;
			return frames > 0;
		case WaitType::Time:
			if (time > 0.0) time -= delta;
			return time > 0.0;
		}
		return false;
	}
};


/*!
 * @brief コルーチンの保持、実行を行います。
 * @details 生成したインスタンスに対し毎フレームごとにTickを実行することで適切にコルーチン処理が行われます。
 * Tickの戻り値がasEXECUTION_SUSPENDEDの間はコルーチンが実行状態にあります。
 * 逆にTickの戻り値がasEXECUTION_SUSPENDED以外であれば終了した、またはエラーが発生しています。
 */
class Coroutine {
private:
	asIScriptContext* context;		//!< コルーチンを実行するコンテキスト
	asIScriptObject* object;		//!< コルーチンをもつオブジェクトのインスタンス
	asIScriptFunction* function;	//!< 実行するコルーチン
	CoroutineWait wait;				//!< 待機情報

public:
	std::string Name;	//!< コルーチンの識別名

public:
	//! @param[in] name 識別名
	//! @param[in] cofunc 実行するコルーチン
	Coroutine(const std::string& name, const asIScriptFunction* cofunc);
	Coroutine(const Coroutine&) = delete;
	~Coroutine();

	//! @brief 関数の実行コンテキストにユーザー指定データを登録します。
	//! @param[in] data 登録するユーザー指定データ。
	//! @param[in] type 登録するデータのタイプID。
	//! @return typeに今まで登録されていたデータ。
	void* SetUserData(void* data, asPWORD type) { return context->SetUserData(data, type); }

	//! @breif 待機フレーム数/待機時間を更新し、必要があれば関数を実行します。
	//! @param[in] delta 前フレームからの経過時間
	//! @return 実行結果。asEXECUTION_FINISHED以外ならおそらく失敗しています。
	//! @note 実行結果が意図しないものだった場合は関数内部でログを吐きます。
	int Tick(double delta);
};
