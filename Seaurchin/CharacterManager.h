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
	CharacterList characters;	//!< キャラクター情報の配列
	int selected;				//!< 選択項目のインデックス。キャラクター一覧が生成されていない時は負値を取ります。

public:
	explicit CharacterManager();

	//! @brief キャラクター一覧を生成します。
	//! @details 読み込む対象は Setting::GetRootDirectory() / SU_CHARACTER_DIR 直下にある *.toml です。
	//! @todo 非同期動作ができた方がベター
	void LoadAllCharacters();

	//! @brief キャラクター一覧の総キャラクター数を返します。
	//! @details キャラクター一覧が生成されていない時は負値を返します。
	int32_t GetSize() const { return SU_TO_INT32(characters.size()); }

	//! @brief 選択キャラクターを次のキャラクターに切り替えます。
	void Next();

	//! @brief 選択キャラクターを前のキャラクターに切り替えます。
	void Previous();

	//! @brief 相対位置を指定してキャラクター情報の生ポインタを取得します。
	//! @param[in] relative 選択中のキャラクターに対する相対キャラクター数。
	//! @return 該当するキャラクター情報の生ポインタ。該当するキャラクターがいない場合nullが返ります。
	CharacterParameter* GetCharacterParameterUnsafe(int relative);

	//! @brief 相対位置を指定してキャラクター情報のスマートポインタを取得します。
	//! @param[in] relative 選択中のキャラクターに対する相対キャラクター数。
	//! @return 該当するキャラクター情報のスマートポインタ。該当するキャラクターがいない場合nullが返ります。
	std::shared_ptr<CharacterParameter> GetCharacterParameterSafe(int relative);

	//! @brief 相対位置を指定してキャラクターの画像情報を生成します。
	//! @param[in] relative 選択中のキャラクターに対する相対キャラクター数。
	//! @return 該当するキャラクターの画像情報の生ポインタ。該当するキャラクターがいない場合nullが返ります。
	CharacterImageSet* CreateCharacterImages(int relative);

	//! @brief スクリプトエンジンにキャラクターマネージャーを登録します。
	//! @param[in] engine スクリプトエンジン。
	//! @note 関連クラス(ImageSet,ImageMetric,Parameter)の登録も行います。
	static void RegisterType(asIScriptEngine* engine);
};
