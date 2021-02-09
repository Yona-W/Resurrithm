#include <boost/algorithm/clamp.hpp>
#include <boost/filesystem.hpp>
#include "Misc.h"
#include "SoundManager.h"

using namespace std;
using namespace boost::algorithm;
using namespace boost::filesystem;

// SoundSample ------------------------
SoundSample::SoundSample(const HSAMPLE sample)
{
    Type = SoundType::Sample;
    hSample = sample;
}

SoundSample::~SoundSample()
{
    if (hSample) BASS_SampleFree(hSample);
    hSample = 0;
}

DWORD SoundSample::GetSoundHandle()
{
    return BASS_SampleGetChannel(hSample, FALSE);
}

void SoundSample::StopSound()
{
    BASS_SampleStop(hSample);
}

void SoundSample::SetVolume(const double vol)
{
    BASS_SAMPLE si = { 0 };
    BASS_SampleGetInfo(hSample, &si);
    si.volume = SU_TO_FLOAT(vol);
    BASS_SampleSetInfo(hSample, &si);
}

SoundSample *SoundSample::CreateFromFile(const string &fileName, const int maxChannels)
{
    const auto handle = BASS_SampleLoad(FALSE, fileName.c_str(), 0, 0, maxChannels, BASS_SAMPLE_OVER_POS);
    int errorCode = BASS_ErrorGetCode();
    if(errorCode == 0){
        spdlog::info("Loaded audio sample {0}", fileName);
    }
    else
    {
        spdlog::warn("Error loading audio sample {0} - BASS error code {1}", fileName, errorCode);
    }
    const auto result = new SoundSample(handle);
    return result;
}

void SoundSample::SetLoop(const bool looping) const
{
    BASS_SAMPLE info;
    BASS_SampleGetInfo(hSample, &info);
    if (looping) {
        info.flags |= BASS_SAMPLE_LOOP;
    } else {
        info.flags = info.flags & ~BASS_SAMPLE_LOOP;
    }
    BASS_SampleSetInfo(hSample, &info);
}

// SoundStream ------------------------
SoundStream::SoundStream(const HSTREAM stream)
{
    Type = SoundType::Stream;
    hStream = stream;
}

SoundStream::~SoundStream()
{
    if (hStream) BASS_StreamFree(hStream);
    hStream = 0;
}

DWORD SoundStream::GetSoundHandle()
{
    return hStream;
}

void SoundStream::StopSound()
{
    BASS_ChannelStop(hStream);
}

void SoundStream::SetVolume(const double vol)
{
    BASS_ChannelSetAttribute(hStream, BASS_ATTRIB_VOL, SU_TO_FLOAT(clamp(vol, 0.0, 1.0)));
}

void SoundStream::Pause() const
{
    BASS_ChannelPause(hStream);
}

void SoundStream::Resume() const
{
    BASS_ChannelPlay(hStream, FALSE);
}

SoundStream *SoundStream::CreateFromFile(const string &fileName)
{
    const auto handle = BASS_StreamCreateFile(FALSE, fileName.c_str(), 0, 0, 0);
    int errorCode = BASS_ErrorGetCode();
    if(errorCode == 0){
        spdlog::info("Loaded audio stream {0}", fileName);
    }
    else
    {
        spdlog::warn("Error loading audio stream {0} - BASS error code {1}", fileName, errorCode);
    }
    const auto result = new SoundStream(handle);
    return result;
}

double SoundStream::GetPlayingPosition() const
{
    const auto pos = BASS_ChannelGetPosition(hStream, BASS_POS_BYTE);
    return BASS_ChannelBytes2Seconds(hStream, pos);
}

void SoundStream::SetPlayingPosition(const double pos) const
{
    const auto bp = BASS_ChannelSeconds2Bytes(hStream, pos);
    BASS_ChannelSetPosition(hStream, bp, BASS_POS_BYTE);
}

// SoundMixerStream ------------------------
SoundMixerStream::SoundMixerStream(const int ch, const int freq)
{
    hMixerStream = BASS_Mixer_StreamCreate(freq, ch, 0);
}

SoundMixerStream::~SoundMixerStream()
{
    if (hMixerStream) {
        for (auto &ch : playingSounds) BASS_Mixer_ChannelRemove(ch);
        BASS_StreamFree(hMixerStream);
        hMixerStream = 0;
    }
}

void SoundMixerStream::Update()
{
    if (!hMixerStream) return;

    auto snd = playingSounds.begin();
    while (snd != playingSounds.end()) {
        const auto state = BASS_ChannelIsActive(*snd);
        if (state != BASS_ACTIVE_STOPPED) {
            ++snd;
            continue;
        }
        BASS_Mixer_ChannelRemove(*snd);
        snd = playingSounds.erase(snd);
    }
}

void SoundMixerStream::Play(Sound * sound)
{
    auto ch = sound->GetSoundHandle();
    playingSounds.emplace(ch);
    BASS_Mixer_StreamAddChannel(hMixerStream, ch, 0);
    BASS_ChannelPlay(ch, FALSE);
}

void SoundMixerStream::Stop(Sound *sound)
{
    sound->StopSound();
    //チャンネル削除はUpdateに任せる
}

void SoundMixerStream::SetVolume(const double vol) const
{
    BASS_ChannelSetAttribute(hMixerStream, BASS_ATTRIB_VOL, SU_TO_FLOAT(vol));
}

// SoundManager -----------------------------
SoundManager::SoundManager()
{
    #ifdef _WIN32
    if (!BASS_Init(-1, 44100, 0, GetMainWindowHandle(), nullptr)) {
    #else
    if (!BASS_Init(-1, 44100, 0, nullptr, nullptr)) {
    #endif
        spdlog::critical(u8"BASS failed to initialize");
    }
    spdlog::info(u8"BASS initialized");
}

SoundManager::~SoundManager()
{
    BASS_Free();
}

SoundMixerStream *SoundManager::CreateMixerStream()
{
    return new SoundMixerStream(2, 44100);
}

void SoundManager::PlayGlobal(Sound *sound)
{
    BASS_ChannelPlay(sound->GetSoundHandle(), FALSE);
}

void SoundManager::StopGlobal(Sound *sound)
{
    sound->StopSound();
}
