/*!
 * @file CharacterManager.cpp
 * @brief キャラクターの管理を行うクラス CharacterManager の定義
 * @author kb10uy
 * @date 2019/04/29
 * @details キャラクターの列挙、ロード、取得を行うインターフェースを提供します。
 */

#include "CharacterManager.h"
#include "Character.h"
#include "SettingManager.h"

using namespace std;

namespace {
	using namespace filesystem;

	/*!
	 * @brief キャラクター定義ファイルへのパスをもとにキャラクター情報を読み込みます。
	 * @return 読み込んだキャラクター情報を返します。キャラクター情報の読み込みに失敗した場合、nullを返します。
	 */
	shared_ptr<CharacterParameter> LoadFromToml(const path& file)
	{
		auto log = spdlog::get("main");
		auto result = make_shared<CharacterParameter>();

		ifstream ifs(file, ios::in);
		auto pr = toml::parse(ifs);
		ifs.close();
		if (!pr.valid()) {
			log->error(u8"キャラクター {0} は不正なファイルです", ConvertUnicodeToUTF8(file));
			log->error(pr.errorReason);
			return nullptr;
		}
		auto& root = pr.value;

		try {
			result->Name = root.get<string>("Name");
			const auto imgpath = file.parent_path() / ConvertUTF8ToUnicode(root.get<string>("Image"));
			result->ImagePath = ConvertUnicodeToUTF8(imgpath);

			const auto ws = root.find("Metric.WholeScale");
			result->Metric.WholeScale = (ws && ws->is<double>()) ? ws->as<double>() : 1.0;

			const auto fo = root.find("Metric.Face");
			if (fo && fo->is<vector<int>>()) {
				const auto arr = fo->as<vector<int>>();
				size_t i = 0;
				for (; i < 2 && i < arr.size(); ++i) result->Metric.FaceOrigin[i] = arr[i];
				for (; i < 2; ++i) result->Metric.FaceOrigin[i] = 0;
			}
			else {
				result->Metric.FaceOrigin[0] = 0;
				result->Metric.FaceOrigin[1] = 0;
			}

			const auto sr = root.find("Metric.SmallRange");
			if (sr && sr->is<vector<int>>()) {
				auto arr = sr->as<vector<int>>();
				size_t i = 0;
				for (; i < 4 && i < arr.size(); ++i) result->Metric.SmallRange[i] = arr[i];
				for (; i < 4; ++i) result->Metric.SmallRange[i] = 0; // NOTE: 適当
			}
			else {
				result->Metric.SmallRange[0] = 0;
				result->Metric.SmallRange[1] = 0;
				result->Metric.SmallRange[2] = 280;
				result->Metric.SmallRange[3] = 170;
			}

			const auto fr = root.find("Metric.FaceRange");
			if (fr && fr->is<vector<int>>()) {
				auto arr = sr->as<vector<int>>();
				size_t i = 0;
				for (; i < 4 && i < arr.size(); ++i) result->Metric.FaceRange[i] = arr[i];
				for (; i < 4; ++i) result->Metric.FaceRange[i] = 0; // NOTE: 適当
			}
			else {
				result->Metric.FaceRange[0] = 0;
				result->Metric.FaceRange[1] = 0;
				result->Metric.FaceRange[2] = 128;
				result->Metric.FaceRange[3] = 128;
			}
		}
		catch (exception ex) {
			log->error(u8"キャラクター {0} の読み込みに失敗しました", ConvertUnicodeToUTF8(file));
			log->error(ex.what());
			return nullptr;
		}

		return result;
	}
}

CharacterManager::CharacterManager()
	: selected(-1)
{}

/*!
 * @brief キャラクター一覧を生成します。
 * @details 読み込む対象は Setting::GetRootDirectory() / SU_CHARACTER_DIR 直下にある *.toml です。
 * @todo 非同期動作ができた方がベター
 */
void CharacterManager::LoadAllCharacters()
{
	const auto sepath = SettingManager::GetRootDirectory() / SU_CHARACTER_DIR;

	if (filesystem::exists(sepath)) {
		for (const auto& fdata : directory_iterator(sepath)) {
			if (is_directory(fdata)) continue;
			if (fdata.path().extension() != ".toml") continue;
			const auto param = LoadFromToml(fdata.path());
			if (param) characters.push_back(param);
		}
	}

	const auto size = characters.size();
	spdlog::get("main")->info(u8"キャラクター総数: {0:d}", size);
	selected = (size == 0) ? -1 : 0;
}

/*!
 * @brief 選択キャラクターを次のキャラクターに切り替えます。
 */
void CharacterManager::Next()
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return;

	selected = (selected + 1) % size;
}

/*!
 * @brief 選択キャラクターを前のキャラクターに切り替えます。
 */
void CharacterManager::Previous()
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return;

	selected = (selected + size - 1) % size;
}

/*!
 * @brief 相対位置を指定してキャラクター情報の生ポインタを取得します。
 * @param[in] relative 選択中のキャラクターに対する相対キャラクター数。
 * @return 該当するキャラクター情報の生ポインタ。該当するキャラクターがいない場合nullが返ります。
 */
CharacterParameter * CharacterManager::GetCharacterParameterUnsafe(const int relative)
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return nullptr;

	auto ri = selected + relative;
	while (ri < 0) ri += size;
	return characters[ri % size].get();
}

/*!
 * @brief 相対位置を指定してキャラクター情報のスマートポインタを取得します。
 * @param[in] relative 選択中のキャラクターに対する相対キャラクター数。
 * @return 該当するキャラクター情報のスマートポインタ。該当するキャラクターがいない場合nullが返ります。
 */
shared_ptr<CharacterParameter> CharacterManager::GetCharacterParameterSafe(const int relative)
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return nullptr;

	auto ri = selected + relative;
	while (ri < 0) ri += size;
	return characters[ri % size];
}

/*!
 * @brief 相対位置を指定してキャラクターの画像情報を生成します。
 * @param[in] relative 選択中のキャラクターに対する相対キャラクター数。
 * @return 該当するキャラクターの画像情報の生ポインタ。該当するキャラクターがいない場合nullが返ります。
 */
CharacterImageSet* CharacterManager::CreateCharacterImages(const int relative)
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return nullptr;

	auto ri = selected + relative;
	while (ri < 0) ri += size;
	const auto cp = characters[ri % size];
	return CharacterImageSet::CreateImageSet(cp);
}

/*!
 * @brief スキンにキャラクターマネージャーを登録します。
 * @param[in] engine スクリプトエンジン。
 * @details 関連クラス(ImageSet,ImageMetric,Parameter)の登録も行います。
 */
void CharacterManager::RegisterType(asIScriptEngine* engine)
{
	CharacterImageSet::RegisterType(engine);
	CharacterImageMetric::RegisterType(engine);
	CharacterParameter::RegisterType(engine);

	engine->RegisterObjectType(SU_IF_CHARACTER_MANAGER, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, "void Next()", asMETHOD(CharacterManager, Next), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, "void Previous()", asMETHOD(CharacterManager, Previous), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, "int GetSize()", asMETHOD(CharacterManager, GetSize), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, SU_IF_CHARACTER_PARAM "@ GetCharacter(int = 0)", asMETHOD(CharacterManager, GetCharacterParameterUnsafe), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, SU_IF_CHARACTER_IMAGES "@ CreateCharacterImages(int = 0)", asMETHOD(CharacterManager, CreateCharacterImages), asCALL_THISCALL);
}
