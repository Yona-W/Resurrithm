#pragma once

#define ROOT_FILE(dir, fn) ((dir) + "/" + (fn))


#define SU_APP_NAME "Resurrithm"
#define SU_APP_VERSION "0.1.0"

#define SU_SETTING_FILE "config.toml"
#define SU_CACHE_MUSIC_FILE "musics.mp"
#define SU_NAMED_PIPE_NAME "\\\\.\\pipe\\seaurchin"

#define SU_DATA_DIR "Data"
#define SU_IMAGE_DIR "Images"
#define SU_SKIN_DIR "Skins"
#define SU_FONT_DIR "Fonts"
#define SU_CACHE_DIR "Cache"
#define SU_CHARACTER_DIR "Characters"
#define SU_MUSIC_DIR "Music"
#define SU_SOUND_DIR "Sounds"
#define SU_EXTENSION_DIR "Extensions"
#define SU_SCRIPT_DIR "Scripts"
#define SU_SKILL_DIR "Skills"
#define SU_ABILITY_DIR "Abilities"
#define SU_ICON_DIR "Icons"

#define SU_SKIN_MAIN_FILE "Skin.as"
#define SU_SKIN_TITLE_FILE "Title.as"
#define SU_SKIN_SELECT_FILE "Select.as"
#define SU_SKIN_PLAY_FILE "Play.as"
#define SU_SKIN_RESULT_FILE "Result.as"
#define SU_SYSTEM_MENU_FILE "System.as"
#define SU_SETTING_DEFINITION_FILE "SettingList.toml"

#define SU_FONT_SYSTEM "ＭＳ ゴシック"

//#define SU_RES_WIDTH 1920
//#define SU_RES_HEIGHT 1080
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
