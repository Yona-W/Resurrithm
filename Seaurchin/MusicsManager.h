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
	void CreateMusicCache();

public:
	MusicSelectionState ReloadMusic(const bool async);
	bool IsReloading() const;
	MusicSelectionState ResetState();

private:
	std::shared_ptr<CategoryInfo> GetCategoryAt(const int32_t relative) const;
	std::shared_ptr<MusicMetaInfo> GetMusicAt(const int32_t relative) const;
	std::shared_ptr<MusicScoreInfo> GetScoreVariantAt(const int32_t relative) const;
public:
	std::filesystem::path GetSelectedScorePath() const;
	std::string GetPrimaryString(int32_t relativeIndex) const;
	std::string GetCategoryName(int32_t relativeIndex) const;
	std::string GetMusicName(int32_t relativeIndex) const;
	std::string GetArtistName(int32_t relativeIndex) const;
	std::string GetMusicJacketFileName(int32_t relativeIndex) const;
	std::string GetBackgroundFileName(int32_t relativeIndex) const;
	int GetDifficulty(int32_t relativeIndex) const;
	int GetLevel(int32_t relativeIndex) const;
	double GetBpm(int32_t relativeIndex) const;
	std::string GetExtraLevel(int32_t relativeIndex) const;
	std::string GetDesignerName(int32_t relativeIndex) const;

	MusicSelectionState Enter();
	MusicSelectionState Exit();
	MusicSelectionState Next();
	MusicSelectionState Previous();
	MusicSelectionState NextVariant();
	MusicSelectionState PreviousVariant();
	MusicSelectionState GetState() const;

	int32_t GetCategorySize() const;
	int32_t GetMusicSize(int32_t relativeIndex) const;
	int32_t GetVariantSize(int32_t relativeIndex) const;

	static void RegisterScriptInterface(asIScriptEngine* engine);
};


struct MusicRawData final {
	std::string SongId;
	std::string Name;
	std::string Artist;
	std::vector<std::tuple<std::string, std::string, std::string, std::string>> Scores;
};
