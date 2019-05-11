#pragma once

#define SU_IF_SKIN "Skin"
#define SU_IF_SIZE "Size"
#define SU_IF_VOID_PTR "Address"

class AngelScript;
class SImage;
class SFont;
class SSound;
class SAnimatedImage;

class SkinHolder final {
private:
	const std::shared_ptr<AngelScript> scriptInterface;
	const std::filesystem::path skinRoot;

	std::unordered_map<std::string, SImage*> images;
	std::unordered_map<std::string, SFont*> fonts;
	std::unordered_map<std::string, SSound*> sounds;
	std::unordered_map<std::string, SAnimatedImage*> animatedImages;

public:
	static SkinHolder* Create(const std::shared_ptr<AngelScript>& script, const std::wstring& name);

public:
	SkinHolder(const std::shared_ptr<AngelScript>& script, const std::filesystem::path& root);
	~SkinHolder();

	bool Initialize();
	void Terminate();

	asIScriptObject* ExecuteSkinScript(const std::filesystem::path& file, bool forceReload = false);

	bool LoadSkinImage(const std::string& key, const std::string& filename);
	bool LoadSkinImageFromMem(const std::string& key, void* buffer, size_t size);
	bool LoadSkinFont(const std::string& key, const std::string& filename, int font, int thick, int fontType);
	bool LoadSkinFontFromMem(const std::string& key, void* buffer, size_t size);
	bool LoadSkinSound(const std::string& key, const std::string& filename);
	bool LoadSkinSoundFromMem(const std::string& key, const void* buffer, size_t size);
	bool LoadSkinAnime(const std::string& key, const std::string& filename, int x, int y, int w, int h, int c, double time);
	bool LoadSkinAnimeFromMem(const std::string& key, void* buffer, size_t size, int x, int y, int w, int h, int c, double time);
	SImage* GetSkinImage(const std::string& key);
	SFont* GetSkinFont(const std::string& key);
	SSound* GetSkinSound(const std::string& key);
	SAnimatedImage* GetSkinAnime(const std::string& key);

	static void RegisterType(asIScriptEngine* engine);
};

SkinHolder* GetSkinObject();
