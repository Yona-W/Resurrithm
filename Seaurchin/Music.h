/*!
 * @file Music.h
 * @brief 譜面や楽曲に関する情報を保持するクラス ScoreParameter, MusicParameter, CategoryParameter の宣言
 * @author amenoshita-429
 * @date 2019/06/02
 * @details 譜面情報や楽曲情報、カテゴリ情報をまとめるクラスを提供します。
 */

#pragma once

#define SU_IF_CATEGORY "Category"
#define SU_IF_MUSIC "Music"
#define SU_IF_SCORE "Score"
#define SU_IF_SCORE_DIFFICULTY "Difficulty"

class SusAnalyzer;

namespace ScoreDifficulty {
	//! @brief 譜面の難易度を表します。
	enum class Enum {
		Basic = 0,	//!< BASIC
		Advanced,	//!< ADVANCED
		Expert,		//!< EXPERT
		Master,		//!< MASTER
		WorldsEnd	//!< World's End
	};

	//! @brief 譜面難易度列挙子をスクリプトエンジンに登録します。
	//! @param[in] engine スクリプトエンジン。
	//! @return 登録に失敗するとfalseを返します。
	bool RegisterTypes(asIScriptEngine* engine);
}


/*!
 * @brief 譜面情報を保持します。
 * @note 手動参照カウント実装クラスです。
 */
class ScoreParameter final {
	INPLEMENT_REF_COUNTER

public:
	std::string SongId;		//!< 楽曲識別ID
	std::string ScorePath;	//!< 譜面の実体となるSUSファイルへのパス (絶対パスを想定しています)

	std::string Title;					//!< 楽曲名
	std::string Artist;					//!< 作曲者名
	std::string Designer;				//!< 譜面製作者名
	ScoreDifficulty::Enum Difficulty;	//!< 難易度情報
	uint32_t Level;						//!< 譜面レベル(整数部のみ) / WEにおける「☆」の数
	std::string DifficultyName;			//!< 譜面レベルにおける「+」 / WEにおける難易度文字
	double Bpm = 0.0;					//!< 表示BPM

	std::string JacketPath;		//!< ジャケット画像ファイルへのパス (絶対パスを想定しています)
	std::string WavePath;		//!< 譜面が用いる音楽ファイルへのパス (絶対パスを想定しています)
	double WaveOffset;			//!< 譜面が用いる音楽ファイルのオフセット(秒単位)
	std::string BackgroundPath;	//!< 背景に表示する画像ファイルへのパス (絶対パスを想定しています)
	std::string MoviePath;		//!< 背景で再生する動画ファイルへのパス (絶対パスを想定しています)
	double MovieOffset;			//!< 背景で再生する動画のオフセット(秒単位)

private:
	ScoreParameter();
	ScoreParameter(const ScoreParameter&) = delete;
	~ScoreParameter();

public:
	//! @brief 譜面情報を生成します。
	//! @param[in] analyzer susファイルを解析するアナライザ。
	//! @param[in] cpath 解析対象となるsusファイルへの絶対パス。
	//! @return 生成した譜面情報の生ポインタ。譜面情報の生成に失敗するとnullptrを返します。
	static ScoreParameter* Create(SusAnalyzer* analyzer, const std::filesystem::path& cpath);

	//! @brief スクリプトエンジンに譜面情報クラスを登録します。
	//! @param[in] engine スクリプトエンジン。
	//! @return 登録に失敗するとfalseを返します。
	static bool RegisterTypes(asIScriptEngine* engine);
};


/*!
 * @brief 楽曲情報を保持します。
 * @note 手動参照カウント実装クラスです。
 */
class MusicParameter final {
	INPLEMENT_REF_COUNTER

public:
	static constexpr size_t MaxItemCount = 1000000;	//!< 保持できる最大譜面数

private:
	std::string songId;						//!< 楽曲識別ID
	std::vector<ScoreParameter*> scores;	//!< 譜面情報の配列

private:
	MusicParameter();
	MusicParameter(const MusicParameter&) = delete;
	~MusicParameter();

public:
	//! @brief 譜面情報を登録します。
	//! @param[in] pScore 登録する譜面情報。
	//! @return 譜面情報の登録に失敗した場合、falseを返します。
	bool AddScore(ScoreParameter* pScore);

public:
	//! @brief 楽曲識別IDを取得します。
	//! @return 楽曲識別ID。
	std::string GetSongId() const { return songId; }

