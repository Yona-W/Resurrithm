#pragma once

#include "Font.h"
#include "SoundManager.h"
#include "Setting.h"
#include "ScriptSpriteMisc.h"

#include <SDL2/SDL_image.h>
#include <boost/range.hpp>

#define SU_IF_IMAGE "Image"
#define SU_IF_FONT "Font"
#define SU_IF_RENDER "RenderTarget"
#define SU_IF_SOUNDMIXER "SoundMixer"
#define SU_IF_SOUND "Sound"
#define SU_IF_ANIMEIMAGE "AnimatedImage"
#define SU_IF_SETTING_ITEM "SettingItem"

//リソース基底クラス
class SResource {
protected:
    int reference = 0;
public:
    SResource();
    virtual ~SResource();
    void AddRef();
    void Release();

    int GetRefCount() const { return reference; }
};

//画像
class SImage : public SResource {
protected:
    SDL_Surface *surfacePtr;
    SDL_Texture *texturePtr;
public:
    explicit SImage(SDL_Surface *surfacePtr);
    ~SImage() override;

    int GetWidth();
    int GetHeight();

    SDL_Surface *GetSurface(){return surfacePtr;}
    SDL_Texture *GetTexture(){return texturePtr;}

    static SImage* CreateBlankImage(int width, int height);
    static SImage* CreateLoadedImageFromFile(const std::string &file, bool async);
    static SImage* CreateLoadedImageFromMemory(void *buffer, size_t size);
};

//描画タゲ
class SRenderTarget : public SImage {
    private:
    SDL_Texture *texture;
    SDL_Renderer *renderer;
public:
    SRenderTarget(int w, int h);
    SDL_Texture *GetTexture();
    static SRenderTarget* CreateBlankTarget(int width, int height);
};

/*
//9patch描画用
class SNinePatchImage : public SImage {
protected:
    int leftSideWidth = 8;
    int topSideHeight = 8;
    int bodyWidth = 32;
    int bodyHeight = 32;

public:
    explicit SNinePatchImage(int ih);
    ~SNinePatchImage() override;
    void SetArea(int leftw, int toph, int bodyw, int bodyh);
    std::tuple<int, int, int, int> GetArea() { return std::make_tuple(leftSideWidth, topSideHeight, bodyWidth, bodyHeight); }
};
*/

//アニメーション用
class SAnimatedImage : public SImage {
protected:
    int cellWidth = 0;
    int cellHeight = 0;
    int frameCount = 0;
    double secondsPerFrame = 0.1;

    SDL_Texture *texturePtr;

public:
    SAnimatedImage(int w, int h, int count, double time);
    ~SAnimatedImage() override;

    double GetCellTime() const { return secondsPerFrame; }
    int GetFrameCount() const { return frameCount; }
    SDL_Rect GetRectAt(const double time) { return {int(time / secondsPerFrame) * cellWidth, 0, cellWidth, cellHeight}; }

    static SAnimatedImage *CreateLoadedImageFromFile(const std::string &file, int w, int h, int count, double time);
    static SAnimatedImage *CreateLoadedImageFromMemory(void *buffer, size_t size, int w, int h, int count, double time);
};

//フォント
class SFont : public SResource {
protected:
    int size = 0;
    std::unordered_map<uint32_t, Sif2Glyph*> glyphs;

public:
    std::vector<SImage*> Images;
    SFont();
    ~SFont() override;

    int GetSize() const { return size; }
    std::tuple<double, double, int> RenderRaw(SRenderTarget *rt, const std::string& utf8Str);
    std::tuple<double, double, int> RenderRich(SRenderTarget *rt, const std::string& utf8Str, const ColorTint &defcol);

    static SFont* CreateBlankFont();
    static SFont* CreateLoadedFontFromFile(const std::string &file);
};

class SSound : public SResource {
    friend class SSoundMixer;

protected:
    SoundSample *sample;

public:
    SSound(SoundSample *smp);
    ~SSound() override;

    SoundSample *GetSample() const { return sample; }
    void SetLoop(bool looping) const;
    void SetVolume(double vol) const;

    static SSound* CreateSound(SoundManager *smanager);
    static SSound* CreateSoundFromFile(SoundManager *smanager, const std::string &file, int simul);
};

class SSoundMixer : public SResource {
protected:
    SoundMixerStream *mixer;

public:
    SSoundMixer(SoundMixerStream *mixer);
    ~SSoundMixer() override;

    void Update() const;
    void Play(SSound *sound) const;
    void Stop(SSound *sound) const;

    static SSoundMixer *CreateMixer(SoundManager *manager);
};

class SSettingItem : public SResource {
protected:
    std::shared_ptr<setting2::SettingItem> setting;

public:
    SSettingItem(std::shared_ptr<setting2::SettingItem> s);
    ~SSettingItem() override;

    void Save() const;
    void MoveNext() const;
    void MovePrevious() const;
    std::string GetItemText() const;
    std::string GetDescription() const;
};

class ExecutionManager;
void RegisterScriptResource(ExecutionManager *exm);
