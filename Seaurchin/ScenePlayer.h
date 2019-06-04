#pragma once

#include "ScriptSprite.h"
#include "SusAnalyzer.h"
#include "Result.h"

#define SU_IF_SCENE_PLAYER "ScenePlayer"
#define SU_IF_SCENE_PLAYER_METRICS "ScenePlayerMetrics"

#define SU_LANE_X_MIN -400.0f
#define SU_LANE_X_MAX 400.0f
#define SU_LANE_X_MIN_EXT -1600.0f
#define SU_LANE_X_MAX_EXT 1600.0f
#define SU_LANE_Z_MIN_EXT -300.0f
#define SU_LANE_Z_MIN 0.0f
#define SU_LANE_Z_MAX 3000.0f
#define SU_LANE_Y_GROUND 0.0f
#define SU_LANE_Y_AIR 240.0f
#define SU_LANE_Y_AIRINDICATE 160.0f
#define SU_LANE_ASPECT ((SU_LANE_Z_MAX - SU_LANE_Z_MIN) / (SU_LANE_X_MAX - SU_LANE_X_MIN))
#define SU_LANE_ASPECT_EXT ((SU_LANE_Z_MAX - SU_LANE_Z_MIN_EXT) / (SU_LANE_X_MAX - SU_LANE_X_MIN))
#define SU_LANE_NOTE_WIDTH 192.0f
#define SU_LANE_NOTE_HEIGHT 64.0f

template<typename T, size_t N>
class BoundedQueue {
private:
	std::queue<T> queue;
	std::mutex guard;
	std::condition_variable not_empty;
	std::condition_variable not_full;
public:
	bool Push(T val) {
		std::unique_lock<std::mutex> lk(guard);
		if (queue.size() >= N) return false;

		not_full.wait(lk, [this] {
			return queue.size() < N;
			});
		queue.push(std::move(val));
		not_empty.notify_all();

		return true;
	}
	T Pop() {
		std::unique_lock<std::mutex> lk(guard);
		not_empty.wait(lk, [this] {
			return !queue.empty();
			});
		T ret = std::move(queue.front());
		queue.pop();
		not_full.notify_all();
		return ret;
	}
	void Dispose(T val) {
		// HACK: こんな処理で大丈夫なのか...?
		while (!Push(val));
	}
};

enum class JudgeType {
	ShortNormal = 0,
	ShortEx,
	SlideTap,
	Action,
};

enum class JudgeSoundType {
	Tap,
	ExTap,
	Flick,
	Air,
	AirDown,
	AirAction,
	Holding,
	HoldStep,
	HoldingStop,
	Sliding,
	SlideStep,
	SlidingStop,
	AirHolding,
	AirHoldingStop,
	Metronome,

	Disposing
};

enum class PlayingState {
	ScoreNotLoaded,
	BgmNotLoaded,
	ReadyToStart,
	Paused,
	ReadyCounting,
	BgmPreceding,
	OnlyScoreOngoing,
	BothOngoing,
	ScoreLasting,
	BgmLasting,
	Completed,
};

enum class AirDrawType {
	Air,
	AirActionStart,
	AirActionStep,
	AirActionCover,
};

struct SusDrawableNoteData;
struct AirDrawQuery {
	double Z = 0.0;
	AirDrawType Type = AirDrawType::Air;
	std::shared_ptr<SusDrawableNoteData> Note, PreviousNote;
};

struct ScenePlayerMetrics {
	double JudgeLineLeftX;
	double JudgeLineLeftY;
	double JudgeLineRightX;
	double JudgeLineRightY;
};

class ExecutionManager;
class MoverObject;
class Ability;
class SkillDetail;
class ScenePlayer : public SSprite {
	friend class ScoreProcessor;
	friend class AutoPlayerProcessor;
	friend class PlayableProcessor;

protected:
	int hGroundBuffer{};
	ExecutionManager* manager;
	BoundedQueue<JudgeSoundType, 32> judgeSoundQueue;
	std::thread judgeSoundThread;
	std::mutex asyncMutex;
	std::thread loadWorkerThread;
	const std::unique_ptr<SusAnalyzer> analyzer;
	std::multiset<SSprite*, SSprite::Comparator> sprites;
	std::vector<SSprite*> spritesPending;
	std::filesystem::path lastUseFile;