	//! @brief 楽曲識別IDが一致するか検証します。
	//! @param[in] songId 検証したい楽曲識別ID。
	//! @return 楽曲識別IDが一致すればtrueを返します。
	bool IsSongId(const std::string& songId) { return this->songId == songId; }

	//! @brief 保持している譜面情報数を取得します。
	//! @return 保持している譜面情報数。
	uint32_t GetScoreCount() const { return SU_TO_UINT32(scores.size()); }

	//! @brief インデックスを指定して譜面情報を取得します。
	//! @param[in] index 取得する譜面情報の楽曲情報内でのインデックス。
	//! @return 取得した譜面情報の生ポインタ。譜面情報の取得に失敗するとnullptrを返します。
	ScoreParameter* GetScore(uint32_t index) const;

public:
	//! @brief 楽曲情報を生成します。
	//! @param[in] songID 生成する楽曲情報の楽曲識別ID。
	//! @return 生成した楽曲情報の生ポインタ。楽曲情報の生成に失敗するとnullptrを返します。
	static MusicParameter* Create(const std::string& songID);

	//! @brief スクリプトエンジンに楽曲情報クラスを登録します。
	//! @param[in] engine スクリプトエンジン。
	//! @return 登録に失敗するとfalseを返します。
	static bool RegisterTypes(asIScriptEngine* engine);
};


/*!
 * @brief 楽曲のカテゴリ情報を保持します。
 * @note 手動参照カウント実装クラスです。
 */
class CategoryParameter final {
	INPLEMENT_REF_COUNTER

public:
	static constexpr size_t MaxItemCount = 1000000;	//!< 保持できる最大楽曲数

private:
	std::string name;					//!< カテゴリ名
	std::vector<MusicParameter*> music;	//!< 楽曲情報の配列

private:
	CategoryParameter();
	CategoryParameter(const CategoryParameter&) = delete;
	~CategoryParameter();

private:
	//! @brief 楽曲情報を登録します。
	//! @param[in] pMusic 登録する楽曲情報。
	//! @return 楽曲情報の登録に失敗した場合、falseを返します。
	bool AddMusic(MusicParameter* pMusic);

public:
	//! @brief カテゴリ名を取得します。
	//! @return カテゴリ名。
	std::string GetName() const { return name; }

	//! @brief 保持している楽曲情報数を取得します。
	//! @return 保持している楽曲情報数。
	uint32_t GetMusicCount() const { return SU_TO_UINT32(music.size()); }

	//! @brief インデックスを指定して楽曲情報を取得します。
	//! @param[in] index 取得する楽曲情報のカテゴリ内でのインデックス。
	//! @return 取得した楽曲情報の生ポインタ。楽曲情報の取得に失敗するとnullptrを返します。
	MusicParameter* GetMusic(uint32_t index) const;

	//! @brief 楽曲識別IDを指定して楽曲情報を取得します。
	//! @param[in] songId 取得する楽曲情報の楽曲識別ID。
	//! @return 取得した楽曲情報の生ポインタ。楽曲情報の取得に失敗するとnullptrを返します。
	MusicParameter* GetMusic(const std::string& songId) const;

public:
	//! @brief 譜面情報を生成します。
	//! @param[in] analyzer susファイルを解析するアナライザ。
	//! @param[in] cpath 解析対象となるsusファイルへの絶対パス。
	//! @return 生成した譜面情報の生ポインタ。譜面情報の生成に失敗するとnullptrを返します。
	static CategoryParameter* Create(SusAnalyzer* analyzer, const std::filesystem::path& cpath);

	//! @brief スクリプトエンジンにカテゴリ情報クラスを登録します。
	//! @param[in] engine スクリプトエンジン。
	//! @return 登録に失敗するとfalseを返します。
	static bool RegisterTypes(asIScriptEngine* engine);
};

