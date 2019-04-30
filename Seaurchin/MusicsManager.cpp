#include "MusicsManager.h"
#include "SusAnalyzer.h"
#include "SettingManager.h"

using namespace std;
using namespace filesystem;

typedef lock_guard<mutex> LockGuard;

// CategoryInfo ---------------------------

/*!
 * @param[in] path カテゴリの実体に当たるディレクトリへのパス。絶対パスを想定しています。
 */
CategoryInfo::CategoryInfo(const path& cpath)
{
	categoryPath = cpath;
	name = ConvertUnicodeToUTF8(categoryPath.filename().wstring());
}

CategoryInfo::~CategoryInfo()
= default;

// ReSharper disable once CppMemberFunctionMayBeStatic
void CategoryInfo::Reload(bool recreateCache) const
{}


// MusicsManager ---------------------------

MusicsManager::MusicsManager()
	: loading(false)
	, analyzer(make_unique<SusAnalyzer>(192))
	, categoryIndex(0)
	, musicIndex(-1)
	, variantIndex(-1)
	, state(MusicSelectionState::Category)
{}

MusicsManager::~MusicsManager()
{
	if (loadWorker.joinable()) loadWorker.join();
}

/*!
 * @brief 譜面一覧を再読み込みします。
 * @details 現在の譜面一覧を破棄し、該当ディレクトリから譜面を列挙し、所有するアナライザで譜面を解析します。
 * かなりの時間がかかる処理になるので、外部からはこれを直接実行せず、
 * loadingフラグを用いて適切に非同期処理することが推奨されます。
 */
void MusicsManager::CreateMusicCache()
{
	{
		LockGuard lock(flagMutex);
		loading = true;
	}

	categories.clear();

	const auto mlpath = SettingManager::GetRootDirectory() / SU_MUSIC_DIR;
	for (const auto& fdata : directory_iterator(mlpath)) {
		if (!is_directory(fdata)) continue;

		auto category = make_shared<CategoryInfo>(fdata);
		for (const auto& mdir : directory_iterator(fdata)) {
			if (!is_directory(mdir)) continue;
			for (const auto& file : directory_iterator(mdir)) {
				if (is_directory(file)) continue;
				if (file.path().extension() != ".sus") continue;     //これ大文字どうすんの
				analyzer->Reset();
				analyzer->LoadFromFile(file.path().wstring(), true);
				auto music = find_if(category->Musics.begin(), category->Musics.end(), [&](const std::shared_ptr<MusicMetaInfo> info) {
					return info->SongId == analyzer->SharedMetaData.USongId;
					});
				if (music == category->Musics.end()) {
					music = category->Musics.insert(category->Musics.begin(), make_shared<MusicMetaInfo>());
					(*music)->SongId = analyzer->SharedMetaData.USongId;
					(*music)->Name = analyzer->SharedMetaData.UTitle;
					(*music)->Artist = analyzer->SharedMetaData.UArtist;
					(*music)->JacketPath = mdir.path().filename() / ConvertUTF8ToUnicode(analyzer->SharedMetaData.UJacketFileName);
				}
				auto score = make_shared<MusicScoreInfo>();
				score->Path = mdir.path().filename() / file.path().filename();
				score->BackgroundPath = ConvertUTF8ToUnicode(analyzer->SharedMetaData.UBackgroundFileName);
				score->WavePath = ConvertUTF8ToUnicode(analyzer->SharedMetaData.UWaveFileName);
				score->Designer = analyzer->SharedMetaData.UDesigner;
				score->BpmToShow = analyzer->SharedMetaData.ShowBpm;
				score->Difficulty = analyzer->SharedMetaData.DifficultyType;
				score->DifficultyName = analyzer->SharedMetaData.UExtraDifficulty;
				score->Level = analyzer->SharedMetaData.Level;
				(*music)->Scores.push_back(score);
			}
		}
		categories.push_back(category);
	}

	{
		LockGuard lock(flagMutex);
		loading = false;
	}
}

/*!
 * @brief 譜面一覧を再読み込みします。
 * @param[in] async 非同期で読み込みたい場合はtrueを渡してください。
 * @return 関数の実行に成功するとSuccessが返ります。譜面一覧再読み込み中に実行するとReloadingを返します。
 * @details エクステンションの列挙、初期化、登録を行うインターフェースを提供します。
 */
