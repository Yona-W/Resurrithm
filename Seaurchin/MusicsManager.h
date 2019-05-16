/*!
 * @file MusicsManager.h
 * @brief SUSファイルの管理を行うクラス MusicsManager と関連クラスの宣言
 * @author kb10uy
 * @date 2019/04/28
 * @details SUSファイルの列挙、ロードを行うインターフェースを提供します。
 */

#pragma once

class SusAnalyzer;

/*!
 * @def SU_IF_MSCURSOR
 * @brief スキンで用いるミュージックカーソルクラスのクラス名
 */
/*!
 * @def SU_IF_MSCSTATE
 * @brief スキンで用いるミュージックカーソルクラスの取りうる状態値の列挙名
 */
#define SU_IF_MSCURSOR "MusicCursor"
#define SU_IF_MSCSTATE "CursorState"

/*!
 * @brief 譜面情報
 * @details 主にスキン側から参照したい譜面に関する情報を持ちます。
 */
struct MusicScoreInfo final {
	uint16_t Difficulty = 0;				//!< 難易度情報 (0:BASIC 1:ADVANCED 2:EXPERT 3:MASTER 4:WORLD'S END)
	uint16_t Level = 0;						//!< 譜面レベル(整数部のみ) / WEにおける「☆」の数
	double BpmToShow = 0.0;					//!< 表示BPM
	std::string DifficultyName;				//!< 譜面レベルにおける「+」 / WEにおける難易度文字
	std::string Designer;					//!< 譜面製作者名
	std::filesystem::path Path;				//!< 譜面の実体となるSUSファイルへのパス (所属カテゴリからの相対パスを想定しています)
	std::filesystem::path WavePath;			//!< 譜面が用いる音楽ファイルへのパス (SUSファイルからの相対パスを想定しています)
	std::filesystem::path BackgroundPath;	//!< 背景で再生する動画ファイルへのパス (SUSファイルからの相対パスを想定しています)
};

/*!
 * @brief 楽曲情報
 * @details 同一楽曲の差分譜面をまとめるクラスです。
 */
struct MusicMetaInfo final {
	std::string SongId;										//!< 楽曲識別ID
	std::string Name;										//!< 楽曲名
	std::string Artist;										//!< 作曲者名
	std::filesystem::path JacketPath;						//!< ジャケット画像ファイルへのパス (所属カテゴリからの相対パスを想定しています)
	std::vector<std::shared_ptr<MusicScoreInfo>> Scores;	//!< 譜面情報の配列
};

/*!
 * @brief カテゴリ情報
 * @details 同一カテゴリの楽曲をまとめるクラスです。
 */
class CategoryInfo final {
private:
	std::string name;					//!< カテゴリ名
	std::filesystem::path categoryPath;	//!< 実体となるディレクトリへのパス (絶対パスを想定しています)

public:
	std::vector<std::shared_ptr<MusicMetaInfo>> Musics;	//!< 楽曲情報の配列

	//! @param[in] cpath カテゴリの実体に当たるディレクトリへのパス。絶対パスを想定しています。
	explicit CategoryInfo(const std::filesystem::path& cpath);
	~CategoryInfo();

	std::string GetName() const { return name; }
	void Reload(bool recreateCache) const;
};

/*!
 * @brief 楽曲選択状態
 * @details 現在の MusicManager の内部状態を示します。
 */
enum class MusicSelectionState {
	OutOfFunction = 0,	//!< 無効な遷移先
	Category,			//!< カテゴリ選択中
	Music,				//!< 楽曲選択中
	Confirmed,			//!< 譜面選択完了

	Reloading,			//!< 譜面一覧再読み込み中
	Error,				//!< 関数の実行に失敗
	Success,			//!< 関数の実行に成功
};

/*!
 * @brief 楽曲と譜面の管理を行います
 * @details 内部的に譜面一覧を[カテゴリ]/[楽曲]/[譜面差分]の階層構造で保持します。
 * 譜面一覧の生成(ReloadMusic)を行ってから、Next,Prevで項目を選択し、
 * Enterで決定していくことで譜面を選択していきます。
 */
