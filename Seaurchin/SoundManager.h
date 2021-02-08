#pragma once

#include <bass.h>
#include <bassmix.h>
#include <bass_fx.h>
#include <string>
#include <unordered_set>
#include <spdlog/spdlog.h>

class SoundManager;

enum class SoundType {
    Sample,
    Stream,
};

class Sound {
    friend class SoundManager;
public:
    virtual ~Sound() = default;
    SoundType Type;

    virtual DWORD GetSoundHandle() = 0;
    virtual void StopSound() = 0;
    virtual void SetVolume(double vol) = 0;
};

class SoundSample : public Sound {
    friend class SoundManager;

protected:
    HSAMPLE hSample;

public:
    explicit SoundSample(HSAMPLE sample);
    ~SoundSample();

    DWORD GetSoundHandle() override;
    void StopSound() override;
    void SetVolume(double vol) override;

    static SoundSample *CreateFromFile(const std::string &fileNameW, int maxChannels = 16);
    void SetLoop(bool looping) const;
};

class SoundStream : public Sound {
    friend class SoundManager;

protected:
    HSTREAM hStream;

public:
    explicit SoundStream(HSTREAM stream);
    ~SoundStream();

    DWORD GetSoundHandle() override;
    void StopSound() override;
    void SetVolume(double vol) override;

    void Pause() const;
    void Resume() const;

    static SoundStream *CreateFromFile(const std::wstring &fileNameW);
    double GetPlayingPosition() const;
    void SetPlayingPosition(double pos) const;
    DWORD GetStatus() const { return BASS_ChannelIsActive(hStream); }
};

class SoundMixerStream {
protected:
    HSTREAM hMixerStream;
    std::unordered_set<HCHANNEL> playingSounds;

public:
    SoundMixerStream(int ch, int freq);
    ~SoundMixerStream();

    void Update();
    void SetVolume(double vol) const;
    void Play(Sound *sound);
    static void Stop(Sound *sound);
};

class SoundManager final {
private:

public:
    SoundManager();
    ~SoundManager();

    static SoundMixerStream *CreateMixerStream();
    static void PlayGlobal(Sound *sound);
    static void StopGlobal(Sound *sound);
};
