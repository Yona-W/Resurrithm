#include "ScriptCoroutine.h"

using namespace std;
using namespace std::filesystem;

namespace {
	static int CALLBACK FontEnumerationProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD type, LPARAM lParam);

	// ReSharper disable once CppParameterNeverUsed
	static int CALLBACK FontEnumerationProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD type, LPARAM lParam)
	{
		return 0;
	}

	// TODO: フォントを列挙したい
	// TODO: 適切な位置に再配置する
	void EnumerateInstalledFonts()
	{
		// HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\Fonts
		const auto hdc = GetDC(GetMainWindowHandle());
		LOGFONT logfont;
		logfont.lfCharSet = DEFAULT_CHARSET;
		memcpy_s(logfont.lfFaceName, sizeof(logfont.lfFaceName), "", 1);
		logfont.lfPitchAndFamily = 0;
		EnumFontFamiliesEx(hdc, &logfont, FONTENUMPROC(FontEnumerationProc), LPARAM(0), 0);
	}
}


Coroutine::Coroutine(const std::string& name, const asIScriptFunction* cofunc)
	: context(cofunc->GetEngine()->CreateContext())
	, object(static_cast<asIScriptObject*>(cofunc->GetDelegateObject()))
	, function(cofunc->GetDelegateFunction())
	, Name(name)
	, wait(0ull)
{
	SU_ASSERT(cofunc->GetFuncType() == asFUNC_DELEGATE);

	function->AddRef();
	object->AddRef();

	const auto r1 = context->Prepare(function) == asSUCCESS;
	const auto r2 = context->SetObject(object) == asSUCCESS;
	SU_ASSERT(r1 && r2);

	SetUserData(&wait, SU_UDTYPE_WAIT);

	cofunc->Release();
}

Coroutine::~Coroutine()
{
	context->Unprepare();

	auto e = context->GetEngine();
	context->Release();
	function->Release();
	e->ReleaseScriptObject(object, object->GetObjectType());
}

int Coroutine::Tick(double delta)
{
	if (wait.Tick(delta)) return asEXECUTION_SUSPENDED;

	const auto result = context->Execute();

	switch (result) {
	case asEXECUTION_FINISHED: // 正常終了
	case asEXECUTION_SUSPENDED: // 中断
		break;
	case asEXECUTION_ABORTED: // Abort 現状起こりえない
		spdlog::get("main")->error(u8"期待しない動作 : 関数実行が強制終了しました。");
		break;
	case asEXECUTION_EXCEPTION:
	{
		int col;
		const char* at;
		const auto row = context->GetExceptionLineNumber(&col, &at);
		spdlog::get("main")->error(u8"{0} ({1:d}行{2:d}列) にて例外を検出しました : {3}", at, row, col, context->GetExceptionString());
		break;
	}
	default:
		spdlog::get("main")->error(u8"期待しない動作 : result as {0}.", result);
		break;
	}

	return result;
}
