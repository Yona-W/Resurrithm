#include "SkinHolder.h"
#include "Setting.h"
#include "ExecutionManager.h"

using namespace std;
using namespace std::filesystem;

// ReSharper disable once CppParameterNeverUsed
bool SkinHolder::IncludeScript(std::wstring include, std::wstring from, CWScriptBuilder* builder)
{
	return false;
}

SkinHolder::SkinHolder(const wstring& name, const shared_ptr<AngelScript>& script, const std::shared_ptr<SoundManager>& sound)
	: scriptInterface(script)
	, soundInterface(sound)
	, skinName(name)
	, skinRoot(Setting::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR / skinName)
{}

SkinHolder::~SkinHolder()
= default;

void SkinHolder::Initialize()
{
	auto log = spdlog::get("main");
	scriptInterface->StartBuildModule("SkinLoader",
		[this](wstring inc, wstring from, CWScriptBuilder * b) {
			if (!exists(skinRoot / SU_SCRIPT_DIR / inc)) return false;
			b->AddSectionFromFile((skinRoot / SU_SCRIPT_DIR / inc).wstring().c_str());
			return true;
		});
	scriptInterface->LoadFile((skinRoot / SU_SKIN_MAIN_FILE).wstring());
	scriptInterface->FinishBuildModule();

	auto mod = scriptInterface->GetLastModule();
	const auto fc = mod->GetFunctionCount();
	asIScriptFunction * ep = nullptr;
	for (asUINT i = 0; i < fc; i++) {
		const auto func = mod->GetFunctionByIndex(i);
		if (!scriptInterface->CheckMetaData(func, "EntryPoint")) continue;
		ep = func;
		break;
	}
	if (!ep) {
		log->critical(u8"スキンにEntryPointがありません");
		mod->Discard();
		return;
	}

	auto ctx = scriptInterface->GetEngine()->CreateContext();
	ctx->Prepare(ep);
	ctx->SetArgObject(0, this);
	ctx->Execute();
	ctx->Release();
	mod->Discard();
}

void SkinHolder::Terminate()
{
	// ReSharper disable CppDeclaratorNeverUsed
	for (const auto& it : images) BOOST_ASSERT(it.second->GetRefCount() == 1);
	for (const auto& it : sounds) BOOST_ASSERT(it.second->GetRefCount() == 1);
	for (const auto& it : fonts) BOOST_ASSERT(it.second->GetRefCount() == 1);
	for (const auto& it : animatedImages) BOOST_ASSERT(it.second->GetRefCount() == 1);
	// ReSharper restore CppDeclaratorNeverUsed

	for (const auto& it : images) it.second->Release();
	for (const auto& it : sounds) it.second->Release();
	for (const auto& it : fonts) it.second->Release();
	for (const auto& it : animatedImages) it.second->Release();
}

asIScriptObject * SkinHolder::ExecuteSkinScript(const wstring & file, const bool forceReload)
{
	auto log = spdlog::get("main");
	//お茶を濁せ
	const auto modulename = ConvertUnicodeToUTF8(file);
	auto mod = scriptInterface->GetExistModule(modulename);
	if (!mod || forceReload) {
		scriptInterface->StartBuildModule(modulename,
			[this](wstring inc, wstring from, CWScriptBuilder * b) {
				if (!exists(skinRoot / SU_SCRIPT_DIR / inc)) return false;
				b->AddSectionFromFile((skinRoot / SU_SCRIPT_DIR / inc).wstring().c_str());
				return true;
			});
		scriptInterface->LoadFile((skinRoot / SU_SCRIPT_DIR / file).wstring());
		if (!scriptInterface->FinishBuildModule()) {
			scriptInterface->GetLastModule()->Discard();
			return nullptr;
		}
		mod = scriptInterface->GetLastModule();
	}

	//エントリポイント検索
	const int cnt = mod->GetObjectTypeCount();
	asITypeInfo * type = nullptr;
	for (auto i = 0; i < cnt; i++) {
		// ScriptBuilderのMetaDataのテーブルは毎回破棄されるので
		// asITypeInfoに情報を保持
		const auto cti = mod->GetObjectTypeByIndex(i);
		if (!(scriptInterface->CheckMetaData(cti, "EntryPoint") || cti->GetUserData(SU_UDTYPE_ENTRYPOINT))) continue;
		type = cti;
		type->SetUserData(reinterpret_cast<void*>(0xFFFFFFFF), SU_UDTYPE_ENTRYPOINT);
		break;
	}
	if (!type) {
		log->critical(u8"スキンにEntryPointがありません");
		return nullptr;
	}

	auto obj = scriptInterface->InstantiateObject(type);
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
	sounds[key] = SSound::CreateSoundFromFile(soundInterface.get(), ConvertUnicodeToUTF8((skinRoot / SU_SOUND_DIR / ConvertUTF8ToUnicode(filename)).wstring()), 1);
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

void RegisterScriptSkin(ExecutionManager * exm)
{
	auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

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
