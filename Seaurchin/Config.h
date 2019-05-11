#pragma once

#define ROOT_FILE(dir, fn) ((dir) + "/" + (fn))


#define SU_APP_NAME u8"Seaurchin"
#define SU_APP_VERSION u8"0.61.0"

#define SU_SETTING_FILE L"config.toml"
#define SU_CACHE_MUSIC_FILE L"musics.mp"
#define SU_NAMED_PIPE_NAME "\\\\.\\pipe\\seaurchin"

#define SU_DATA_DIR L"Data"
#define SU_IMAGE_DIR L"Images"
#define SU_SKIN_DIR L"Skins"
#define SU_FONT_DIR L"Fonts"
#define SU_CACHE_DIR L"Cache"
#define SU_CHARACTER_DIR L"Characters"
#define SU_MUSIC_DIR L"Music"
#define SU_SOUND_DIR L"Sounds"
#define SU_EXTENSION_DIR L"Extensions"
#define SU_SCRIPT_DIR L"Scripts"
#define SU_SKILL_DIR L"Skills"
#define SU_ABILITY_DIR L"Abilities"
#define SU_ICON_DIR L"Icons"

#define SU_SKIN_MAIN_FILE L"Skin.as"
#define SU_SKIN_TITLE_FILE L"Title.as"
#define SU_SKIN_SELECT_FILE L"Select.as"
#define SU_SKIN_PLAY_FILE L"Play.as"
#define SU_SKIN_RESULT_FILE L"Result.as"
#define SU_SYSTEM_MENU_FILE L"System.as"
#define SU_SETTING_DEFINITION_FILE L"SettingList.toml"

//! @def SU_SETTING_GENERAL
//! @brief 「全体設定」に属する設定値のグループ名
#define SU_SETTING_GENERAL "General"

//! @def SU_SETTING_SKIN
//! @brief 「全体設定/スキン」に属する設定値のキー名
#define SU_SETTING_SKIN "Skin"

#define SU_FONT_SYSTEM "ＭＳ ゴシック"

#define SU_RES_WIDTH 1280
#define SU_RES_HEIGHT 720

#define SU_EFX_PMAX 1024

//AngelScriptのUserData用
#define SU_UDTYPE_SCENE 100
#define SU_UDTYPE_WAIT 101
#define SU_UDTYPE_SKIN 102
#define SU_UDTYPE_ENTRYPOINT 103

// カスタムウィンドウメッセージ

// Seaurchinを落とす
// wp, lp: なし
#define WM_SEAURCHIN_ABORT (WM_APP + 1)

// 再生開始する
// wp, lp: なし
// \\.\pipe\seaurchin に所定の形式でデータを書き出すこと
#define WM_SEAURCHIN_PLAY (WM_APP + 10)

// 情報を取得する
// wp, lp: なし
#define WM_SEAURCHIN_GET_INFO (WM_APP + 11)
