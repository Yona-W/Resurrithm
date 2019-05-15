/*!
 * @file SkillManager.cpp
 * @brief スキル管理を行うクラス SkillManager の定義
 * @author kb10uy
 * @date 2019/04/29
 * @details スキル定義ファイルの列挙、管理を行うインターフェースを提供します。
 */

#include "SkillManager.h"
#include "Skill.h"
#include "SettingManager.h"

using namespace std;

namespace {
	using namespace filesystem;

	/*!
	 * @brief スキル定義ファイルへのパスをもとにスキルをロードします。
	 * @param[in] file スキル定義ファイルへのパス。Setting::GetRootDirectory() / SU_SKILL_DIR / SU_ICON_DIR に対する相対パスです。
	 * @return スキル情報を返します。ロード、パースに失敗した場合はnullを返します。
	 */
	shared_ptr<SkillParameter> LoadFromToml(asIScriptEngine* engine, path file)
	{
		auto log = spdlog::get("main");
		auto result = make_shared<SkillParameter>();
		const auto iconRoot = SettingManager::GetRootDirectory() / SU_SKILL_DIR / SU_ICON_DIR;

		ifstream ifs(file, ios::in);
		auto pr = toml::parse(ifs);
		ifs.close();
		if (!pr.valid()) {
			log->error(u8"スキル {0} は不正なファイルです : {1}", ConvertUnicodeToUTF8(file), pr.errorReason);
			return nullptr;
		}
		auto& root = pr.value;

		try {
			result->Name = root.get<string>("Name");
			result->IconPath = ConvertUnicodeToUTF8(iconRoot / ConvertUTF8ToUnicode(root.get<string>("Icon")));
			result->Details.clear();
			result->CurrentLevel = 0;
			result->MaxLevel = 0;

			auto details = root.get<vector<toml::Table>>("Detail");
			for (const auto& detail : details) {
				SkillDetail* sdt = new SkillDetail();

				sdt->Level = detail.at("Level").as<int>();
				sdt->Description = detail.at("Description").as<string>();

				const auto ability = detail.at("Ability").as<toml::Table>();
				sdt->AbilityName = ability.at("Type").as<string>();
				const auto args = ability.at("Arguments").as<toml::Table>();
				sdt->Arguments = CScriptDictionary::Create(engine);
				for (const auto& p : args) {
					switch (p.second.type()) {
					case toml::Value::INT_TYPE:
					{
						asINT64 avalue = p.second.as<int>();
						sdt->Arguments->Set(p.first, avalue);
						break;
					}
					case toml::Value::DOUBLE_TYPE:
					{
						double avalue = p.second.as<double>();
						sdt->Arguments->Set(p.first, avalue);
						break;
					}
					case toml::Value::STRING_TYPE:
					{
						auto avalue = p.second.as<string>();
						sdt->Arguments->Set(p.first, &avalue, engine->GetTypeIdByDecl("string"));
						break;
					}
					default:
						break;
					}
				}

				const auto& ind = detail.at("Indicator").as<toml::Table>();
				sdt->Indicators = CScriptDictionary::Create(engine);
				for (const auto& p : ind) {
					switch (p.second.type()) {
					case toml::Value::STRING_TYPE:
					{
						auto avalue = ConvertUnicodeToUTF8(iconRoot / ConvertUTF8ToUnicode(p.second.as<string>()));
						sdt->Indicators->Set(p.first, &avalue, engine->GetTypeIdByDecl("string"));
						break;
					}
					default:
						break;
					}
				}

				result->Details.push_back(sdt);

				if (result->MaxLevel < sdt->Level) result->MaxLevel = sdt->Level;
			}
		}
		catch (exception & ex) {
			log->error(u8"スキル {0} の読み込みに失敗しました: {1}", ConvertUnicodeToUTF8(file), ex.what());
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
void SkillManager::LoadAllSkills(asIScriptEngine* engine)
{
	using namespace filesystem;
	const auto skillroot = SettingManager::GetRootDirectory() / SU_SKILL_DIR / SU_SKILL_DIR;

	if (exists(skillroot)) {
		for (const auto& fdata : directory_iterator(skillroot)) {
			if (is_directory(fdata)) continue;
			if (fdata.path().extension() != ".toml") continue;
			const auto skill = LoadFromToml(engine, fdata.path());
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
