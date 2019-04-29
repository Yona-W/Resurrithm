/*!
 * @file SkillManager.cpp
 * @brief スキル管理を行うクラス SkillManager の定義
 * @author kb10uy
 * @date 2019/04/29
 * @details スキル定義ファイルの列挙、管理を行うインターフェースを提供します。
 */

#include "SkillManager.h"
#include "Skill.h"
#include "Setting.h"

using namespace std;

namespace {
	using namespace filesystem;

	/*!
	 * @brief スキル定義ファイルへのパスをもとにスキルをロードします。
	 * @param[in] file スキル定義ファイルへのパス。Setting::GetRootDirectory() / SU_SKILL_DIR / SU_ICON_DIR に対する相対パスです。
	 * @return スキル情報を返します。ロード、パースに失敗した場合はnullを返します。
	 */
	shared_ptr<SkillParameter> LoadFromToml(path file)
	{
		auto log = spdlog::get("main");
		auto result = make_shared<SkillParameter>();
		const auto iconRoot = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_ICON_DIR;

		ifstream ifs(file.wstring(), ios::in);
		auto pr = toml::parse(ifs);
		ifs.close();
		if (!pr.valid()) {
			log->error(u8"スキル {0} は不正なファイルです : {1}", ConvertUnicodeToUTF8(file.wstring()), pr.errorReason);
			return nullptr;
		}
		auto& root = pr.value;

		try {
			result->Name = root.get<string>("Name");
			result->IconPath = ConvertUnicodeToUTF8((iconRoot / ConvertUTF8ToUnicode(root.get<string>("Icon"))).wstring());
			result->Details.clear();
			result->CurrentLevel = 0;
			result->MaxLevel = 0;

			auto details = root.get<vector<toml::Table>>("Detail");
			for (const auto& detail : details) {
				SkillDetail sdt;
				sdt.Description = detail.at("Description").as<string>();

				auto abilities = detail.at("Abilities").as<vector<toml::Table>>();
				for (const auto& ability : abilities) {
					AbilityParameter ai;
					ai.Name = ability.at("Type").as<string>();
					auto args = ability.at("Arguments").as<toml::Table>();
					for (const auto& p : args) {
						switch (p.second.type()) {
						case toml::Value::INT_TYPE:
							ai.Arguments[p.first] = p.second.as<int>();
							break;
						case toml::Value::DOUBLE_TYPE:
							ai.Arguments[p.first] = p.second.as<double>();
							break;
						case toml::Value::STRING_TYPE:
							ai.Arguments[p.first] = p.second.as<string>();
							break;
						default:
							break;
						}
					}
					sdt.Abilities.push_back(ai);
				}

				auto level = detail.at("Level").as<int>();
				result->Details.push_back(sdt);

				if (result->MaxLevel < level) result->MaxLevel = level;
			}
		}
		catch (exception & ex) {
			log->error(u8"スキル {0} の読み込みに失敗しました: {1}", ConvertUnicodeToUTF8(file.wstring()), ex.what());
			return nullptr;
		}

		return result;
	}
}

SkillManager::SkillManager()
	: selected(-1)
{}

/*!
 * @brief スキル一覧を生成します。
 * @details 読み込む対象は Setting::GetRootDirectory() / SU_SKILL_DIR / SU_SKILL_DIR 直下にある *.toml です。
 * @todo 非同期動作ができた方がベター
 */
void SkillManager::LoadAllSkills()
{
	using namespace filesystem;
	const auto skillroot = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_SKILL_DIR;

	if (exists(skillroot)) {
		for (const auto& fdata : directory_iterator(skillroot)) {
			if (is_directory(fdata)) continue;
			if (fdata.path().extension() != ".toml") continue;
			const auto skill = LoadFromToml(fdata.path());
			if (skill) skills.push_back(skill);
		}
	}

	const auto size = skills.size();
	spdlog::get("main")->info(u8"スキル総数: {0:d}", size);
	selected = (size == 0) ? -1 : 0;
}

/*!
 * @brief 選択スキルを次のスキルに切り替えます。
 */
void SkillManager::Next()
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return;

	selected = (selected + 1) % size;
}

/*!
 * @brief 選択スキルを前のスキルに切り替えます。
 */
void SkillManager::Previous()
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return;

	selected = (selected + size - 1) % size;
}

/*!
 * @brief 相対位置を指定してスキル情報の生ポインタを取得します。
 * @param[in] relative 選択中のスキルに対する相対スキル数。
 * @return 該当するスキル情報の生ポインタ。該当するスキルがない場合nullが返ります。
 */
SkillParameter * SkillManager::GetSkillParameterUnsafe(const int relative)
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return nullptr;

	auto ri = selected + relative;
	while (ri < 0) ri += size;
	return skills[ri % size].get();
}

/*!
 * @brief 相対位置を指定してスキル情報のポインタを取得します。
 * @param[in] relative 選択中のスキルに対する相対スキル数。
 * @return 該当するスキル情報のポインタ。該当するスキルがない場合nullが返ります。
 */
shared_ptr<SkillParameter> SkillManager::GetSkillParameterSafe(const int relative)
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return nullptr;

	auto ri = selected + relative;
	while (ri < 0) ri += size;
	return skills[ri % size];
}

/*!
 * @brief スキンにスキルマネージャーを登録します。
 * @param[in] engine スクリプトエンジン。
 * @return 関連クラス等の登録も行います。
 */
void SkillManager::RegisterType(asIScriptEngine* engine)
{
	RegisterSkillTypes(engine);

	engine->RegisterObjectType(SU_IF_SKILL_MANAGER, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_SKILL_MANAGER, "void Next()", asMETHOD(SkillManager, Next), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_MANAGER, "void Previous()", asMETHOD(SkillManager, Previous), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_MANAGER, SU_IF_SKILL "@ GetSkill(int)", asMETHOD(SkillManager, GetSkillParameterUnsafe), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_MANAGER, "int GetSize()", asMETHOD(SkillManager, GetSize), asCALL_THISCALL);
}
