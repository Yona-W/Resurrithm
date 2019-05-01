#include "SkinHolder.h"
#include "AngelScriptManager.h"
#include "SoundManager.h"
#include "ScriptResource.h"
#include "SettingManager.h"


using namespace std;
using namespace std::filesystem;

SkinHolder::SkinHolder(const wstring& name, const shared_ptr<AngelScript>& script)
	: scriptInterface(script)
	, skinRoot(SettingManager::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR / name)
{}

SkinHolder::~SkinHolder()
= default;

bool SkinHolder::Initialize()
{
	auto log = spdlog::get("main");

	const bool forceReload = true;
	const auto mod = scriptInterface->GetModule(skinRoot, SU_SKIN_MAIN_FILE, forceReload);
	if (!mod) return false;

	asIScriptFunction* ep = scriptInterface->GetEntryPointAsFunction(mod);
	if (!ep) {
		log->critical(u8"スキンにEntryPointがありません");
		mod->Discard();
		return false;
	}

	auto ctx = scriptInterface->GetEngine()->CreateContext();
	ctx->Prepare(ep);
	ctx->SetArgObject(0, this);
	ctx->Execute();
	ctx->Release();
	mod->Discard();

	return true;
}

void SkinHolder::Terminate()
{
	for (const auto& it : images) BOOST_ASSERT(it.second->GetRefCount() == 1);
	for (const auto& it : sounds) BOOST_ASSERT(it.second->GetRefCount() == 1);
	for (const auto& it : fonts) BOOST_ASSERT(it.second->GetRefCount() == 1);
	for (const auto& it : animatedImages) BOOST_ASSERT(it.second->GetRefCount() == 1);

	for (const auto& it : images) it.second->Release();
	for (const auto& it : sounds) it.second->Release();
	for (const auto& it : fonts) it.second->Release();
	for (const auto& it : animatedImages) it.second->Release();
}

asIScriptObject * SkinHolder::ExecuteSkinScript(const path& file, const bool forceReload)
{
	auto log = spdlog::get("main");

	const auto obj = scriptInterface->ExecuteScript(skinRoot, SU_SCRIPT_DIR / file, forceReload);
	if (!obj) return nullptr;

	obj->SetUserData(this, SU_UDTYPE_SKIN);
	return obj;
}

void SkinHolder::LoadSkinImage(const string & key, const string & filename)
{
	if (images[key]) images[key]->Release();
	images[key] = SImage::CreateLoadedImageFromFile(ConvertUnicodeToUTF8((skinRoot / SU_IMAGE_DIR / ConvertUTF8ToUnicode(filename)).wstring()), false);
}

void SkinHolder::LoadSkinImageFromMem(const string & key, void* buffer, const size_t size)
{
	if (images[key]) images[key]->Release();
	images[key] = SImage::CreateLoadedImageFromMemory(buffer, size);
}

void SkinHolder::LoadSkinFont(const string & key, const string & filename)
{
	if (fonts[key]) fonts[key]->Release();
	fonts[key] = SFont::CreateLoadedFontFromFile(ConvertUnicodeToUTF8((skinRoot / SU_FONT_DIR / ConvertUTF8ToUnicode(filename)).wstring()));
}

void SkinHolder::LoadSkinFontFromMem(const string & key, void* buffer, const size_t size)
{
	//if (fonts[key]) fonts[key]->Release();
	//images[key] = SFont::CreateLoadedFontFromMemory(buffer, size);
}

void SkinHolder::LoadSkinSound(const std::string & key, const std::string & filename)
{
	if (sounds[key]) sounds[key]->Release();
	sounds[key] = SSound::CreateSoundFromFile(ConvertUnicodeToUTF8((skinRoot / SU_SOUND_DIR / ConvertUTF8ToUnicode(filename)).wstring()), 1);
}

void SkinHolder::LoadSkinSoundFromMem(const string & key, const void* buffer, const size_t size)
{
	//if (sounds[key]) sounds[key]->Release();
	//images[key] = SSound::CreateLoadedSoundFromMemory(buffer, size, 1);
}

void SkinHolder::LoadSkinAnime(const std::string & key, const std::string & filename, const int x, const int y, const int w, const int h, const int c, const double time)
{
	if (animatedImages[key]) animatedImages[key]->Release();
	animatedImages[key] = SAnimatedImage::CreateLoadedImageFromFile(ConvertUnicodeToUTF8((skinRoot / SU_IMAGE_DIR / ConvertUTF8ToUnicode(filename)).wstring()), x, y, w, h, c, time);
}