class MusicsManager final {
	typedef std::vector<std::shared_ptr<CategoryInfo>> MusicList;

private:
	bool loading;							//!< 譜面一覧更新中か否か (排他制御してください)
	mutable std::mutex flagMutex;			//!< 譜面一覧更新中か否かのフラグの排他制御に用いるミューテックス
	std::thread loadWorker;					//!< 譜面一覧更新スレッド
	std::unique_ptr<SusAnalyzer> analyzer;	//!< SUSファイルアナライザ
	MusicList categories;					//!< 譜面一覧

	int32_t categoryIndex;		//!< 選択中のカテゴリインデックス
	int32_t musicIndex;			//!< 選択中の楽曲インデックス
	uint16_t variantIndex;		//!< 選択中の譜面インデックス
	MusicSelectionState state;	//!< 現在どの項目を選択中かを表す

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
	//! @return 関数の実行に成功するとSuccessが返ります。譜面一覧再読み込み中に実行するとReloadingを返します。
	//! @details エクステンションの列挙、初期化、登録を行うインターフェースを提供します。
	MusicSelectionState ReloadMusic(const bool async);

	//! @brief 譜面一覧の再読み込み中かどうかを確認します。
	//! @return 譜面一覧の再読み込み中ならtrueを返します。
	bool IsReloading() const;

	//! @brief 譜面選択状態をリセットします。
	//! @return 関数の実行に成功するとSuccessが返ります。譜面一覧再読み込み中に実行するとReloadingを返します。
	//! @details 関数の実行に成功すると、カテゴリ選択状態で0番目のカテゴリを選択中の状態になります。
	MusicSelectionState ResetState();

private:
	//! @brief 現在選択中のカテゴリから相対位置を指定してカテゴリ情報を取得します。
	//! @param[in] relative 現在選択中のカテゴリに対する相対カテゴリ数。
	//! @return 該当位置にあるカテゴリ情報を返します。カテゴリ情報取得に失敗するとnullを返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::shared_ptr<CategoryInfo> GetCategoryAt(const int32_t relative) const;

	//! @brief 現在選択中の楽曲から相対位置を指定して楽曲情報を取得します。
	//! @param[in] relative 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にある楽曲情報を返します。楽曲情報取得に失敗するとnullを返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::shared_ptr<MusicMetaInfo> GetMusicAt(const int32_t relative) const;

	//! @brief 現在選択中の譜面から相対位置を指定して譜面情報を取得します。
	//! @param[in] relative 現在選択中の譜面に対する相対譜面差分数。
	//! @return 該当位置にある譜面情報を返します。譜面情報取得に失敗するとnullを返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::shared_ptr<MusicScoreInfo> GetScoreVariantAt(const int32_t relative) const;

public:
	//! @brief 現在選択中の譜面の実体となるSUSファイルのパスを返します。
	//! @return 現在選択中の譜面の実体となるSUSファイルへの絶対パス。パスの取得に失敗した場合、空のパスが返ります。
	//! @details 特にリロード中に実行するとパスの取得に失敗します。
	std::filesystem::path GetSelectedScorePath() const;

	//! @brief 現在選択中の譜面の実体となるSUSファイルのパスをUTF-8文字列で返します。
	//! @return 現在選択中の譜面の実体となるSUSファイルへの絶対パスをUTF-8へ変換した文字列。パスの取得に失敗した場合、空文字列が返ります。
	//! @details 特にリロード中に実行するとパスの取得に失敗します。
	std::string GetSelectedScorePathString() const;

	//! @brief 現在の選択状態で取得可能な文字列情報を相対位置を指定して取得します。
	//! @param[in] relativeIndex 現在選択中の項目に対する相対項目数。
	//! @return カテゴリ選択中ならカテゴリ名を、楽曲選択中なら楽曲名を返します。情報の取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::string GetPrimaryString(int32_t relativeIndex) const;