	SSound* soundBGM;

	ScoreProcessor* const processor; // processor のアドレスが不変、 processor の実体が持つ値は変わりうる

	// 状態管理変数
	bool isLoadCompleted = false;   // LoadWorkerで書き換え 譜面読み込みが完了していればtrue
	bool isReady = false;           // GetReadyで書き換え 譜面読み込み完了後、動画再生位置設定とキャラクタースキル発動完了でtrue
	bool isTerminating = false;     // Finalizeで書き換え インスタンス破棄時にtrue これをもって音声スレッドを破棄
	bool usePrioritySort = false;   // LoadWorkerで書き換え 優先度付きノーツ描画が有効ならtrue
	bool hasEnded = false;          // ProcessSoundで書き換え すべてのノーツの判定が終わっていればtrue

	// 描画関連定数
	double cameraZ = -340, cameraY = 620, cameraTargetZ = 580;  // カメラ位置 (スキンから設定可にしてるけどぶっちゃけconstで良い気がする)
	const float laneBufferX = 1024.0f;                          // 背景バッファ横幅
	const float laneBufferY = laneBufferX * SU_LANE_ASPECT;     // 背景バッファ縦幅
	const float widthPerLane = laneBufferX / 16;                // 最小ノーツ幅
	const float cullingLimit = SU_LANE_ASPECT_EXT / SU_LANE_ASPECT; // 判定線位置 (y座標)
	const float noteImageBlockX = 64.0f;                        // ショートノーツ描画単位 (幅)
	const float noteImageBlockY = 64.0f;                        // ショートノーツ描画単位 (高さ)
	const float scaleNoteY = 2.0f;                              // ショートノーツ高さ拡大率
	const float actualNoteScaleX = (widthPerLane / 2) / noteImageBlockX;   // ショートノーツ実拡大率 (幅)
	const float actualNoteScaleY = actualNoteScaleX * scaleNoteY;          // ショートノーツ実拡大率 (高さ)

	// リソースマップ SetPlayerResourceで初期化、Finalizeで破棄
	std::map<std::string, SResource*> resources;

	// 実リソース LoadResourcesでresourcesを用いて初期化する 実体はresourcesで管理するのでAddRefもReleaseもしない
	SSound * soundTap{}, *soundExTap{}, *soundFlick{}, *soundAir{}, *soundAirDown{}, *soundAirAction{}, *soundAirLoop{};
	SSound * soundHoldLoop{}, *soundSlideLoop{}, *soundHoldStep{}, *soundSlideStep{};
	SSound * soundMetronome{};
	SImage * imageLaneGround{}, *imageLaneJudgeLine{};
	SImage * imageTap{}, *imageExTap{}, *imageFlick{}, *imageHellTap{};
	SImage * imageAir{}, *imageAirUp{}, *imageAirDown{};
	SImage * imageHold{}, *imageHoldStep{}, *imageHoldStrut{};
	SImage * imageSlide{}, *imageSlideStep{}, *imageSlideStrut{};
	SImage * imageAirAction{};
	SAnimatedImage * animeTap{}, *animeExTap{}, *animeSlideTap{}, *animeSlideLoop{}, *animeAirAction{};

	// レーンスプライト
	SSprite * spriteLane{};

	// LoadResourcesで初期化 (設定ファイルから取得)
	unsigned int slideLineColor = GetColor(0, 200, 255);        // Slide中心線色
	unsigned int airActionJudgeColor = GetColor(128, 255, 160); // Air入力線色
	bool showSlideLine{}, showAirActionJudge{};                     // Slide中心線/Air入力線が有効でtrue
	double slideLineThickness{};                                  // Slide中心線太さ

