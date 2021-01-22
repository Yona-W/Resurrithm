#include "ScriptFunction.h"

#include "Setting.h"
#include "Config.h"
#include "Font.h"
#include "Misc.h"

using namespace std;
using namespace boost::filesystem;


#ifdef _WIN32

static int CALLBACK FontEnumerationProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD type, LPARAM lParam);

static int CALLBACK FontEnumerationProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD type, LPARAM lParam)
{
    return 0;
}

#endif //_WIN32

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

SImage* LoadSystemImage(const string &file)
{
    auto p = Setting::GetRootDirectory() / SU_DATA_DIR / SU_IMAGE_DIR / ConvertUTF8ToUnicode(file);
    return SImage::CreateLoadedImageFromFile(ConvertUnicodeToUTF8(p.wstring()), false);
}

SFont* LoadSystemFont(const std::string &file)
{
    auto p = Setting::GetRootDirectory() / SU_DATA_DIR / SU_FONT_DIR / (ConvertUTF8ToUnicode(file) + L".sif");
    return SFont::CreateLoadedFontFromFile(ConvertUnicodeToUTF8(p.wstring()));
}

SSound *LoadSystemSound(SoundManager *smng, const std::string & file)
{
    auto p = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SOUND_DIR / ConvertUTF8ToUnicode(file);
    return SSound::CreateSoundFromFile(smng, ConvertUnicodeToUTF8(p.wstring()), 4);
}

void CreateImageFont(const string &fileName, const string &saveName, const int size)
{
    Sif2CreatorOption option;
    option.FontPath = fileName;
    option.Size = SU_TO_FLOAT(size);
    option.ImageSize = 1024;
    option.TextSource = "";
    const auto op = Setting::GetRootDirectory() / SU_DATA_DIR / SU_FONT_DIR / (ConvertUTF8ToUnicode(saveName) + L".sif");

    Sif2Creator creator;
    creator.CreateSif2(option, op);
}

#ifdef _WIN32

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

#endif // _WIN32