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
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_BACK", SDL_SCANCODE_AC_BACK);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_TAB", SDL_SCANCODE_TAB);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RETURN", SDL_SCANCODE_RETURN);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LSHIFT", SDL_SCANCODE_LSHIFT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RSHIFT", SDL_SCANCODE_RSHIFT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LCONTROL", SDL_SCANCODE_LCTRL);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RCONTROL", SDL_SCANCODE_RCTRL);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_ESCAPE", SDL_SCANCODE_ESCAPE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SPACE", SDL_SCANCODE_SPACE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PGUP", SDL_SCANCODE_PAGEUP);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PGDN", SDL_SCANCODE_PAGEDOWN);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_END", SDL_SCANCODE_END);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_HOME", SDL_SCANCODE_HOME);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LEFT", SDL_SCANCODE_LEFT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_UP", SDL_SCANCODE_UP);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RIGHT", SDL_SCANCODE_RIGHT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_DOWN", SDL_SCANCODE_DOWN);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_INSERT", SDL_SCANCODE_INSERT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_DELETE", SDL_SCANCODE_DELETE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_MINUS", SDL_SCANCODE_MINUS);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_YEN", SDL_SCANCODE_INTERNATIONAL3);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PREVTRACK", SDL_SCANCODE_AUDIOPREV);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PERIOD", SDL_SCANCODE_PERIOD);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SLASH", SDL_SCANCODE_SLASH);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LALT", SDL_SCANCODE_LALT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RALT", SDL_SCANCODE_RALT);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SCROLL", SDL_SCANCODE_SCROLLLOCK);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SEMICOLON", SDL_SCANCODE_SEMICOLON);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_COLON", SDL_SCANCODE_SEMICOLON);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LBRACKET", SDL_SCANCODE_LEFTBRACKET);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RBRACKET", SDL_SCANCODE_RIGHTBRACKET);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_AT", SDL_SCANCODE_KP_AT); // IDK what the fuck this is supposed to be
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_BACKSLASH", SDL_SCANCODE_BACKSLASH);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_COMMA", SDL_SCANCODE_COMMA);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_KANJI", SDL_SCANCODE_LANG2); // man i have no clue
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_CONVERT", SDL_SCANCODE_LANG4);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NOCONVERT", SDL_SCANCODE_LANG1);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_KANA", SDL_SCANCODE_LANG3);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_APPS", SDL_SCANCODE_APPLICATION); // what the fuck
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_CAPSLOCK", SDL_SCANCODE_CAPSLOCK);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SYSRQ", SDL_SCANCODE_SYSREQ);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_PAUSE", SDL_SCANCODE_PAUSE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_LWIN", SDL_SCANCODE_LGUI);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_RWIN", SDL_SCANCODE_RGUI);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMLOCK", SDL_SCANCODE_NUMLOCKCLEAR);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD0", SDL_SCANCODE_KP_0);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD1", SDL_SCANCODE_KP_1);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD2", SDL_SCANCODE_KP_2);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD3", SDL_SCANCODE_KP_3);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD4", SDL_SCANCODE_KP_4);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD5", SDL_SCANCODE_KP_5);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD6", SDL_SCANCODE_KP_6);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD7", SDL_SCANCODE_KP_7);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD8", SDL_SCANCODE_KP_8);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPAD9", SDL_SCANCODE_KP_9);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_MULTIPLY", SDL_SCANCODE_KP_MULTIPLY);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_ADD", SDL_SCANCODE_KP_PLUS);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_SUBTRACT", SDL_SCANCODE_KP_MINUS);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_DECIMAL", SDL_SCANCODE_KP_PERIOD);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_DIVIDE", SDL_SCANCODE_KP_DIVIDE);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_NUMPADENTER", SDL_SCANCODE_KP_ENTER);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F1", SDL_SCANCODE_F1);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F2", SDL_SCANCODE_F2);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F3", SDL_SCANCODE_F3);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F4", SDL_SCANCODE_F4);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F5", SDL_SCANCODE_F5);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F6", SDL_SCANCODE_F6);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F7", SDL_SCANCODE_F7);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F8", SDL_SCANCODE_F8);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F9", SDL_SCANCODE_F9);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F10", SDL_SCANCODE_F10);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F11", SDL_SCANCODE_F11);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F12", SDL_SCANCODE_F12);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_A", SDL_SCANCODE_A);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_B", SDL_SCANCODE_B);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_C", SDL_SCANCODE_C);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_D", SDL_SCANCODE_D);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_E", SDL_SCANCODE_E);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_F", SDL_SCANCODE_F);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_G", SDL_SCANCODE_G);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_H", SDL_SCANCODE_H);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_I", SDL_SCANCODE_I);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_J", SDL_SCANCODE_J);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_K", SDL_SCANCODE_K);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_L", SDL_SCANCODE_L);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_M", SDL_SCANCODE_M);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_N", SDL_SCANCODE_N);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_O", SDL_SCANCODE_O);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_P", SDL_SCANCODE_P);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_Q", SDL_SCANCODE_Q);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_R", SDL_SCANCODE_R);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_S", SDL_SCANCODE_S);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_T", SDL_SCANCODE_T);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_U", SDL_SCANCODE_U);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_V", SDL_SCANCODE_V);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_W", SDL_SCANCODE_W);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_X", SDL_SCANCODE_X);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_Y", SDL_SCANCODE_Y);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_Z", SDL_SCANCODE_Z);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_0", SDL_SCANCODE_0);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_1", SDL_SCANCODE_1);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_2", SDL_SCANCODE_2);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_3", SDL_SCANCODE_3);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_4", SDL_SCANCODE_4);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_5", SDL_SCANCODE_5);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_6", SDL_SCANCODE_6);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_7", SDL_SCANCODE_7);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_8", SDL_SCANCODE_8);
    engine->RegisterEnumValue(SU_IF_KEY, "INPUT_9", SDL_SCANCODE_9);

    engine->RegisterEnum(SU_IF_SEVERITY);
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Trace", int(ScriptLogSeverity::Trace));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Debug", int(ScriptLogSeverity::Debug));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Info", int(ScriptLogSeverity::Info));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Warn", int(ScriptLogSeverity::Warning));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Error", int(ScriptLogSeverity::Error));
    engine->RegisterEnumValue(SU_IF_SEVERITY, "Critical", int(ScriptLogSeverity::Critical));
}

// その他適当な関数

void InterfacesWriteLog(ScriptLogSeverity severity, const string & message)
{
    
    switch (severity) {
        case ScriptLogSeverity::Trace:
            spdlog::trace(message);
            break;
        case ScriptLogSeverity::Debug:
            spdlog::debug(message);
            break;
        case ScriptLogSeverity::Info:
            spdlog::info(message);
            break;
        case ScriptLogSeverity::Warning:
            spdlog::warn(message);
            break;
        case ScriptLogSeverity::Error:
            spdlog::error(message);
            break;
        case ScriptLogSeverity::Critical:
            spdlog::critical(message);
            break;
    }
}
