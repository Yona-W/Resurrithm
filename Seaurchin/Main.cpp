#include "Main.h"
#include "resource.h"
#include "Debug.h"
#include "Setting.h"
#include "ExecutionManager.h"
#include "AngelScriptManager.h"
#include "MoverFunctionExpression.h"
#include "Easing.h"
#include "ScriptSpriteMover.h"

using namespace std;

void PreInitialize(HINSTANCE hInstance);
bool Initialize();
void Run();
void Terminate();
LRESULT CALLBACK CustomWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

unique_ptr<Logger> logger;
unique_ptr<ExecutionManager> manager;
WNDPROC dxlibWndProc;

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
	logger = make_unique<Logger>();
	logger->Initialize();
	logger->LogDebug(u8"ロガー起動");

	auto setting = make_unique<SettingTree>(hInstance, SU_SETTING_FILE);
	setting->Load();
	const auto vs = setting->ReadValue<bool>("Graphic", "WaitVSync", false);
	const auto fs = setting->ReadValue<bool>("Graphic", "Fullscreen", false);

	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);
	ChangeWindowMode(fs ? FALSE : TRUE);
	SetMainWindowText(SU_APP_NAME u8" " SU_APP_VERSION);
	SetAlwaysRunFlag(TRUE);
	SetWaitVSyncFlag(vs ? TRUE : FALSE);
	SetWindowIconID(IDI_ICON1);
	SetUseFPUPreserveFlag(TRUE);
	SetGraphMode(SU_RES_WIDTH, SU_RES_HEIGHT, 32);
	SetFullSceneAntiAliasingMode(2, 2);

	manager = make_unique<ExecutionManager>(setting.release());

	logger->LogDebug(u8"PreInitialize完了");
}

bool Initialize()
{
	logger->LogDebug(u8"DxLib初期化開始");
	if (DxLib_Init() == -1) abort();
	logger->LogInfo(u8"DxLib初期化OK");

	//WndProc差し替え
	{
		const HWND hDxlibWnd = GetMainWindowHandle();
		dxlibWndProc = WNDPROC(GetWindowLong(hDxlibWnd, GWL_WNDPROC));
		SetWindowLong(hDxlibWnd, GWL_WNDPROC, LONG(CustomWindowProc));
	}

	//D3D設定
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetDrawScreen(DX_SCREEN_BACK);

	MoverFunctionExpressionManager::Initialize();
	if (!easing::RegisterDefaultMoverFunctionExpressions()) {
		logger->LogError(u8"デフォルトのMoverFunctionの登録に失敗しました。");
		return false;
	}

	manager->Initialize();

	SSpriteMover::StrTypeId = manager->GetScriptInterfaceUnsafe()->GetEngine()->GetTypeIdByDecl("string");

	logger->LogDebug(u8"Initialize完了");

	return true;
}

void Run()
{
	using namespace std::chrono;

	logger->LogDebug(u8"スキン列挙開始");
	manager->EnumerateSkins();
	logger->LogDebug(u8"Skin.as起動");
	manager->ExecuteSkin();
	logger->LogDebug(u8"Skin.as終了");


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
	manager.reset();
	MoverFunctionExpressionManager::Finalize();
	DxLib_End();
	logger->Terminate();
	logger.reset();
}

LRESULT CALLBACK CustomWindowProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	LRESULT result;

	if (manager->CustomWindowProc(hWnd, msg, wParam, lParam, &result)) {
		return result;
	}
	else {
		return CallWindowProc(dxlibWndProc, hWnd, msg, wParam, lParam);
	}
}
