#include "ScriptFunction.h"

#include "SettingManager.h"

using namespace std;
using namespace std::filesystem;


static int CALLBACK FontEnumerationProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD type, LPARAM lParam);

// ReSharper disable once CppParameterNeverUsed
static int CALLBACK FontEnumerationProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD type, LPARAM lParam)
{
	return 0;
}

void YieldTime(const double time)
{
	auto ctx = asGetActiveContext();
	auto pcw = static_cast<CoroutineWait*>(ctx->GetUserData(SU_UDTYPE_WAIT));
	if (!pcw) {
		ScriptSceneWarnOutOf("YieldTime", "Coroutine Scene Class or Coroutine", ctx);
		return;
	}
	pcw->Type = WaitType::Time;
	pcw->Time = time;
	ctx->Suspend();
}

void YieldFrames(const int64_t frames)
{
	auto ctx = asGetActiveContext();
	auto pcw = static_cast<CoroutineWait*>(ctx->GetUserData(SU_UDTYPE_WAIT));
	if (!pcw) {
		ScriptSceneWarnOutOf("YieldFrame", "Coroutine Scene Class or Coroutine", ctx);
		return;
	}
	pcw->Type = WaitType::Frame;
	pcw->Frames = frames;
	ctx->Suspend();
}

SImage* LoadSystemImage(const string& file)
{
	auto p = SettingManager::GetRootDirectory() / SU_DATA_DIR / SU_IMAGE_DIR / ConvertUTF8ToUnicode(file);
	return SImage::CreateLoadedImageFromFile(ConvertUnicodeToUTF8(p.wstring()), false);
}

SFont* LoadSystemFont(const std::string& file)
{
	return nullptr;
}

SSound * LoadSystemSound(const std::string & file)
{
	auto p = SettingManager::GetRootDirectory() / SU_DATA_DIR / SU_SOUND_DIR / ConvertUTF8ToUnicode(file);
	return SSound::CreateSoundFromFile(ConvertUnicodeToUTF8(p.wstring()), 4);
}

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
