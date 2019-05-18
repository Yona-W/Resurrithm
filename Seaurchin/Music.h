#pragma once

#ifdef _DEBUG 
#define INPLEMENT_REF_COUNTER \
private: \
	int32_t refcount; \
public: \
	void AddRef() { ++refcount; } \
	void Release() { if (--refcount == 0) delete this; } \
	int32_t GetRefcount() const { return refcount; }
#else
#define INPLEMENT_REF_COUNTER \
private: \
	int32_t refcount; \
public: \
	void AddRef() { ++refcount; } \
	void Release() { if (--refcount == 0) delete this; }
#endif

#define SU_IF_CATEGORY "Category"
#define SU_IF_MUSIC "Music"
#define SU_IF_SCORE "Score"
#define SU_IF_SCORE_DIFFICULTY "Difficulty"

class SusAnalyzer;

namespace ScoreDifficulty {
	enum class Enum {
		Basic = 0,
		Advanced,
		Expert,
		Master,
		WorldsEnd
	};

	bool RegisterTypes(asIScriptEngine* engine);
}


class ScoreParameter final {
	INPLEMENT_REF_COUNTER

public:
	std::string SongId;		//!< 楽曲識別ID
	std::string ScorePath;	//!< 譜面の実体となるSUSファイルへのパス (絶対パスを想定しています)

	std::string Title;					//!< 楽曲名
	std::string Artist;					//!< 作曲者名
	std::string Designer;				//!< 譜面製作者名
	ScoreDifficulty::Enum Difficulty;	//!< 難易度情報 (0:BASIC 1:ADVANCED 2:EXPERT 3:MASTER 4:WORLD'S END)
	uint32_t Level;						//!< 譜面レベル(整数部のみ) / WEにおける「☆」の数
	std::string DifficultyName;			//!< 譜面レベルにおける「+」 / WEにおける難易度文字
	double Bpm = 0.0;					//!< 表示BPM

	std::string JacketPath;		//!< ジャケット画像ファイルへのパス (絶対パスを想定しています)
	std::string WavePath;		//!< 譜面が用いる音楽ファイルへのパス (絶対パスを想定しています)
	double WaveOffset;			//!< 譜面が用いる音楽ファイルのオフセット(秒単位)
	std::string BackgroundPath;	//!< 背景で再生する動画ファイルへのパス (絶対パスを想定しています)
	double BackgroundOffset;	//!< 背景で再生する動画のオフセット(秒単位)

private:
	ScoreParameter();
	ScoreParameter(const ScoreParameter&) = delete;
	~ScoreParameter();

public:
	static ScoreParameter* Create(SusAnalyzer* analyzer, const std::filesystem::path& cpath);
	static bool RegisterTypes(asIScriptEngine* engine);
};


class MusicParameter;
class CategoryParameter final {
	INPLEMENT_REF_COUNTER

private:
	std::string name;					//!< カテゴリ名
	std::vector<MusicParameter*> music;	//!< 楽曲情報の配列

private:
	CategoryParameter();
	~CategoryParameter();

private:
	bool AddMusic(MusicParameter* pMusic);

public:
	std::string GetName() const { return name; }
	uint32_t GetMusicCount() const { return music.size(); }
	MusicParameter* GetMusic(uint32_t index) const;
	MusicParameter* GetMusic(const std::string& songId) const;

public:
	static CategoryParameter* Create(SusAnalyzer* analyzer, const std::filesystem::path& cpath);
	static bool RegisterTypes(asIScriptEngine* engine);
};


class MusicParameter final {
	INPLEMENT_REF_COUNTER

public:
	friend MusicParameter* CategoryParameter::GetMusic(const std::string& songId) const;

private:
	std::string songId;						//!< 楽曲識別ID
	std::vector<ScoreParameter*> scores;	//!< 譜面情報の配列

private:
	MusicParameter();
	~MusicParameter();

public:
	bool AddScore(ScoreParameter* pScore);

public:
	std::string GetSongId() const { return songId; }
	uint32_t GetScoreCount() const { return scores.size(); }
	ScoreParameter* GetScore(uint32_t index) const;

public:
	static MusicParameter* Create(const std::string& songID);
	static bool RegisterTypes(asIScriptEngine* engine);
};