	// Slide描画関係
	int segmentsPerSecond{};                  // Slide分解能
	mutable std::vector<VERTEX2D> slideVertices;    // Slide描画用頂点座標配列 関数内ローカル変数で頻繁に生成,破棄されるのを嫌って宣言
	mutable std::vector<uint16_t> slideIndices;     // Slide描画用頂点番号指定配列 同上

	const std::shared_ptr<Result> currentResult;
	DrawableResult previousStatus{}, status{};

	std::unique_ptr<Ability> ability;

	DrawableNotesList data;
	DrawableNotesList seenData, judgeData;
	std::unordered_map<std::shared_ptr<SusDrawableNoteData>, SSprite*> slideEffects;
	mutable NoteCurvesList curveData;
	double currentTime = 0;
	double currentSoundTime = 0;
	double seenDuration = 0.8;
	const double hispeedMultiplier; // = 6.0
	const double preloadingTime = 0.5;
	double backingTime = 0.0;
	double nextMetronomeTime = 0.0;
	double scoreDuration = 0.0;
	const double soundBufferingLatency; // = 0.030
	const double airRollSpeed; // = 1.5
	PlayingState state = PlayingState::ScoreNotLoaded;	// TODO: たぶんこいつも排他制御しないとぶっ壊れる
	PlayingState lastState;
	bool airActionShown = false;
	bool metronomeAvailable = true;

	void TickGraphics(double delta);
	void AddSprite(SSprite * sprite);
	void LoadResources();
	void LoadWorker(const std::filesystem::path& file);
	void RemoveSlideEffect();
	void UpdateSlideEffect();
	void CalculateNotes(double time, double duration, double preced);
	void DrawShortNotes(const std::shared_ptr<SusDrawableNoteData> & note) const;
	void DrawAirNotes(const AirDrawQuery & query) const;
	void DrawHoldNotes(const std::shared_ptr<SusDrawableNoteData> & note) const;
	void DrawSlideNotes(const std::shared_ptr<SusDrawableNoteData> & note) const;
	void DrawAirActionStart(const AirDrawQuery & query) const;
	void DrawAirActionStep(const AirDrawQuery & query) const;
	void DrawAirActionStepBox(const AirDrawQuery & query) const;
	void DrawAirActionCover(const AirDrawQuery & query) const;
	void DrawTap(float lane, int length, double relpos, int handle) const;
	void DrawMeasureLine(const std::shared_ptr<SusDrawableNoteData> & note) const;
	void Prepare3DDrawCall() const;
	void DrawAerialNotes(const std::vector<std::shared_ptr<SusDrawableNoteData>> & notes) const;

	void ProcessSound();
	void ProcessSoundQueue();

	void SpawnJudgeEffect(const std::shared_ptr<SusDrawableNoteData> & target, JudgeType type);
	void SpawnSlideLoopEffect(const std::shared_ptr<SusDrawableNoteData> & target);
	void EnqueueJudgeSound(JudgeSoundType type);

public:
	explicit ScenePlayer(ExecutionManager * exm);
	~ScenePlayer() override;

	void AdjustCamera(double cy, double cz, double ctz);
	void GetMetrics(ScenePlayerMetrics * metrics);
	void SetPlayerResource(const std::string & name, SResource * resource);
	void SetLaneSprite(SSprite * spriteLane);
	void Tick(double delta) override;
	void Draw() const override;
	void Finalize();

	void Initialize();
	void Load(const std::string& fileName);
	bool IsScoreLoaded();
	bool IsLoadCompleted();
	void GetReady();
	void Play();
	double GetPlayingTime() const;
	int GetPlayingPosition() const;
	void GetCurrentResult(DrawableResult * result) const;
	void MovePositionBySecond(double sec);
	void MovePositionByMeasure(int meas);
	void SetAbility(SkillDetail* detail);
	void SetJudgeCallback(asIScriptFunction* func) const;
	void SetAbilityCallback(asIScriptFunction * func) const;
	void Pause();
	void Resume();
	void Reload();
	double GetFirstNoteTime() const;
	double GetLastNoteTime() const;
};

void RegisterPlayerScene(asIScriptEngine* engine);
