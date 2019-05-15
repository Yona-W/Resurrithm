#include "ScriptResource.h"
#include "ExecutionManager.h"
#include "AngelScriptManager.h"

using namespace std;
using namespace filesystem;

SResource::SResource()
= default;

SResource::~SResource()
= default;

// SImage ----------------------

void SImage::ObtainSize()
{
	GetGraphSize(handle, &width, &height);
}

SImage::SImage(const int ih)
{
	handle = ih;
}

SImage::~SImage()
{
	if (handle) DeleteGraph(handle);
	handle = 0;
}

int SImage::GetWidth()
{
	if (!width) ObtainSize();
	return width;
}

int SImage::GetHeight()
{
	if (!height) ObtainSize();
	return height;
}

SImage* SImage::CreateBlankImage()
{
	auto result = new SImage(0);
	result->AddRef();

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

SImage* SImage::CreateLoadedImageFromFile(const path & file, const bool async)
{
	if (!is_regular_file(file) || !exists(file)) return nullptr;

	if (async) SetUseASyncLoadFlag(TRUE);
	auto result = new SImage(LoadGraph(ConvertUnicodeToUTF8(file).c_str()));
	if (async) SetUseASyncLoadFlag(FALSE);
	result->AddRef();

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

SImage* SImage::CreateLoadedImageFromFileName(const string& file, const bool async)
{
	const path path = ConvertUTF8ToUnicode(file);
	if (!is_regular_file(path) || !exists(path)) return nullptr;

	if (async) SetUseASyncLoadFlag(TRUE);
	auto result = new SImage(LoadGraph(file.c_str()));
	if (async) SetUseASyncLoadFlag(FALSE);
	result->AddRef();

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

SImage* SImage::CreateLoadedImageFromMemory(void* buffer, const size_t size)
{
	auto result = new SImage(CreateGraphFromMem(buffer, size));
	result->AddRef();

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

// SRenderTarget -----------------------------

SRenderTarget::SRenderTarget(const int w, const int h)
	: SImage(0)
{
	width = w;
	height = h;
	if (w * h) handle = MakeScreen(w, h, TRUE);
}

SRenderTarget* SRenderTarget::CreateBlankTarget(const int w, const int h)
{
	auto result = new SRenderTarget(w, h);
	result->AddRef();

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

// SNinePatchImage ----------------------------
SNinePatchImage::SNinePatchImage(const int ih)
	: SImage(ih)
{}

SNinePatchImage::~SNinePatchImage()
{
	DeleteGraph(handle);
	handle = 0;
	leftSideWidth = topSideHeight = bodyWidth = bodyHeight = 0;
}

void SNinePatchImage::SetArea(const int leftw, const int toph, const int bodyw, const int bodyh)
{
	leftSideWidth = leftw;
	topSideHeight = toph;
	bodyWidth = bodyw;
	bodyHeight = bodyh;
}

// SAnimatedImage --------------------------------

SAnimatedImage::SAnimatedImage(const int w, const int h, const int count, const double time)
	: SImage(0)
{
	cellWidth = width = w;
	cellHeight = height = h;
	frameCount = count;
	secondsPerFrame = time;
}

SAnimatedImage::~SAnimatedImage()
{
	for (auto& img : images) DeleteGraph(img);
}

SAnimatedImage* SAnimatedImage::CreateLoadedImageFromFile(const path & file, const int xc, const int yc, const int w, const int h, const int count, const double time, bool async)
{
	if (!is_regular_file(file) || !exists(file)) return nullptr;

	auto result = new SAnimatedImage(w, h, count, time);
	result->AddRef();

	result->images.resize(count);
	if (async) SetUseASyncLoadFlag(TRUE);
	LoadDivGraph(ConvertUnicodeToUTF8(file).c_str(), count, xc, yc, w, h, result->images.data());
	if (async) SetUseASyncLoadFlag(FALSE);

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

SAnimatedImage* SAnimatedImage::CreateLoadedImageFromFileName(const string& file, const int xc, const int yc, const int w, const int h, const int count, const double time, bool async)
{
	const path path = ConvertUTF8ToUnicode(file);
	if (!is_regular_file(path) || !exists(path)) return nullptr;

	auto result = new SAnimatedImage(w, h, count, time);
	result->AddRef();

	result->images.resize(count);
	if (async) SetUseASyncLoadFlag(TRUE);
	LoadDivGraph(file.c_str(), count, xc, yc, w, h, result->images.data());
	if (async) SetUseASyncLoadFlag(FALSE);

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

SAnimatedImage* SAnimatedImage::CreateLoadedImageFromMemory(void* buffer, const size_t size, const int xc, const int yc, const int w, const int h, const int count, const double time)
{
	auto result = new SAnimatedImage(w, h, count, time);
	result->AddRef();

	result->images.resize(count);
	CreateDivGraphFromMem(buffer, size, count, xc, yc, w, h, result->images.data());

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}


// SFont --------------------------------------

SFont::SFont()
	: size(0)
	, thick(0)
	, fontType(0)
{}

SFont::~SFont()
{
	if (handle) DeleteFontToHandle(handle);
	handle = 0;
}

tuple<int, int> SFont::RenderRaw(SRenderTarget * rt, const string & utf8Str)
{
	const TCHAR* str = utf8Str.c_str();
	if (rt) {
		BEGIN_DRAW_TRANSACTION(rt->GetHandle());
		ClearDrawScreen();
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		SetDrawBright(255, 255, 255);
		DrawStringToHandle(0, 0, str, GetColor(255, 255, 255), handle);
		FINISH_DRAW_TRANSACTION;
	}

	int sx = 0, sy = 0, lc = 0;
	GetDrawStringSizeToHandle(&sx, &sy, &lc, str, strlen(str), handle);
	return make_tuple(sx, sy);
}

tuple<int, int> SFont::RenderRich(SRenderTarget * rt, const string & utf8Str)
{
	using namespace crc32_constexpr;

	const std::regex cmd(R"(\$\{(?:([\w]+)|#([0-9A-Fa-f]{2,2})([0-9A-Fa-f]{2,2})([0-9A-Fa-f]{2,2}))\})");
	int cx = 0, cy = 0;
	int mx = 0, my = 0;

	ColorTint defcol = { 255, 255, 255, 255 };
	uint8_t cr = 255, cg = 255, cb = 255;
	float cw = 1;
	bool visible = true;

	if (rt) {
		BEGIN_DRAW_TRANSACTION(rt->GetHandle());
		ClearDrawScreen();
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		SetDrawBright(255, 255, 255);
	}

	auto ccp = utf8Str.begin(); // 走査対象の先頭
	std::smatch match;
	while (ccp != utf8Str.end()) {
		const bool ret = std::regex_search(ccp, utf8Str.end(), match, cmd);

		const auto head = ccp;												// 描画対象の先頭
		const auto tail = (ret) ? ccp + match.position() : utf8Str.end();	// 描画対象の終端

		if (visible && head != tail) {
			auto begin = head;
			const auto end = tail;

			auto cur = begin;
			while (cur != end) {
				if (*cur != '\n') {
					if (++cur != end) continue;
				}

				if (begin != cur) {
					auto str = string(begin, cur);
					auto cstr = str.c_str();
					auto lstr = strlen(cstr);
					if (rt) {
						DrawExtendStringToHandle(cx, my, cw, 1.0, cstr, GetColor(cr, cg, cb), handle);
					}

					int sx = 0, sy = 0;
					GetDrawExtendStringSizeToHandle(&sx, &sy, nullptr, cw, 1.0, cstr, lstr, handle);
					cx += sx;
					cy = max(cy, sy);
				}

				if (cur == end) break;
				if (*cur == '\n') {
					mx = max(mx, cx);
					my += cy;
					cx = 0;
					cy = 0;
				}
				++cur;
				begin = cur;
			}
		}

		if (ret) {
			if (match[1].matched) {
				switch (Crc32Rec(0xffffffff, match[1].str().c_str())) {
				case "reset"_crc32:
					cr = defcol.R;
					cg = defcol.G;
					cb = defcol.B;
					cw = 1;
					visible = true;
					break;
				case "red"_crc32:
					cr = 255;
					cg = cb = 0;
					break;
				case "green"_crc32:
					cg = 255;
					cr = cb = 0;
					break;
				case "blue"_crc32:
					cb = 255;
					cr = cg = 0;
					break;
				case "magenta"_crc32:
					cr = cb = 255;
					cg = 0;
					break;
				case "cyan"_crc32:
					cg = cb = 255;
					cr = 0;
					break;
				case "yellow"_crc32:
					cr = cg = 255;
					cb = 0;
					break;
				case "defcolor"_crc32:
					cr = defcol.R;
					cg = defcol.G;
					cb = defcol.B;
					break;
				case "bold"_crc32:
					cw = 1.2f;
					break;
				case "normal"_crc32:
					cw = 1.0f;
					break;
				case "visible"_crc32:
					visible = true;
					break;
				case "hidden"_crc32:
					visible = false;
					break;
				default: break;
				}
			}
			else {
				cr = std::stoi(match[2].str(), nullptr, 16);
				cg = std::stoi(match[3].str(), nullptr, 16);
				cb = std::stoi(match[4].str(), nullptr, 16);
			}
			ccp += match.position() + match[0].length();
		}
		else {
			ccp = tail;
		}
	}
	mx = max(mx, cx);
	my += cy;	// TODO: これ終端改行文字の場合どうしよう、仕様で良いか?
	if (rt) {
		FINISH_DRAW_TRANSACTION;
	}

	return make_tuple(mx, my);
}

SFont* SFont::CreateLoadedFontFromFont(const string& name, int size, int thick, int fontType, bool async)
{
	auto result = new SFont();
	result->AddRef();

	if (async) SetUseASyncLoadFlag(TRUE);
	result->handle = CreateFontToHandle(name.c_str(), size, thick, fontType);
	if (async) SetUseASyncLoadFlag(FALSE);
	result->size = size;
	result->thick = thick;
	result->fontType = fontType;

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

SFont* SFont::CreateLoadedFontFromMem(const void *mem, size_t memsize, int edge, int size, int thick, int fontType)
{
	auto result = new SFont();
	result->AddRef();

	int handle = LoadFontDataFromMemToHandle(mem, memsize, edge);
	result->handle = handle;
	result->size = size;
	result->thick = thick;
	result->fontType = fontType;

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}


// SSound -----------------------------------
SSound::SSound()
	: state(State::Stop)
	, isLoop(false)
{
}

SSound::~SSound()
{
	Stop();
	if (handle) DeleteSoundMem(handle);
}

void SSound::SetPosition(double ms) {
	if (!handle) return;

	bool needPlay = GetState() == State::Play;
	if (needPlay) Pause();
	SetSoundCurrentTime(ms, handle);
	if (needPlay) Play();
}

void SSound::Play()
{
	if (GetState() == State::Play) {
		Stop();
		SetSoundCurrentTime(0, handle);
	}

	PlaySoundMem(handle, (isLoop) ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
	state = State::Play;
}

SSound::State SSound::GetState()
{
	if (!handle) return state;

	if (!CheckSoundMem(handle)) {
		switch (state)
		{
		case State::Stop: break;
		case State::Play: Stop(); break;
		case State::Pause: break;
		}
	}
	else {
		switch (state)
		{
		case State::Stop: SU_ASSERT(false); state = State::Play; break;
		case State::Play: break;
		case State::Pause: SU_ASSERT(false); state = State::Play; break;
		}
	}
	return state;
}

SSound* SSound::CreateSoundFromFile(const path& file, bool async, int loadType)
{
	if (!is_regular_file(file) || !exists(file)) return nullptr;

	SetCreateSoundDataType(loadType);
	if (async) SetUseASyncLoadFlag(TRUE);
	const auto handle = LoadSoundMem(ConvertUnicodeToUTF8(file).c_str(), 16);
	if (async) SetUseASyncLoadFlag(FALSE);

	if (handle == -1) return nullptr;

	auto result = new SSound();
	result->AddRef();
	result->handle = handle;

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

SSound* SSound::CreateSoundFromFileName(const string& file, bool async, int loadType)
{
	const path path = ConvertUTF8ToUnicode(file);
	if (!is_regular_file(path) || !exists(path)) return nullptr;

	auto result = new SSound();
	result->AddRef();

	SetCreateSoundDataType(loadType);
	if (async) SetUseASyncLoadFlag(TRUE);
	result->handle = LoadSoundMem(file.c_str(), 16);
	if (async) SetUseASyncLoadFlag(FALSE);
	if (result->handle == -1) {
		result->Release();
		return nullptr;
	}

	SU_ASSERT(result->GetRefCount() == 1);
	return result;
}

// SSettingItem --------------------------------------------

SSettingItem::SSettingItem(const shared_ptr<SettingItem> s)
	: setting(s)
{
}

SSettingItem::~SSettingItem()
{
	setting->SaveValue();
}

void SSettingItem::Save() const
{
	setting->SaveValue();
}

void SSettingItem::MoveNext() const
{
	setting->MoveNext();
}

void SSettingItem::MovePrevious() const
{
	setting->MovePrevious();
}

std::string SSettingItem::GetItemText() const
{
	return setting->GetItemString();
}

std::string SSettingItem::GetDescription() const
{
	return setting->GetDescription();
}


void RegisterScriptResource(ExecutionManager * exm)
{
	auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

	engine->RegisterObjectType(SU_IF_IMAGE, 0, asOBJ_REF);
	engine->RegisterObjectBehaviour(SU_IF_IMAGE, asBEHAVE_FACTORY, SU_IF_IMAGE "@ f(const string &in, bool = false)", asFUNCTION(SImage::CreateLoadedImageFromFileName), asCALL_CDECL);
	engine->RegisterObjectBehaviour(SU_IF_IMAGE, asBEHAVE_ADDREF, "void f()", asMETHOD(SImage, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_IMAGE, asBEHAVE_RELEASE, "void f()", asMETHOD(SImage, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_IMAGE, "int get_Width()", asMETHOD(SImage, GetWidth), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_IMAGE, "int get_Height()", asMETHOD(SImage, GetHeight), asCALL_THISCALL);
	//engine->RegisterObjectMethod(SU_IF_IMAGE, SU_IF_IMAGE "& opAssign(" SU_IF_IMAGE "&)", asFUNCTION(asAssign<SImage>), asCALL_CDECL_OBJFIRST);

	engine->RegisterEnum(SU_IF_FONT_TYPE);
	engine->RegisterEnumValue(SU_IF_FONT_TYPE, "Normal", DX_FONTTYPE_NORMAL);
	engine->RegisterEnumValue(SU_IF_FONT_TYPE, "Edge", DX_FONTTYPE_EDGE);
	engine->RegisterEnumValue(SU_IF_FONT_TYPE, "AntiAliasing", DX_FONTTYPE_ANTIALIASING);
	engine->RegisterEnumValue(SU_IF_FONT_TYPE, "AntiAliasingEdge", DX_FONTTYPE_ANTIALIASING_EDGE);

	engine->RegisterObjectType(SU_IF_FONT, 0, asOBJ_REF);
	engine->RegisterObjectBehaviour(SU_IF_FONT, asBEHAVE_FACTORY, SU_IF_FONT "@ f(const string &in, int, int = 1, " SU_IF_FONT_TYPE " = " SU_IF_FONT_TYPE "::Normal, bool = false)", asFUNCTION(SFont::CreateLoadedFontFromFont), asCALL_CDECL);
	engine->RegisterObjectBehaviour(SU_IF_FONT, asBEHAVE_ADDREF, "void f()", asMETHOD(SFont, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_FONT, asBEHAVE_RELEASE, "void f()", asMETHOD(SFont, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_FONT, "int get_Size()", asMETHOD(SFont, GetSize), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_FONT, "int get_Thick()", asMETHOD(SFont, GetThick), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_FONT, SU_IF_FONT_TYPE " get_Type()", asMETHOD(SFont, GetFontType), asCALL_THISCALL);

	engine->RegisterEnum(SU_IF_SOUND_STATE);
	engine->RegisterEnumValue(SU_IF_SOUND_STATE, "Stop", SU_TO_INT32(SSound::State::Stop));
	engine->RegisterEnumValue(SU_IF_SOUND_STATE, "Play", SU_TO_INT32(SSound::State::Play));
	engine->RegisterEnumValue(SU_IF_SOUND_STATE, "Pause", SU_TO_INT32(SSound::State::Pause));

	engine->RegisterObjectType(SU_IF_SOUND, 0, asOBJ_REF);
	engine->RegisterObjectBehaviour(SU_IF_SOUND, asBEHAVE_FACTORY, SU_IF_SOUND "@ f(const string &in, bool = false, int = 0)", asFUNCTION(SSound::CreateSoundFromFileName), asCALL_CDECL);
	engine->RegisterObjectBehaviour(SU_IF_SOUND, asBEHAVE_ADDREF, "void f()", asMETHOD(SSound, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_SOUND, asBEHAVE_RELEASE, "void f()", asMETHOD(SSound, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SOUND, "void SetLoop(bool)", asMETHOD(SSound, SetLoop), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SOUND, "void SetVolume(double)", asMETHOD(SSound, SetVolume), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SOUND, "void SetPosition(double)", asMETHOD(SSound, SetPosition), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SOUND, "void Play()", asMETHODPR(SSound, Play, (), void), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SOUND, "void Play(double)", asMETHODPR(SSound, Play, (double), void), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SOUND, "void Pause()", asMETHOD(SSound, Pause), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SOUND, SU_IF_SOUND_STATE " GetState()", asMETHOD(SSound, GetState), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SOUND, "double GetPosition()", asMETHOD(SSound, GetPosition), asCALL_THISCALL);

	// TODO: Imageを継承させる
	engine->RegisterObjectType(SU_IF_ANIMEIMAGE, 0, asOBJ_REF);
	engine->RegisterObjectBehaviour(SU_IF_ANIMEIMAGE, asBEHAVE_FACTORY, SU_IF_ANIMEIMAGE "@ f(const string &in, int, int, int, int, int, double, bool = false)", asFUNCTION(SAnimatedImage::CreateLoadedImageFromFileName), asCALL_CDECL);
	engine->RegisterObjectBehaviour(SU_IF_ANIMEIMAGE, asBEHAVE_ADDREF, "void f()", asMETHOD(SAnimatedImage, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_ANIMEIMAGE, asBEHAVE_RELEASE, "void f()", asMETHOD(SAnimatedImage, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_ANIMEIMAGE, "int get_CellTime()", asMETHOD(SAnimatedImage, GetCellTime), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_ANIMEIMAGE, "int get_Frame()", asMETHOD(SAnimatedImage, GetFrameCount), asCALL_THISCALL);

	engine->RegisterObjectType(SU_IF_SETTING_ITEM, 0, asOBJ_REF);
	engine->RegisterObjectBehaviour(SU_IF_SETTING_ITEM, asBEHAVE_ADDREF, "void f()", asMETHOD(SSettingItem, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_SETTING_ITEM, asBEHAVE_RELEASE, "void f()", asMETHOD(SSettingItem, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "void Save()", asMETHOD(SSettingItem, Save), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "void MoveNext()", asMETHOD(SSettingItem, MoveNext), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "void MovePrevious()", asMETHOD(SSettingItem, MovePrevious), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "string GetItemText()", asMETHOD(SSettingItem, GetItemText), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "string GetDescription()", asMETHOD(SSettingItem, GetDescription), asCALL_THISCALL);
}
