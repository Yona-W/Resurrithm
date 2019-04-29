/*!
 * @file CharacterManager.h
 * @brief キャラクターの管理を行うクラス CharacterManager の宣言
 * @author kb10uy
 * @date 2019/04/29
 * @details キャラクターの列挙、ロード、取得を行うインターフェースを提供します。
 */

#pragma once

/*!
 * @def SU_IF_CHARACTER_MANAGER
 * @brief スキンで用いるキャラクターマネージャークラスのクラス名
 */
#define SU_IF_CHARACTER_MANAGER "CharacterManager"

class CharacterParameter;
class CharacterImageSet;

/*!
 * @brief キャラクターの管理を行います
 * @details 内部的にキャラクターを配列で保持します。
 * キャラクター一覧の生成(LoadAllCharacters)を行ってから、Next,Prevでキャラクターを選択します。
 */
class CharacterManager final {
	typedef std::vector<std::shared_ptr<CharacterParameter>> CharacterList;

private:
	CharacterList characters;	//<! キャラクター情報の配列
	int selected;				//<! 選択項目のインデックス。キャラクター一覧が生成されていない時は負値を取ります。

public:
	explicit CharacterManager();

	void LoadAllCharacters();

	//! @brief キャラクター一覧の総キャラクター数を返します。
	//! @details キャラクター一覧が生成されていない時は負値を返します。
	int32_t GetSize() const { return SU_TO_INT32(characters.size()); }
	void Next();
	void Previous();

	CharacterParameter* GetCharacterParameterUnsafe(int relative);
	std::shared_ptr<CharacterParameter> GetCharacterParameterSafe(int relative);
	CharacterImageSet* CreateCharacterImages(int relative);

	static void RegisterType(asIScriptEngine* engine);
};
