#pragma once

#include "ScriptSprite.h"

#define SU_IF_CHARACTER_METRIC "CharacterMetric"
#define SU_IF_CHARACTER_PARAM "Character"
#define SU_IF_CHARACTER_IMAGES "CharacterImages"

struct CharacterImageMetric final {
	double WholeScale;
	int FaceOrigin[2];
	int SmallRange[4];
	int FaceRange[4];

	int GetFaceOrigin(const uint32_t index) { return index < 2 ? FaceOrigin[index] : 0; }
	int GetSmallRange(const uint32_t index) { return index < 4 ? SmallRange[index] : 0; }
	int GetFaceRange(const uint32_t index) { return index < 4 ? FaceRange[index] : 0; }

	static void RegisterType(asIScriptEngine* engine);
};

class CharacterParameter final {
public:
	std::string Name;
	std::string ImagePath;
	CharacterImageMetric Metric;

	static void RegisterType(asIScriptEngine* engine);
};

class CharacterImageSet final {
	INPLEMENT_REF_COUNTER

private:
	std::shared_ptr<CharacterParameter> parameter;
	SImage* imageFull = nullptr;
	SImage* imageSmall = nullptr;
	SImage* imageFace = nullptr;

	void LoadAllImage();

private:
	explicit CharacterImageSet(std::shared_ptr<CharacterParameter> param);
	CharacterImageSet(const CharacterImageSet&) = delete;
	~CharacterImageSet();

public:
	void ApplyFullImage(SSprite * sprite) const;
	void ApplySmallImage(SSprite * sprite) const;
	void ApplyFaceImage(SSprite * sprite) const;

	static CharacterImageSet* CreateImageSet(std::shared_ptr<CharacterParameter> param);
	static void RegisterType(asIScriptEngine * engine);
};