MusicSelectionState MusicsManager::ReloadMusic(const bool async)
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	if (async) {
		if (loadWorker.joinable()) {
			spdlog::get("main")->error(u8"MusicCursor::ReloadMusicは実行中です。");
			return MusicSelectionState::Reloading;
		}

		thread loadthread([this] { CreateMusicCache(); });
		loadWorker.swap(loadthread);
	}
	else {
		CreateMusicCache();
	}

	return MusicSelectionState::Success;
}

/*!
 * @brief 譜面一覧の再読み込み中かどうかを確認します。
 * @return 譜面一覧の再読み込み中ならtrueを返します。
 */
bool MusicsManager::IsReloading() const
{
	LockGuard lock(flagMutex);
	return loading;
}

/*!
 * @brief 譜面選択状態をリセットします。
 * @return 関数の実行に成功するとSuccessが返ります。譜面一覧再読み込み中に実行するとReloadingを返します。
 * @details 関数の実行に成功すると、カテゴリ選択状態で0番目のカテゴリを選択中の状態になります。
 */
MusicSelectionState MusicsManager::ResetState()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	categoryIndex = 0;
	musicIndex = -1;
	variantIndex = -1;
	state = MusicSelectionState::Category;

	return MusicSelectionState::Success;
}

/*!
 * @brief 現在選択中のカテゴリから相対位置を指定してカテゴリ情報を取得します。
 * @param[in] relative 現在選択中のカテゴリに対する相対カテゴリ数。
 * @return 該当位置にあるカテゴリ情報を返します。カテゴリ情報取得に失敗するとnullを返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
std::shared_ptr<CategoryInfo> MusicsManager::GetCategoryAt(const int32_t relative) const
{
	// NOTE: リロード中は負数が返ってくる
	const auto categorySize = GetCategorySize();
	if (categorySize <= 0) return nullptr;

	auto actual = relative + categoryIndex;
	while (actual < 0) actual += categorySize;
	return categories[actual % categorySize];
}

/*!
 * @brief 現在選択中の楽曲から相対位置を指定して楽曲情報を取得します。
 * @param[in] relative 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にある楽曲情報を返します。楽曲情報取得に失敗するとnullを返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
std::shared_ptr<MusicMetaInfo> MusicsManager::GetMusicAt(const int32_t relative) const
{
	// NOTE: リロード中はnullが返ってくる
	const auto category = GetCategoryAt(0);
	if (!category) return nullptr;

	const auto musicSize = category->Musics.size();
	if (musicSize == 0) return nullptr;

	auto actual = relative + musicIndex;
	while (actual < 0) actual += musicSize;
	return category->Musics[actual % musicSize];
}

/*!
 * @brief 現在選択中の譜面から相対位置を指定して譜面情報を取得します。
 * @param[in] relative 現在選択中の譜面に対する相対譜面差分数。
 * @return 該当位置にある譜面情報を返します。譜面情報取得に失敗するとnullを返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
std::shared_ptr<MusicScoreInfo> MusicsManager::GetScoreVariantAt(const int32_t relative) const
{
	// NOTE: リロード中はnullが返ってくる
	const auto music = GetMusicAt(relative);
	if (!music) return nullptr;

	const auto size = music->Scores.size();
	if (size == 0 || variantIndex < 0) return nullptr;

	return music->Scores[min(SU_TO_UINT32(variantIndex), size - 1)];
}

/*!
 * @brief 現在選択中の譜面の実体となるSUSファイルのパスを返します。
 * @return 現在選択中の譜面の実体となるSUSファイルへの絶対パス。パスの取得に失敗した場合、空のパスが返ります。
 * @details 特にリロード中に実行するとパスの取得に失敗します。
 */
path MusicsManager::GetSelectedScorePath() const
{
	const auto cat = GetCategoryAt(0);
	const auto score = GetScoreVariantAt(0);
	if (!cat || !score) return path();

	return SettingManager::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(cat->GetName()) / score->Path;
}