	//! @brief 現在選択中のカテゴリから相対位置を指定してカテゴリ名を取得します。
	//! @param[in] relativeIndex 現在選択中のカテゴリに対する相対カテゴリ数。
	//! @return 該当位置にあるカテゴリ名を返します。カテゴリ名の取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::string GetCategoryName(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定して楽曲名を取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にある楽曲名を返します。楽曲名の取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::string GetMusicName(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定してアーティスト名を取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にあるアーティスト名を返します。アーティスト名の取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::string GetArtistName(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定してジャケットファイルパスを取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にあるジャケットファイルパスを返します。ジャケットファイルパスの取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::string GetMusicJacketFileName(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定して背景動画ファイルパスを取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にある背景動画ファイルパスを返します。絶対パスです。背景動画ファイルパスの取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::string GetBackgroundFileName(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定して難易度を取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にある難易度を返します。難易度の取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	int GetDifficulty(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定して譜面レベルを取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にある譜面レベルを返します。譜面レベルの取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	int GetLevel(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定してBPMを取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にあるBPMを返します。BPMの取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	double GetBpm(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定してレベル追加情報を取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にあるレベル追加情報を返します。レベル追加情報の取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::string GetExtraLevel(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置を指定して譜面製作者名を取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当位置にある譜面製作者名を返します。譜面製作者名の取得に失敗すると"Unavailable!"を返します。
	//! @details 特にリロード中に実行すると情報取得に失敗します。
	std::string GetDesignerName(int32_t relativeIndex) const;


	//! @brief 現在選択中の項目を「確定」します。
	//! @return 関数の実行に成功するとSuccess/Confirmedを返します。
	//! 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
	//! @details 譜面選択操作のすべてが完了した際にはConfirmedが返ります。
	//! 「確定」できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
	MusicSelectionState Enter();

	//! @brief 現在選択中の項目を「取り消し」ます。
	//! @return 関数の実行に成功するとSuccessを返します。
	//! 関数の実行に失敗するとReloading/OutOfFunctionが返ります。
	//! @details 「取り消し」できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
	MusicSelectionState Exit();

	//! @brief 現在選択中の項目に対して「次」の項目へ移ります。
	//! @return 関数の実行に成功するとSuccessを返します。
	//! 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
	//! @details 「次」の項目を選択できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
	MusicSelectionState Next();

	//! @brief 現在選択中の項目に対して「前」の項目へ移ります。
	//! @return 関数の実行に成功するとSuccessを返します。
	//! 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
	//! @details 「前」の項目を選択できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
	MusicSelectionState Previous();

	//! @brief 現在選択中の項目に対して「次の差分」へ移ります。
	//! @return 関数の実行に成功するとSuccessを返します。
	//! 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
	//! @details 「次の差分」を選択できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
	MusicSelectionState NextVariant();

	//! @brief 現在選択中の項目に対して「前の差分」へ移ります。
	//! @return 関数の実行に成功するとSuccessを返します。
	//! 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
	//! @details 「前の差分」を選択できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
	MusicSelectionState PreviousVariant();

	//! @brief 現在の選択状態を返します。
	//! @return 現在の選択状態。
	MusicSelectionState GetState() const;

	//! @brief 総カテゴリ数を返します。
	//! @return 総カテゴリ数。
	//! @details 再読み込み中は負数が返ります。
	int32_t GetCategorySize() const;

	//! @brief 現在選択中のカテゴリから相対位置をしてカテゴリ内の楽曲数を取得します。
	//! @param[in] relativeIndex 現在選択中のカテゴリに対する相対カテゴリ数。
	//! @return 該当するカテゴリ内の楽曲数。
	//! @details 再読み込み中は負数が返ります。
	int32_t GetMusicSize(int32_t relativeIndex) const;

	//! @brief 現在選択中の楽曲から相対位置をして楽曲内の譜面差分数を取得します。
	//! @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
	//! @return 該当する楽曲内の差分譜面数。
	//! @details 再読み込み中は負数が返ります。
	int32_t GetVariantSize(int32_t relativeIndex) const;

	//! @brief ミュージックカーソルクラスをスキンに登録します。
	//! @param[in] engine スクリプトエンジン。
	static void RegisterType(asIScriptEngine* engine);
};
