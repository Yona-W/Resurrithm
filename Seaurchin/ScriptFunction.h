#pragma once

#include "ScriptResource.h"
#include "SoundManager.h"
#include <stdint.h>

enum class WaitType {
    Frame,
    Time,
};

struct CoroutineWait {
    WaitType Type;
    union {
        double Time;
        int64_t Frames;
    };

    bool Tick(const double delta)
    {
        switch (Type) {
            case WaitType::Frame:
                if (Frames > 0) --Frames;
                return Frames > 0;
            case WaitType::Time:
                if (Time > 0.0) Time -= delta;
                return Time > 0.0;
            default:
                spdlog::get("main")->critical(u8"CoroutineWaitのステータスが不正です");
                abort();
        }
    }
};

void YieldTime(double time);
void YieldFrames(int64_t frames);
SImage *LoadSystemImage(const std::string & file);
SFont *LoadSystemFont(const std::string & file);
SSound *LoadSystemSound(SoundManager *smng, const std::string & file);
void CreateImageFont(const std::string & fileName, const std::string & saveName, int size);

#ifdef _WIN32
void EnumerateInstalledFonts();
#endif