/*!
 * @brief 現在の選択状態で取得可能な文字列情報を相対位置を指定して取得します。
 * @param[in] relativeIndex 現在選択中の項目に対する相対項目数。
 * @return カテゴリ選択中ならカテゴリ名を、楽曲選択中なら楽曲名を返します。情報の取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
std::string MusicsManager::GetPrimaryString(const int32_t relativeIndex) const
{
	switch (state) {
	case MusicSelectionState::Category:
		return GetCategoryName(relativeIndex);
	case MusicSelectionState::Music:
		return GetMusicName(relativeIndex);
	default:
		return "Unavailable!";
	}
}

/*!
 * @brief 現在選択中のカテゴリから相対位置をしてカテゴリ名を取得します。
 * @param[in] relativeIndex 現在選択中のカテゴリに対する相対カテゴリ数。
 * @return 該当位置にあるカテゴリ名を返します。カテゴリ名の取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
string MusicsManager::GetCategoryName(const int32_t relativeIndex) const
{
	const auto category = GetCategoryAt(relativeIndex);
	return category ? category->GetName() : "Unavailable!";
}

/*!
 * @brief 現在選択中の楽曲から相対位置をして楽曲名を取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にある楽曲名を返します。楽曲名の取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
string MusicsManager::GetMusicName(const int32_t relativeIndex) const
{
	const auto music = GetMusicAt(relativeIndex);
	return music ? music->Name : "Unavailable!";
}

/*!
 * @brief 現在選択中の楽曲から相対位置をしてアーティスト名を取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にあるアーティスト名を返します。アーティスト名の取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
string MusicsManager::GetArtistName(const int32_t relativeIndex) const
{
	const auto music = GetMusicAt(relativeIndex);
	return music ? music->Artist : "Unavailable!";
}

/*!
 * @brief 現在選択中の楽曲から相対位置をしてジャケットファイルパスを取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にあるジャケットファイルパスを返します。ジャケットファイルパスの取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
string MusicsManager::GetMusicJacketFileName(const int32_t relativeIndex) const
{
	const auto music = GetMusicAt(relativeIndex);
	if (!music || music->JacketPath.empty()) return "";

	const auto result = SettingManager::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(GetCategoryName(0)) / music->JacketPath;
	return ConvertUnicodeToUTF8(result.wstring());
}

/*!
 * @brief 現在選択中の楽曲から相対位置をして背景動画ファイルパスを取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にある背景動画ファイルパスを返します。絶対パスです。背景動画ファイルパスの取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
string MusicsManager::GetBackgroundFileName(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	if (!variant || variant->BackgroundPath.empty()) return "";

	const auto result = SettingManager::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(GetCategoryName(0)) / variant->Path.parent_path() / variant->BackgroundPath;
	return ConvertUnicodeToUTF8(result.wstring());
}

/*!
 * @brief 現在選択中の楽曲から相対位置をして難易度を取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にある難易度を返します。難易度の取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
int MusicsManager::GetDifficulty(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->Difficulty : 0;
}

/*!
 * @brief 現在選択中の楽曲から相対位置をして譜面レベルを取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にある譜面レベルを返します。譜面レベルの取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
int MusicsManager::GetLevel(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->Level : 0;
}

/*!
 * @brief 現在選択中の楽曲から相対位置をしてBPMを取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にあるBPMを返します。BPMの取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
double MusicsManager::GetBpm(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->BpmToShow : 0;
}

/*!
 * @brief 現在選択中の楽曲から相対位置をしてレベル追加情報を取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にあるレベル追加情報を返します。レベル追加情報の取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
std::string MusicsManager::GetExtraLevel(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->DifficultyName : "";
}

/*!
 * @brief 現在選択中の楽曲から相対位置をして譜面製作者名を取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当位置にある譜面製作者名を返します。譜面製作者名の取得に失敗すると"Unavailable!"を返します。
 * @details 特にリロード中に実行すると情報取得に失敗します。
 */
std::string MusicsManager::GetDesignerName(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->Designer : "";
}

/*!
 * @brief 現在選択中の項目を「確定」します。
 * @return 関数の実行に成功するとSuccess/Confirmedを返します。
 * 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
 * @details 譜面選択操作のすべてが完了した際にはConfirmedが返ります。
 * 「確定」できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
 */
