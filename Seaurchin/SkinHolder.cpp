#include "SkinHolder.h"
#include "AngelScriptManager.h"
#include "SoundManager.h"
#include "ScriptResource.h"
#include "SettingManager.h"


using namespace std;
using namespace std::filesystem;

SkinHolder* SkinHolder::Create(const shared_ptr<AngelScript>& script, const wstring& name)
{
	const auto root = SettingManager::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR / name;
	if (!exists(root)) return nullptr;

	const auto ptr = new SkinHolder(script, root);
	ptr->Initialize();

	return ptr;
}

SkinHolder::SkinHolder(const shared_ptr<AngelScript>& script, const path& root)
	: scriptInterface(script)
	, skinRoot(root)
{}

SkinHolder::~SkinHolder()
= default;

bool SkinHolder::Initialize()
{
	auto log = spdlog::get("main");

	const bool forceReload = true;
	const auto func = scriptInterface->ExecuteScriptAsFunction(skinRoot, SU_SKIN_MAIN_FILE, forceReload);
	if (!func) return false;

	func->AddRef();
	const auto funcObj = FunctionObject::Create(func);
	if (!funcObj) return false;

	funcObj->Prepare();
	if (funcObj->SetArgument([this](auto p) { p->SetArgObject(0, this); return true; })) {
		if (funcObj->Execute() != asEXECUTION_FINISHED) {
		}
		funcObj->Unprepare();
	}
	delete funcObj;

	func->Release();
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

	const auto obj = scriptInterface->ExecuteScriptAsObject(skinRoot, SU_SCRIPT_DIR / file, forceReload);
	if (!obj) return nullptr;

	obj->SetUserData(this, SU_UDTYPE_SKIN);
	return obj;
}

bool SkinHolder::LoadSkinImage(const string & key, const string & filename)
{
	const auto it = images.find(key);
	if (it != images.end() && it->second) {
		it->second->Release();
		it->second = nullptr;
	}

	const auto ptr = SImage::CreateLoadedImageFromFile(skinRoot / SU_IMAGE_DIR / ConvertUTF8ToUnicode(filename), false);
	if (ptr) images[key] = ptr;
	return !!ptr;
}

bool SkinHolder::LoadSkinImageFromMem(const string & key, void* buffer, const size_t size)
{
	const auto it = images.find(key);
	if (it != images.end() && it->second) {
		it->second->Release();
		it->second = nullptr;
	}

	const auto ptr = SImage::CreateLoadedImageFromMemory(buffer, size);
	if (ptr) images[key] = ptr;
	return !!ptr;
}

bool SkinHolder::LoadSkinFont(const string & key, const string & filename, int size, int thick, int fontType)
{
	const auto it = fonts.find(key);
	if (it != fonts.end() && it->second) {
		it->second->Release();
		it->second = nullptr;
	}

	const auto ptr = SFont::CreateLoadedFontFromFont(filename, size, thick, fontType);
	if (ptr) fonts[key] = ptr;
	return !!ptr;
}

bool SkinHolder::LoadSkinFontFromMem(const string & key, void* buffer, const size_t size)
{
	const auto it = fonts.find(key);
	if (it != fonts.end() && it->second) {
		it->second->Release();
		it->second = nullptr;
	}

//	const auto ptr = SFont::CreateLoadedFontFromMemory(buffer, size);
	const auto ptr = nullptr;
	if (ptr) fonts[key] = ptr;
	return !!ptr;
}

bool SkinHolder::LoadSkinSound(const std::string & key, const std::string & filename)
{
	const auto it = sounds.find(key);
	if (it != sounds.end() && it->second) {
		it->second->Release();
		it->second = nullptr;
	}

	const auto ptr = SSound::CreateSoundFromFile(skinRoot / SU_SOUND_DIR / ConvertUTF8ToUnicode(filename), 1);
	if (ptr) sounds[key] = ptr;
	return !!ptr;
}

bool SkinHolder::LoadSkinSoundFromMem(const string & key, const void* buffer, const size_t size)
{
	const auto it = sounds.find(key);
	if (it != sounds.end() && it->second) {
		it->second->Release();
		it->second = nullptr;
	}

//	const auto ptr = SSound::CreateSoundFromMemory(buffer, size);
	const auto ptr = nullptr;
	if (ptr) sounds[key] = ptr;
	return !!ptr;
}

bool SkinHolder::LoadSkinAnime(const std::string & key, const std::string & filename, const int x, const int y, const int w, const int h, const int c, const double time)
{
	const auto it = animatedImages.find(key);
	if (it != animatedImages.end() && it->second) {
		it->second->Release();
		it->second = nullptr;
	}

	const auto ptr = SAnimatedImage::CreateLoadedImageFromFile(skinRoot / SU_IMAGE_DIR / ConvertUTF8ToUnicode(filename), x, y, w, h, c, time);
	if (ptr) animatedImages[key] = ptr;
	return !!ptr;
}

bool SkinHolder::LoadSkinAnimeFromMem(const string & key, void* buffer, const size_t size, const int x, const int y, const int w, const int h, const int c, const double time)
{
	const auto it = animatedImages.find(key);
	if (it != animatedImages.end() && it->second) {
		it->second->Release();
		it->second = nullptr;
	}

	const auto ptr = SAnimatedImage::CreateLoadedImageFromMemory(buffer, size, x, y, w, h, c, time);
	if (ptr) animatedImages[key] = ptr;
	return !!ptr;
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
#ifdef _WIN64
	engine->RegisterTypedef(SU_IF_SIZE, "uint64");
	engine->RegisterTypedef(SU_IF_VOID_PTR, "uint64");
#else
	engine->RegisterTypedef(SU_IF_SIZE, "uint32");
	engine->RegisterTypedef(SU_IF_VOID_PTR, "uint32");
#endif

	engine->RegisterObjectType(SU_IF_SKIN, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_SKIN, "bool LoadImage(const string &in, const string &in)", asMETHOD(SkinHolder, LoadSkinImage), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "bool LoadImageFromMem(const string &in, " SU_IF_VOID_PTR ", " SU_IF_SIZE ")", asMETHOD(SkinHolder, LoadSkinImageFromMem), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "bool LoadFont(const string &in, const string &in, int, int = 1, " SU_IF_FONT_TYPE " = " SU_IF_FONT_TYPE "::Normal)", asMETHOD(SkinHolder, LoadSkinFont), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "bool LoadFontFromMem(const string &in, " SU_IF_VOID_PTR ", " SU_IF_SIZE ")", asMETHOD(SkinHolder, LoadSkinFontFromMem), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "bool LoadSound(const string &in, const string &in)", asMETHOD(SkinHolder, LoadSkinSound), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "bool LoadSoundFromMem(const string &in, " SU_IF_VOID_PTR ", " SU_IF_SIZE ")", asMETHOD(SkinHolder, LoadSkinSoundFromMem), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "bool LoadAnime(const string &in, const string &in, int, int, int, int, int, double)", asMETHOD(SkinHolder, LoadSkinAnime), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKIN, "bool LoadAnimeFromMem(const string &in, " SU_IF_VOID_PTR ", " SU_IF_SIZE ", const string &in, int, int, int, int, int, double)", asMETHOD(SkinHolder, LoadSkinAnimeFromMem), asCALL_THISCALL);
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
