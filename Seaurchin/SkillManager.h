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

	//! @brief スキル一覧を生成します。
	//! @details 読み込む対象は Setting::GetRootDirectory() / SU_SKILL_DIR / SU_SKILL_DIR 直下にある *.toml です。
	//! @param[in] engine スクリプトエンジン。
	//! @todo 非同期動作ができた方がベター
	void LoadAllSkills(asIScriptEngine* engine);


	//! @brief スキル一覧の総キャラクター数を返します。
	//! @details スキル一覧が生成されていない時は負値を返します。
	int32_t GetSize() const { return SU_TO_INT32(skills.size()); }

	//! @brief 選択スキルを次のスキルに切り替えます。
	void Next();

	//! @brief 選択スキルを前のスキルに切り替えます。
	void Previous();

	//! @brief 相対位置を指定してスキル情報の生ポインタを取得します。
	//! @param[in] relative 選択中のスキルに対する相対スキル数。
	//! @return 該当するスキル情報の生ポインタ。該当するスキルがない場合nullが返ります。
	SkillParameter* GetSkillParameterUnsafe(const int relative);

	//! @brief 相対位置を指定してスキル情報のポインタを取得します。
	//! @param[in] relative 選択中のスキルに対する相対スキル数。
	//! @return 該当するスキル情報のポインタ。該当するスキルがない場合nullが返ります。
	std::shared_ptr<SkillParameter> GetSkillParameterSafe(const int relative);

	//! @brief スキンにスキルマネージャーを登録します。
	//! @param[in] engine スクリプトエンジン。
	//! @return 関連クラス等の登録も行います。
	static void RegisterType(asIScriptEngine* engine);
};