void SkinHolder::LoadSkinAnimeFromMem(const string & key, void* buffer, const size_t size, const int x, const int y, const int w, const int h, const int c, const double time)
{
	if (animatedImages[key]) animatedImages[key]->Release();
	images[key] = SAnimatedImage::CreateLoadedImageFromMemory(buffer, size, x, y, w, h, c, time);
}

SImage* SkinHolder::GetSkinImage(const string & key)
{
	auto it = images.find(key);
	if (it == images.end()) return nullptr;
	it->second->AddRef();
	return it->second;
}

SFont* SkinHolder::GetSkinFont(const string & key)
{
	auto it = fonts.find(key);
	if (it == fonts.end()) return nullptr;
	it->second->AddRef();
	return it->second;
}

SSound* SkinHolder::GetSkinSound(const std::string & key)
{
	auto it = sounds.find(key);
	if (it == sounds.end()) return nullptr;
	it->second->AddRef();
	return it->second;
}

SAnimatedImage* SkinHolder::GetSkinAnime(const std::string & key)
{
	auto it = animatedImages.find(key);
	if (it == animatedImages.end()) return nullptr;
	it->second->AddRef();
	return it->second;
}

void SkinHolder::RegisterType(asIScriptEngine* engine)
{
#ifdef  _WIN64
	engine->RegisterTypedef(SU_IF_SIZE, "uint64");
	engine->RegisterTypedef(SU_IF_VOID_PTR, "uint64");
#else
	engine->RegisterTypedef(SU_IF_SIZE, "uint32");
	engine->RegisterTypedef(SU_IF_VOID_PTR, "uint32");
#endif

	engine->RegisterObjectType(SU_IF_SKIN, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadImage(const string &in, const string &in)", asMETHOD(SkinHolder, LoadSkinImage), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadImageFromMem(const string &in, " SU_IF_VOID_PTR ", " SU_IF_SIZE ")", asMETHOD(SkinHolder, LoadSkinImageFromMem), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadFont(const string &in, const string &in)", asMETHOD(SkinHolder, LoadSkinFont), asCALL_THISCALL);
	//engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadFontFromMem(const string &in, " SU_IF_VOID_PTR ", " SU_IF_SIZE ")", asMETHOD(SkinHolder, LoadSkinFontFromMem), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadSound(const string &in, const string &in)", asMETHOD(SkinHolder, LoadSkinSound), asCALL_THISCALL);
	//engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadSoundFromMem(const string &in, " SU_IF_VOID_PTR ", " SU_IF_SIZE ")", asMETHOD(SkinHolder, LoadSkinSoundFromMem), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadAnime(const string &in, const string &in, int, int, int, int, int, double)", asMETHOD(SkinHolder, LoadSkinAnime), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "void LoadAnimeFromMem(const string &in, " SU_IF_VOID_PTR ", " SU_IF_SIZE ", const string &in, int, int, int, int, int, double)", asMETHOD(SkinHolder, LoadSkinAnimeFromMem), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, SU_IF_IMAGE "@ GetImage(const string &in)", asMETHOD(SkinHolder, GetSkinImage), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, SU_IF_FONT "@ GetFont(const string &in)", asMETHOD(SkinHolder, GetSkinFont), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, SU_IF_SOUND "@ GetSound(const string &in)", asMETHOD(SkinHolder, GetSkinSound), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, SU_IF_ANIMEIMAGE "@ GetAnime(const string &in)", asMETHOD(SkinHolder, GetSkinAnime), asCALL_THISCALL);

	engine->RegisterGlobalFunction(SU_IF_SKIN "@ GetSkin()", asFUNCTION(GetSkinObject), asCALL_CDECL);
}

//スキン専用
SkinHolder* GetSkinObject()
{
	auto ctx = asGetActiveContext();
	const auto obj = static_cast<asIScriptObject*>(ctx->GetThisPointer());
	if (!obj) {
		ScriptSceneWarnOutOf("GetSkin", "Instance Method", ctx);
		return nullptr;
	}
	const auto skin = obj->GetUserData(SU_UDTYPE_SKIN);
	if (!skin) {
		ScriptSceneWarnOutOf("GetSkin", "Skin-Related Scene", ctx);
		return nullptr;
	}
	return static_cast<SkinHolder*>(skin);
}
