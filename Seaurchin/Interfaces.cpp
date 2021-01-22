#include "Interfaces.h"

#include "ExecutionManager.h"
#include "ScriptFunction.h"

using namespace std;

void InterfacesRegisterSceneFunction(ExecutionManager *exm)
{
    auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

    engine->RegisterGlobalFunction("void YieldTime(double)", asFUNCTION(YieldTime), asCALL_CDECL);
    engine->RegisterGlobalFunction("void YieldFrame(int64)", asFUNCTION(YieldFrames), asCALL_CDECL);
}

void InterfacesRegisterGlobalFunction(ExecutionManager *exm)
{
    auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

    engine->RegisterGlobalFunction("void WriteLog(Severity, const string &in)", asFUNCTION(InterfacesWriteLog), asCALL_CDECL);
    engine->RegisterGlobalFunction(SU_IF_FONT "@ LoadSystemFont(const string & in)", asFUNCTION(LoadSystemFont), asCALL_CDECL);
    engine->RegisterGlobalFunction(SU_IF_IMAGE "@ LoadSystemImage(const string &in)", asFUNCTION(LoadSystemImage), asCALL_CDECL);
    engine->RegisterGlobalFunction("void CreateImageFont(const string &in, const string &in, int)", asFUNCTION(CreateImageFont), asCALL_CDECL);
}

void InterfacesRegisterEnum(ExecutionManager *exm)
{
    auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

    engine->RegisterEnum(SU_IF_KEY);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_BACK", KEY_BACK);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_TAB", KEY_TAB);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RETURN", KEY_ENTER);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LSHIFT", KEY_LEFTSHIFT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RSHIFT", KEY_RIGHTSHIFT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LCONTROL", KEY_LEFTCTRL);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RCONTROL", KEY_RIGHTCTRL);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_ESCAPE", KEY_ESC);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SPACE", KEY_SPACE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PGUP", KEY_PAGEUP);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PGDN", KEY_PAGEDOWN);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_END", KEY_END);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_HOME", KEY_HOME);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LEFT", KEY_LEFT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_UP", KEY_UP);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RIGHT", KEY_RIGHT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_DOWN", KEY_DOWN);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_INSERT", KEY_INSERT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_DELETE", KEY_DELETE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_MINUS", KEY_MINUS);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_YEN", KEY_YEN);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PREVTRACK", KEY_PREVIOUSSONG);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PERIOD", KEY_DOT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SLASH", KEY_SLASH);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LALT", KEY_LEFTALT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RALT", KEY_RIGHTALT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SCROLL", KEY_SCROLLLOCK);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SEMICOLON", KEY_SEMICOLON);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_COLON", KEY_SEMICOLON);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LBRACKET", KEY_LEFTBRACE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RBRACKET", KEY_RIGHTBRACE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_AT", KEY_PRINT); // IDK what the fuck this is supposed to be
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_BACKSLASH", KEY_BACKSLASH);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_COMMA", KEY_COMMA);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_KANJI", KEY_KATAKANAHIRAGANA); // man i have no clue
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_CONVERT", KEY_KATAKANAHIRAGANA);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NOCONVERT", KEY_KATAKANAHIRAGANA);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_KANA", KEY_KATAKANAHIRAGANA);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_APPS", KEY_APPSELECT); // what the fuck
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_CAPSLOCK", KEY_CAPSLOCK);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SYSRQ", KEY_SYSRQ);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PAUSE", KEY_PAUSE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LWIN", KEY_LEFTMETA);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RWIN", KEY_RIGHTMETA);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMLOCK", KEY_NUMLOCK);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD0", KEY_KP0);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD1", KEY_KP1);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD2", KEY_KP2);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD3", KEY_KP3);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD4", KEY_KP4);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD5", KEY_KP5);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD6", KEY_KP6);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD7", KEY_KP7);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD8", KEY_KP8);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD9", KEY_KP9);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_MULTIPLY", KEY_KPASTERISK);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_ADD", KEY_KPPLUS);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SUBTRACT", KEY_KPMINUS);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_DECIMAL", KEY_KPDOT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_DIVIDE", KEY_KPSLASH);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPADENTER", KEY_KPENTER);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F1", KEY_F1);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F2", KEY_F2);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F3", KEY_F3);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F4", KEY_F4);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F5", KEY_F5);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F6", KEY_F6);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F7", KEY_F7);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F8", KEY_F8);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F9", KEY_F9);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F10", KEY_F10);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F11", KEY_F11);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F12", KEY_F12);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_A", KEY_A);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_B", KEY_B);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_C", KEY_C);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_D", KEY_D);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_E", KEY_E);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F", KEY_F);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_G", KEY_G);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_H", KEY_H);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_I", KEY_I);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_J", KEY_J);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_K", KEY_K);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_L", KEY_L);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_M", KEY_M);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_N", KEY_N);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_O", KEY_O);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_P", KEY_P);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_Q", KEY_Q);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_R", KEY_R);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_S", KEY_S);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_T", KEY_T);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_U", KEY_U);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_V", KEY_V);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_W", KEY_W);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_X", KEY_X);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_Y", KEY_Y);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_Z", KEY_Z);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_0", KEY_0);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_1", KEY_1);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_2", KEY_2);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_3", KEY_3);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_4", KEY_4);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_5", KEY_5);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_6", KEY_6);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_7", KEY_7);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_8", KEY_8);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_9", KEY_9);

    engine->RegisterEnum(SU_IF_SEVERITY);
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Trace", int(ScriptLogSeverity::Trace));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Debug", int(ScriptLogSeverity::Debug));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Info", int(ScriptLogSeverity::Info));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Warn", int(ScriptLogSeverity::Warning));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Error", int(ScriptLogSeverity::Error));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Critical", int(ScriptLogSeverity::Critical));
}

// その他適当な関数

void InterfacesExitApplication()
{
    #ifdef _WIN32
    const auto hwnd = GetMainWindowHandle();
    PostMessage(hwnd, WM_CLOSE, 0, 0);
    #endif //_WIN32
}

void InterfacesWriteLog(ScriptLogSeverity severity, const string & message)
{
    auto log = spdlog::get("main");
    switch (severity) {
        case ScriptLogSeverity::Trace:
            log->trace(message);
            break;
        case ScriptLogSeverity::Debug:
            log->debug(message);
            break;
        case ScriptLogSeverity::Info:
            log->info(message);
            break;
        case ScriptLogSeverity::Warning:
            log->warn(message);
            break;
        case ScriptLogSeverity::Error:
            log->error(message);
            break;
        case ScriptLogSeverity::Critical:
            log->critical(message);
            break;
    }
}