MusicSelectionState MusicsManager::Enter()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Category:
		if (GetCategorySize() <= 0) return MusicSelectionState::Error;
		state = MusicSelectionState::Music;
		musicIndex = 0;
		variantIndex = 0;
		return MusicSelectionState::Success;
	case MusicSelectionState::Music:
		//選曲終了
		state = MusicSelectionState::OutOfFunction;
		return MusicSelectionState::Confirmed;
	default:
		return MusicSelectionState::OutOfFunction;
	}
}

/*!
 * @brief 現在選択中の項目を「取り消し」ます。
 * @return 関数の実行に成功するとSuccessを返します。
 * 関数の実行に失敗するとReloading/OutOfFunctionが返ります。
 * @details 「取り消し」できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
 */
MusicSelectionState MusicsManager::Exit()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Category:
		state = MusicSelectionState::OutOfFunction;
		break;
	case MusicSelectionState::Music:
		state = MusicSelectionState::Category;
		musicIndex = -1;
		variantIndex = -1;
		break;
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

/*!
 * @brief 現在選択中の項目に対して「次」の項目へ移ります。
 * @return 関数の実行に成功するとSuccessを返します。
 * 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
 * @details 「次」の項目を選択できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
 */
MusicSelectionState MusicsManager::Next()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Category: {
		const auto categorySize = GetCategorySize();
		if (categorySize <= 0 || categoryIndex < 0) return MusicSelectionState::Error;

		categoryIndex = (categoryIndex + 1) % categorySize;
		break;
	}
	case MusicSelectionState::Music: {
		const auto musicSize = GetMusicSize(0);
		const auto lastMusicIndex = musicIndex;
		if (musicSize <= 0 || musicIndex < 0) return MusicSelectionState::Error;

		musicIndex = (musicIndex + 1) % musicSize;
		const auto nextVariant = min(SU_TO_INT32(variantIndex), GetVariantSize(0) - 1);
		if (nextVariant < 0) {
			musicIndex = lastMusicIndex;
			return MusicSelectionState::Error;
		}
		variantIndex = SU_TO_UINT16(nextVariant);
		break;
	}
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

/*!
 * @brief 現在選択中の項目に対して「前」の項目へ移ります。
 * @return 関数の実行に成功するとSuccessを返します。
 * 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
 * @details 「前」の項目を選択できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
 */
MusicSelectionState MusicsManager::Previous()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Category: {
		const auto categorySize = GetCategorySize();
		if (categorySize <= 0 || categoryIndex < 0) return MusicSelectionState::Error;

		categoryIndex = (categoryIndex + categorySize - 1) % categorySize;
		break;

	}
	case MusicSelectionState::Music: {
		const auto musicSize = GetMusicSize(0);
		const auto lastMusicIndex = musicIndex;
		if (musicSize <= 0 || musicIndex < 0) return MusicSelectionState::Error;

		musicIndex = (musicIndex + musicSize - 1) % musicSize;
		const auto nextVariant = min(SU_TO_INT32(variantIndex), GetVariantSize(0) - 1);
		if (nextVariant < 0) {
			musicIndex = lastMusicIndex;
			return MusicSelectionState::Error;
		}
		variantIndex = SU_TO_UINT16(nextVariant);
		break;
	}
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

/*!
 * @brief 現在選択中の項目に対して「次の差分」へ移ります。
 * @return 関数の実行に成功するとSuccessを返します。
 * 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
 * @details 「次の差分」を選択できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
 */
MusicSelectionState MusicsManager::NextVariant()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Music: {
		const auto variantSize = GetVariantSize(0);
		if (variantSize <= 0 || variantIndex < 0) return MusicSelectionState::Error;

		variantIndex = (variantIndex + 1) % variantSize;
		break;
	}
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

/*!
 * @brief 現在選択中の項目に対して「前の差分」へ移ります。
 * @return 関数の実行に成功するとSuccessを返します。
 * 関数の実行に失敗するとReloading/Error/OutOfFunctionが返ります。
 * @details 「前の差分」を選択できない状態でこの関数を実行した際にはOutOfFunctionが返ります。
 */
MusicSelectionState MusicsManager::PreviousVariant()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Music: {
		const auto variantSize = GetVariantSize(0);
		if (variantSize <= 0 || variantIndex < 0) return MusicSelectionState::Error;

		variantIndex = (variantIndex + variantSize - 1) % variantSize;
		break;
	}
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

