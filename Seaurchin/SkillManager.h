/*!
 * @file SkillManager.h
 * @brief スキル管理を行うクラス SkillManager の宣言
 * @author kb10uy
 * @date 2019/04/29
 * @details スキル定義ファイルの列挙、管理を行うインターフェースを提供します。
 */

#pragma once

/*!
 * @def SU_IF_SKILL_MANAGER
 * @brief スキンで用いるスキルマネージャークラスのクラス名
 */
#define SU_IF_SKILL_MANAGER "SkillManager"

class SkillParameter;

/*!
 * @brief スキルの管理を行います
 * @details 内部的にスキルを配列で保持します。
 * スキル一覧の生成(LoadAllSkills)を行ってから、Next,Prevでスキルを選択します。
 */
class SkillManager final {
	typedef std::vector<std::shared_ptr<SkillParameter>> SkillList;

private:
	SkillList skills;	//!< スキル情報の配列
	int selected;		//!< 選択項目のインデックス。スキル一覧が生成されていない時は負値を取ります。

public:
	explicit SkillManager();

	void LoadAllSkills();

	int32_t GetSize() const { return SU_TO_INT32(skills.size()); }
	void Next();
	void Previous();
	SkillParameter* GetSkillParameterUnsafe(const int relative);
	std::shared_ptr<SkillParameter> GetSkillParameterSafe(const int relative);

	static void RegisterType(asIScriptEngine* engine);
};
