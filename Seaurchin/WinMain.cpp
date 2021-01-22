﻿#include "Misc.h"
#include "WinMain.h"
#include "resource.h"
#include "Debug.h"
#include "Setting.h"
#include "ExecutionManager.h"
#include "SceneDebug.h"
#include "MoverFunctionExpression.h"
#include "Easing.h"
#include "ScriptSpriteMover.h"

using namespace std;
using namespace std::chrono;

void PreInitialize(HINSTANCE hInstance);
bool Initialize();
void Run();
void Terminate();
LRESULT CALLBACK CustomWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

shared_ptr<Setting> setting;
shared_ptr<Logger> logger;
unique_ptr<ExecutionManager> manager;
WNDPROC dxlibWndProc;
HWND hDxlibWnd;

int WINAPI WinMain(const HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    PreInitialize(hInstance);
    if (!Initialize()) {
        logger->LogError(u8"初期化処理に失敗しました。強制終了します。");
        Terminate();
        return -1;
    }

    Run();

    Terminate();
    return 0;
}

void PreInitialize(HINSTANCE hInstance)
{
    logger = make_shared<Logger>();
    logger->Initialize();
    logger->LogDebug(u8"ロガー起動");

    setting = make_shared<Setting>(hInstance);
    setting->Load(SU_SETTING_FILE);
    const auto vs = setting->ReadValue<bool>("Graphic", "WaitVSync", false);
    const auto fs = setting->ReadValue<bool>("Graphic", "Fullscreen", false);

    SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF16LE);
    ChangeWindowMode(fs ? FALSE : TRUE);
    SetMainWindowText(reinterpret_cast<const char*>(ConvertUTF8ToUnicode(SU_APP_NAME " " SU_APP_VERSION).c_str()));
    SetAlwaysRunFlag(TRUE);
    SetWaitVSyncFlag(vs ? TRUE : FALSE);
    SetWindowIconID(IDI_ICON1);
    SetUseFPUPreserveFlag(TRUE);
    SetGraphMode(SU_RES_WIDTH, SU_RES_HEIGHT, 32);
    SetFullSceneAntiAliasingMode(2, 2);

    logger->LogDebug(u8"PreInitialize完了");
}

bool Initialize()
{
    logger->LogDebug(u8"DxLib初期化開始");
    if (DxLib_Init() == -1) abort();
    logger->LogInfo(u8"DxLib初期化OK");

    //WndProc差し替え
    hDxlibWnd = GetMainWindowHandle();
    dxlibWndProc = WNDPROC(GetWindowLong(hDxlibWnd, GWL_WNDPROC));
    SetWindowLong(hDxlibWnd, GWL_WNDPROC, LONG(CustomWindowProc));
    //D3D設定
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetDrawScreen(DX_SCREEN_BACK);

    MoverFunctionExpressionManager::Initialize();
    if (!easing::RegisterDefaultMoverFunctionExpressions()) {
        logger->LogError(u8"デフォルトのMoverFunctionの登録に失敗しました。");
        return false;
    }

    manager = make_unique<ExecutionManager>(setting);
    manager->Initialize();

    SSpriteMover::StrTypeId = manager->GetScriptInterfaceUnsafe()->GetEngine()->GetTypeIdByDecl("string");

    logger->LogDebug(u8"Initialize完了");

    return true;
}

void Run()
{
    if (CheckHitKey(KEY_INPUT_F2)) {
        manager->ExecuteSystemMenu();
    } else {
        logger->LogDebug(u8"スキン列挙開始");
        manager->EnumerateSkins();
        logger->LogDebug(u8"Skin.as起動");
        manager->ExecuteSkin();
        logger->LogDebug(u8"Skin.as終了");
    }
    manager->AddScene(static_pointer_cast<Scene>(make_shared<SceneDebug>()));


    auto start = high_resolution_clock::now();
    auto pstart = start;
    while (ProcessMessage() != -1) {
        pstart = start;
        start = high_resolution_clock::now();
        const auto delta = duration_cast<nanoseconds>(start - pstart).count() / 1000000000.0;
        manager->Tick(delta);
        manager->Draw();
    }
}

void Terminate()
{
    if(manager) manager->Shutdown();
    manager.reset(nullptr);
    MoverFunctionExpressionManager::Finalize();
    if(setting) setting->Save();
    if(setting) setting.reset();
    DxLib_End();
    if(logger) logger->Terminate();
}

LRESULT CALLBACK CustomWindowProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
    bool processed;
    LRESULT result;
    tie(processed, result) = manager->CustomWindowProc(hWnd, msg, wParam, lParam);

    if (processed) {
        return result;
    } else {
        return CallWindowProc(dxlibWndProc, hWnd, msg, wParam, lParam);
    }
}
