#include "ScriptResource.h"
#include "ExecutionManager.h"
#include "Misc.h"

using namespace std;

SResource::SResource()
= default;

SResource::~SResource()
= default;

void SResource::AddRef()
{
    reference++;
}

void SResource::Release()
{
    if (--reference == 0) delete this;
}

// SImage ----------------------

SImage::SImage(SDL_Surface *ptr)
{
    surfacePtr = ptr;
    texturePtr = SDL_CreateTextureFromSurface(renderer, ptr);
}

SImage::~SImage()
{
    if (surfacePtr) SDL_FreeSurface(surfacePtr);
    if (texturePtr) SDL_DestroyTexture(texturePtr);
}

int SImage::GetWidth()
{
    return surfacePtr->w;
}

int SImage::GetHeight()
{
    return surfacePtr->h;
}

SImage * SImage::CreateBlankImage(int width, int height)
{
    auto result = new SImage(SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0));
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

SImage * SImage::CreateLoadedImageFromFile(const string &file, const bool async)
{
    auto result = new SImage(IMG_Load(file.c_str()));
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

SImage * SImage::CreateLoadedImageFromMemory(void * buffer, size_t size)
{
    auto rw = SDL_RWFromMem(buffer, size);
    auto result = new SImage(IMG_Load_RW(rw, true));
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

// SRenderTarget -----------------------------

SRenderTarget::SRenderTarget(const int width, const int height)
    : SImage(0)
{
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
}

SRenderTarget * SRenderTarget::CreateBlankTarget(const int width, const int height)
{
    auto result = new SRenderTarget(width, height);
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

/*
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
*/

// SAnimatedImage --------------------------------

SAnimatedImage::SAnimatedImage(const int w, const int h, const int count, const double time)
    : SImage(0)
{
    cellWidth = w;
    cellHeight = h;
    frameCount = count;
    secondsPerFrame = time;
}

SAnimatedImage::~SAnimatedImage()
{
    SDL_FreeSurface(surfacePtr);
    SDL_DestroyTexture(texturePtr);
}

SAnimatedImage * SAnimatedImage::CreateLoadedImageFromFile(const std::string & file, const int w, const int h, const int count, const double time)
{
    auto result = new SAnimatedImage(w, h, count, time);
    result->AddRef();

    result->surfacePtr = IMG_Load(file.c_str());
    result->texturePtr = SDL_CreateTextureFromSurface(renderer, result->surfacePtr);

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

SAnimatedImage * SAnimatedImage::CreateLoadedImageFromMemory(void * buffer, const size_t size, const int w, const int h, const int count, const double time)
{
    auto result = new SAnimatedImage(w, h, count, time);
    result->AddRef();

    auto rw = SDL_RWFromMem(buffer, size);
    result->surfacePtr = IMG_Load_RW(rw, true);
    result->texturePtr = SDL_CreateTextureFromSurface(renderer, result->surfacePtr);

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}


// SFont --------------------------------------

SFont::SFont()
= default;

SFont::~SFont()
{
    for (auto &i : glyphs) delete i.second;
    for (auto &i : Images) i->Release();
}

tuple<double, double, int> SFont::RenderRaw(SRenderTarget *rt, const string &utf8Str)
{
    uint32_t cx = 0, cy = 0;
    uint32_t mx = 0;
    auto line = 1;
    if (rt) {
        SDL_SetRenderTarget(renderer, rt->GetTexture());
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
    const auto *ccp = reinterpret_cast<const uint8_t*>(utf8Str.c_str());
    while (*ccp) {
        uint32_t gi = 0;
        if (*ccp >= 0xF0) {
            gi = (*ccp & 0x07) << 18 | (*(ccp + 1) & 0x3F) << 12 | (*(ccp + 2) & 0x3F) << 6 | (*(ccp + 3) & 0x3F);
            ccp += 4;
        } else if (*ccp >= 0xE0) {
            gi = (*ccp & 0x0F) << 12 | (*(ccp + 1) & 0x3F) << 6 | (*(ccp + 2) & 0x3F);
            ccp += 3;
        } else if (*ccp >= 0xC2) {
            gi = (*ccp & 0x1F) << 6 | (*(ccp + 1) & 0x3F);
            ccp += 2;
        } else {
            gi = *ccp & 0x7F;
            ccp++;
        }
        if (gi == 0x0A) {
            line++;
            mx = max(mx, cx);
            cx = 0;
            cy += size;
            continue;
        }
        const auto sg = glyphs[gi];
        if (!sg) continue;

        if (rt) {
            SDL_Rect srcRect = {sg->GlyphX, sg->GlyphY, sg->GlyphWidth, sg->GlyphHeight};
            SDL_Rect dstRect = {sg->BearX, sg->BearY, sg->GlyphWidth, sg->GlyphHeight};
            SDL_RenderCopy(renderer, Images[sg->ImageNumber]->GetTexture(), &srcRect, &dstRect);
        }
        
        /*
        DrawRectGraph(
            cx + sg->BearX, // DestX
            cy + sg->BearY, // DestY
            sg->GlyphX, // SrcX
            sg->GlyphY, // SrcY
            sg->GlyphWidth, // Src / Dst Width
            sg->GlyphHeight, // Src / Dst Height
            Images[sg->ImageNumber]->GetHandle(), // Image
            TRUE, // Transparency
            FALSE); // Invert?
        */
        cx += sg->WholeAdvance;
    }
    if (rt) {
        SDL_SetRenderTarget(renderer, NULL);
    }
    mx = max(mx, cx);
    double my = line * size;
    return make_tuple(mx, my, line);
}

tuple<double, double, int> SFont::RenderRich(SRenderTarget *rt, const string &utf8Str, const ColorTint &defcol)
{
    namespace bx = boost::xpressive;
    using namespace crc32_constexpr;

    const bx::sregex cmd = bx::bos >> "${" >> (bx::s1 = -+bx::_w) >> "}";
    const bx::sregex cmdhex = bx::bos >> "${#" >> (bx::s1 = bx::repeat<2, 2>(bx::xdigit)) >> (bx::s2 = bx::repeat<2, 2>(bx::xdigit)) >> (bx::s3 = bx::repeat<2, 2>(bx::xdigit)) >> "}";
    uint32_t cx = 0, cy = 0;
    uint32_t mx = 0;
    auto visible = true;
    auto line = 1;

    auto cr = defcol.R, cg = defcol.G, cb = defcol.B, ca = defcol.A;
    float cw = 1;

    if (rt) {
        SDL_SetRenderTarget(renderer, rt->GetTexture());
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, cr, cg, cb, ca);
        //SetDrawMode(DX_DRAWMODE_ANISOTROPIC); // Not sure how to translate this tbh
    }
    auto ccp = utf8Str.begin();
    bx::smatch match;
    while (ccp != utf8Str.end()) {
        boost::sub_range<const string> sr(ccp, utf8Str.end());
        if (bx::regex_search(sr, match, cmd)) {
            auto tcmd = match[1].str();
            switch (Crc32Rec(0xffffffff, tcmd.c_str())) {
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
                case "hide"_crc32:
                    visible = false;
                    break;
                default: break;
            }
            ccp += match[0].length();
            continue;
        }
        if (bx::regex_search(sr, match, cmdhex)) {
            cr = std::stoi(match[1].str(), nullptr, 16);
            cg = std::stoi(match[2].str(), nullptr, 16);
            cb = std::stoi(match[3].str(), nullptr, 16);
            ccp += match[0].length();
            continue;
        }

        uint32_t gi = 0;
        if (uint8_t(*ccp) >= 0xF0) {
            gi = (uint8_t(*ccp) & 0x07) << 18 | (uint8_t(*(ccp + 1)) & 0x3F) << 12 | (uint8_t(*(ccp + 2)) & 0x3F) << 6 | (uint8_t(*(ccp + 3)) & 0x3F);
            ccp += 4;
        } else if (uint8_t(*ccp) >= 0xE0) {
            gi = (uint8_t(*ccp) & 0x0F) << 12 | (uint8_t(*(ccp + 1)) & 0x3F) << 6 | (uint8_t(*(ccp + 2)) & 0x3F);
            ccp += 3;
        } else if (uint8_t(*ccp) >= 0xC2) {
            gi = (uint8_t(*ccp) & 0x1F) << 6 | (uint8_t(*(ccp + 1)) & 0x3F);
            ccp += 2;
        } else {
            gi = uint8_t(*ccp) & 0x7F;
            ++ccp;
        }
        if (!visible) continue;
        if (gi == 0x0A) {
            line++;
            mx = max(mx, cx);
            cx = 0;
            cy += size;
            continue;
        }
        const auto sg = glyphs[gi];
        if (!sg) continue;
        if (rt) {
            SDL_SetRenderDrawColor(renderer, cr, cg, cb, ca);
            SDL_Rect srcRect = {sg->GlyphX, sg->GlyphY, sg->GlyphWidth, sg->GlyphHeight};
            SDL_Rect dstRect = {sg->BearX, sg->BearY, sg->GlyphWidth, sg->GlyphHeight};
            SDL_RenderCopy(renderer, Images[sg->ImageNumber]->GetTexture(), &srcRect, &dstRect);
            cx += sg->WholeAdvance;
            SDL_SetRenderTarget(renderer, NULL);
        }
    }
    mx = max(mx, cx);
    double my = line * size;
    return make_tuple(mx, my, line);
}


SFont * SFont::CreateBlankFont()
{
    auto result = new SFont();
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

SFont * SFont::CreateLoadedFontFromFile(const string & file)
{
    auto result = new SFont();
    result->AddRef();
    ifstream font(file, ios::in | ios::binary);

    Sif2Header header;
    font.read(reinterpret_cast<char*>(&header), sizeof(Sif2Header));
    result->size = SU_TO_INT32(header.FontSize);

    for (auto i = 0u; i < header.Glyphs; i++) {
        const auto info = new Sif2Glyph();
        font.read(reinterpret_cast<char*>(info), sizeof(Sif2Glyph));
        result->glyphs[info->Codepoint] = info;
    }
    uint32_t size;
    for (auto i = 0; i < header.Images; i++) {
        font.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
        const auto pngdata = new uint8_t[size];
        font.read(reinterpret_cast<char*>(pngdata), size);
        result->Images.push_back(SImage::CreateLoadedImageFromMemory(pngdata, size));
        delete[] pngdata;
    }

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

// SSoundMixer ------------------------------

SSoundMixer::SSoundMixer(SoundMixerStream * mixer)
{
    this->mixer = mixer;
}

SSoundMixer::~SSoundMixer()
{
    delete mixer;
}

void SSoundMixer::Update() const
{
    mixer->Update();
}

void SSoundMixer::Play(SSound *sound) const
{
    if (!sound) return;

    mixer->Play(sound->sample);

    sound->Release();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void SSoundMixer::Stop(SSound *sound) const
{
    if (!sound) return;

    SoundMixerStream::Stop(sound->sample);

    sound->Release();
}

SSoundMixer * SSoundMixer::CreateMixer(SoundManager * manager)
{
    auto result = new SSoundMixer(SoundManager::CreateMixerStream());
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}


// SSound -----------------------------------
SSound::SSound(SoundSample *smp)
{
    sample = smp;
}

SSound::~SSound()
{
    delete sample;
}

void SSound::SetLoop(const bool looping) const
{
    sample->SetLoop(looping);
}

void SSound::SetVolume(const double vol) const
{
    sample->SetVolume(vol);
}

SSound * SSound::CreateSound(SoundManager *smanager)
{
    auto result = new SSound(nullptr);
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

SSound * SSound::CreateSoundFromFile(SoundManager *smanager, const std::string &file, const int simul)
{
    const auto hs = SoundSample::CreateFromFile(ConvertUTF8ToUnicode(file), simul);
    auto result = new SSound(hs);
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

// SSettingItem --------------------------------------------

SSettingItem::SSettingItem(const shared_ptr<setting2::SettingItem> s)
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


void RegisterScriptResource(ExecutionManager *exm)
{
    auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

    engine->RegisterObjectType(SU_IF_IMAGE, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_IMAGE, asBEHAVE_FACTORY, SU_IF_IMAGE "@ f()", asFUNCTION(SImage::CreateBlankImage), asCALL_CDECL);
    engine->RegisterObjectBehaviour(SU_IF_IMAGE, asBEHAVE_FACTORY, SU_IF_IMAGE "@ f(const string &in, bool = false)", asFUNCTION(SImage::CreateLoadedImageFromFile), asCALL_CDECL);
    engine->RegisterObjectBehaviour(SU_IF_IMAGE, asBEHAVE_ADDREF, "void f()", asMETHOD(SImage, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_IMAGE, asBEHAVE_RELEASE, "void f()", asMETHOD(SImage, Release), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_IMAGE, "int get_Width()", asMETHOD(SImage, GetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_IMAGE, "int get_Height()", asMETHOD(SImage, GetHeight), asCALL_THISCALL);
    //engine->RegisterObjectMethod(SU_IF_IMAGE, SU_IF_IMAGE "& opAssign(" SU_IF_IMAGE "&)", asFUNCTION(asAssign<SImage>), asCALL_CDECL_OBJFIRST);

    engine->RegisterObjectType(SU_IF_FONT, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_FONT, asBEHAVE_FACTORY, SU_IF_FONT "@ f()", asFUNCTION(SFont::CreateBlankFont), asCALL_CDECL);
    engine->RegisterObjectBehaviour(SU_IF_FONT, asBEHAVE_ADDREF, "void f()", asMETHOD(SFont, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_FONT, asBEHAVE_RELEASE, "void f()", asMETHOD(SFont, Release), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_FONT, "int get_Size()", asMETHOD(SFont, GetSize), asCALL_THISCALL);

    engine->RegisterObjectType(SU_IF_SOUND, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_SOUND, asBEHAVE_ADDREF, "void f()", asMETHOD(SSound, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_SOUND, asBEHAVE_RELEASE, "void f()", asMETHOD(SSound, Release), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SOUND, "void SetLoop(bool)", asMETHOD(SSound, SetLoop), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SOUND, "void SetVolume(double)", asMETHOD(SSound, SetVolume), asCALL_THISCALL);

    engine->RegisterObjectType(SU_IF_SOUNDMIXER, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_SOUNDMIXER, asBEHAVE_ADDREF, "void f()", asMETHOD(SSoundMixer, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_SOUNDMIXER, asBEHAVE_RELEASE, "void f()", asMETHOD(SSoundMixer, Release), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SOUNDMIXER, "void Play(" SU_IF_SOUND "@)", asMETHOD(SSoundMixer, Play), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SOUNDMIXER, "void Stop(" SU_IF_SOUND "@)", asMETHOD(SSoundMixer, Stop), asCALL_THISCALL);

    engine->RegisterObjectType(SU_IF_ANIMEIMAGE, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_ANIMEIMAGE, asBEHAVE_ADDREF, "void f()", asMETHOD(SAnimatedImage, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_ANIMEIMAGE, asBEHAVE_RELEASE, "void f()", asMETHOD(SAnimatedImage, Release), asCALL_THISCALL);

    engine->RegisterObjectType(SU_IF_SETTING_ITEM, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_SETTING_ITEM, asBEHAVE_ADDREF, "void f()", asMETHOD(SSettingItem, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_SETTING_ITEM, asBEHAVE_RELEASE, "void f()", asMETHOD(SSettingItem, Release), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "void Save()", asMETHOD(SSettingItem, Save), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "void MoveNext()", asMETHOD(SSettingItem, MoveNext), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "void MovePrevious()", asMETHOD(SSettingItem, MovePrevious), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "string GetItemText()", asMETHOD(SSettingItem, GetItemText), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SETTING_ITEM, "string GetDescription()", asMETHOD(SSettingItem, GetDescription), asCALL_THISCALL);
}
