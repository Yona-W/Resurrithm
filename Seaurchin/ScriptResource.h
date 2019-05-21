#pragma once

#include "ScriptSpriteMisc.h"

#define SU_IF_IMAGE "Image"
#define SU_IF_FONT "Font"
#define SU_IF_FONT_TYPE "FontType"
#define SU_IF_RENDER "RenderTarget"
#define SU_IF_SOUND "Sound"
#define SU_IF_SOUND_STATE "SoundState"
#define SU_IF_ANIMEIMAGE "AnimatedImage"
#define SU_IF_SETTING_ITEM "SettingItem"

//リソース基底クラス
class SResource {
	INPLEMENT_REF_COUNTER

protected:
	int handle = 0;

public:
	SResource();
	virtual ~SResource();

	int GetHandle() const { return handle; }
};

//画像
class SImage : public SResource {
protected:
	int width = 0;
	int height = 0;

	void ObtainSize();
public:
	explicit SImage(int ih);
	~SImage() override;

	int GetWidth();
	int GetHeight();

	static SImage* CreateBlankImage();
	static SImage* CreateLoadedImageFromFile(const std::filesystem::path& file, bool async);
	static SImage* CreateLoadedImageFromFileName(const std::string& file, bool async);
	static SImage* CreateLoadedImageFromMemory(void* buffer, size_t size);
};

//描画タゲ
class SRenderTarget : public SImage {
public:
	SRenderTarget(int w, int h);

	static SRenderTarget* CreateBlankTarget(int w, int h);
};

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

//アニメーション用
class SAnimatedImage : public SImage {
protected:
	int cellWidth = 0;
	int cellHeight = 0;
	int frameCount = 0;
	double secondsPerFrame = 0.1;
	std::vector<int> images;

public:
	SAnimatedImage(int w, int h, int count, double time);
	~SAnimatedImage() override;

	double GetCellTime() const { return secondsPerFrame; }
	int GetFrameCount() const { return frameCount; }
	int GetImageHandleAt(const double time) { return images[int(time / secondsPerFrame) % frameCount]; }

	static SAnimatedImage* CreateLoadedImageFromFile(const std::filesystem::path& file, int xc, int yc, int w, int h, int count, double time, bool async);
	static SAnimatedImage* CreateLoadedImageFromFileName(const std::string& file, int xc, int yc, int w, int h, int count, double time, bool async);
	static SAnimatedImage* CreateLoadedImageFromMemory(void* buffer, size_t size, int xc, int yc, int w, int h, int count, double time);
};

//フォント
class SFont : public SResource {
protected:
	int size;
	int thick;
	int fontType;

public:
	SFont();
	~SFont() override;

	int GetSize() const { return size; }
	int GetThick() const { return thick; }
	int GetFontType() const { return fontType; }

	std::tuple<int, int> RenderRaw(SRenderTarget* rt, const std::string& utf8Str);
	std::tuple<int, int> RenderRich(SRenderTarget* rt, const std::string& utf8Str);

	static SFont* CreateLoadedFontFromFont(const std::string& name, int size, int thick, int fontType, bool async);
	static SFont* CreateLoadedFontFromMem(const void* mem, size_t memsize, int edge, int size, int thick, int fontType);
};

class SSound : public SResource {
public:
	enum class State {
		Stop,
		Play,
		Pause
	};

private:
	State state;
	bool isLoop;

public:
	SSound();
	~SSound() override;

	void SetLoop(bool looping) { isLoop = looping; }
	void SetVolume(double vol) { if (vol < 0.0) vol = 0.0; SetVolumeSoundMem(SU_TO_INT32(std::max(vol * 100, 10000.0)), handle); }
	void SetPosition(double ms);
	void Play();
	void Play(double ms) { SetPosition(ms); Play(); }
	void Pause() { StopSoundMem(handle); state = State::Pause; }
	void Stop() { StopSoundMem(handle); state = State::Stop; SetPosition(0.0); }
	State GetState();
	double GetPosition() const { return GetSoundCurrentTime(handle) / 1000.0; }

	static SSound* CreateSoundFromFile(const std::filesystem::path& file, bool async, int loadType = DX_SOUNDDATATYPE_MEMNOPRESS);
	static SSound* CreateSoundFromFileName(const std::string& file, bool async, int loadType = DX_SOUNDDATATYPE_MEMNOPRESS);
};

class SettingItem;
class SSettingItem : public SResource {
protected:
	std::shared_ptr<SettingItem> setting;

public:
	SSettingItem(std::shared_ptr<SettingItem> s);
	~SSettingItem() override;

	void Save() const;
	void MoveNext() const;
	void MovePrevious() const;
	std::string GetItemText() const;
	std::string GetDescription() const;
};

class ExecutionManager;
void RegisterScriptResource(ExecutionManager* exm);
