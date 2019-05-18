/*!
 * @file MusicsManager.h
 * @brief SUSファイルの管理を行うクラス MusicsManager と関連クラスの宣言
 * @author kb10uy
 * @date 2019/04/28
 * @details SUSファイルの列挙、ロードを行うインターフェースを提供します。
 */

#pragma once

#define SU_IF_MUSIC_MANAGER "MusicManager"

class SusAnalyzer;
class CategoryParameter;

/*!
 * @brief 楽曲と譜面の管理を行います
 * @details 内部的に譜面一覧を[カテゴリ]/[楽曲]/[譜面差分]の階層構造で保持します。
 * 譜面一覧の生成(ReloadMusic)を行ってから、Next,Prevで項目を選択し、
 * Enterで決定していくことで譜面を選択していきます。
 */
class MusicsManager final {
private:
	bool loading;								//!< 譜面一覧更新中か否か (排他制御してください)
	mutable std::mutex flagMutex;				//!< 譜面一覧更新中か否かのフラグの排他制御に用いるミューテックス
	std::thread loadWorker;						//!< 譜面一覧更新スレッド
	std::unique_ptr<SusAnalyzer> analyzer;		//!< SUSファイルアナライザ
	std::vector<CategoryParameter*> categories;	//!< 譜面一覧

public:
	explicit MusicsManager();
	~MusicsManager();

private:
	//! @brief 譜面一覧を再読み込みします。
	//! @details 現在の譜面一覧を破棄し、該当ディレクトリから譜面を列挙し、所有するアナライザで譜面を解析します。
	//! かなりの時間がかかる処理になるので、外部からはこれを直接実行せず、
	//! loadingフラグを用いて適切に非同期処理することが推奨されます。
	void CreateMusicCache();

public:
	//! @brief 譜面一覧を再読み込みします。
	//! @param[in] async 非同期で読み込みたい場合はtrueを渡してください。
	//! @return 関数の実行に成功するとtrueが返ります。譜面一覧再読み込み中に実行するとfalseを返します。
	bool Reload(const bool async);

	//! @brief 譜面一覧の再読み込み中かどうかを確認します。
	//! @return 譜面一覧の再読み込み中ならtrueを返します。
	bool IsReloading() const;

public:
	//! @brief インデックスを指定してカテゴリ情報を取得します。
	//! @param[in] index カテゴリ一覧内でのインデックス。
	//! @return 該当位置にあるカテゴリ情報を返します。カテゴリ情報取得に失敗するとnullを返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	CategoryParameter* GetCategory(uint32_t index) const;

	//! @brief 総カテゴリ数を返します。
	//! @return 総カテゴリ数。
	//! @details 再読み込み中は0が返ります。
	uint32_t GetCategorySize() const { return IsReloading() ? 0 : categories.size(); }

public:
	//! @brief ミュージックカーソルクラスをスキンに登録します。
	//! @param[in] engine スクリプトエンジン。
	static void RegisterType(asIScriptEngine* engine);
};