/*!
 * @brief 現在の選択状態を返します。
 * @return 現在の選択状態。
 */
MusicSelectionState MusicsManager::GetState() const
{
	return IsReloading() ? MusicSelectionState::Reloading : state;
}

/*!
 * @brief 総カテゴリ数を返します。
 * @return 総カテゴリ数。
 * @details 再読み込み中は負数が返ります。
 */
int32_t MusicsManager::GetCategorySize() const
{
	return IsReloading() ? -1 : SU_TO_INT32(categories.size());
}

/*!
 * @brief 現在選択中のカテゴリから相対位置をしてカテゴリ内の楽曲数を取得します。
 * @param[in] relativeIndex 現在選択中のカテゴリに対する相対カテゴリ数。
 * @return 該当するカテゴリ内の楽曲数。
 * @details 再読み込み中は負数が返ります。
 */
int32_t MusicsManager::GetMusicSize(int32_t relativeIndex) const
{
	const auto category = GetCategoryAt(relativeIndex);
	return category ? SU_TO_INT32(category->Musics.size()) : -1;
}

/*!
 * @brief 現在選択中の楽曲から相対位置をして楽曲内の譜面差分数を取得します。
 * @param[in] relativeIndex 現在選択中の楽曲に対する相対楽曲数。
 * @return 該当する楽曲内の差分譜面数。
 * @details 再読み込み中は負数が返ります。
 */
int32_t MusicsManager::GetVariantSize(int32_t relativeIndex) const
{
	const auto music = GetMusicAt(relativeIndex);
	return music ? SU_TO_INT32(music->Scores.size()) : -1;
}

/*!
 * @brief ミュージックカーソルクラスをスキンに登録します。
 * @param[in] engine スクリプトエンジン。
 */
void MusicsManager::RegisterScriptInterface(asIScriptEngine* engine)
{
	engine->RegisterEnum(SU_IF_MSCSTATE);
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "OutOfFunction", int(MusicSelectionState::OutOfFunction));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Category", int(MusicSelectionState::Category));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Music", int(MusicSelectionState::Music));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Confirmed", int(MusicSelectionState::Confirmed));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Reloading", int(MusicSelectionState::Reloading));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Error", int(MusicSelectionState::Error));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Success", int(MusicSelectionState::Success));

	engine->RegisterObjectType(SU_IF_MSCURSOR, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " ReloadMusic(bool = false)", asMETHOD(MusicsManager, ReloadMusic), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " ResetState()", asMETHOD(MusicsManager, ResetState), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetPrimaryString(int = 0)", asMETHOD(MusicsManager, GetPrimaryString), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetCategoryName(int = 0)", asMETHOD(MusicsManager, GetCategoryName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetMusicName(int = 0)", asMETHOD(MusicsManager, GetMusicName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetArtistName(int = 0)", asMETHOD(MusicsManager, GetArtistName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetMusicJacketFileName(int = 0)", asMETHOD(MusicsManager, GetMusicJacketFileName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetBackgroundFileName(int = 0)", asMETHOD(MusicsManager, GetBackgroundFileName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetDifficulty(int = 0)", asMETHOD(MusicsManager, GetDifficulty), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetLevel(int = 0)", asMETHOD(MusicsManager, GetLevel), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "double GetBpm(int = 0)", asMETHOD(MusicsManager, GetBpm), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetExtraLevel(int = 0)", asMETHOD(MusicsManager, GetExtraLevel), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetDesignerName(int = 0)", asMETHOD(MusicsManager, GetDesignerName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Next()", asMETHOD(MusicsManager, Next), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Previous()", asMETHOD(MusicsManager, Previous), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " NextVariant()", asMETHOD(MusicsManager, NextVariant), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " PreviousVariant()", asMETHOD(MusicsManager, PreviousVariant), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Enter()", asMETHOD(MusicsManager, Enter), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Exit()", asMETHOD(MusicsManager, Exit), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " GetState()", asMETHOD(MusicsManager, GetState), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetCategorySize()", asMETHOD(MusicsManager, GetCategorySize), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetMusicSize(int = 0)", asMETHOD(MusicsManager, GetMusicSize), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetVariantSize(int = 0)", asMETHOD(MusicsManager, GetVariantSize), asCALL_THISCALL);
}

