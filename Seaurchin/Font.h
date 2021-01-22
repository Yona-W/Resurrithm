#pragma once
#include <stdint.h>
#include <freetype/freetype.h>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <libpng16/png.h>

struct Rect {
    int X;
    int Y;
    int Width;
    int Height;
};

class RectPacker final {
private:
    int width = 0;
    int height = 0;
    int row = 0;
    int cursorX = 0;
    int cursorY = 0;

public:
    void Init(int w, int h, int rowh);
    Rect Insert(int w, int h);
};

struct Sif2Header {
    uint16_t Magic;
    uint16_t Images;
    uint32_t Glyphs;
    float FontSize;
};

struct Sif2Glyph {
    uint32_t Codepoint;
    uint16_t ImageNumber;
    uint16_t GlyphWidth;
    uint16_t GlyphHeight;
    uint16_t GlyphX;
    uint16_t GlyphY;
    int16_t BearX;
    int16_t BearY;
    uint16_t WholeAdvance;
};

enum class FontCreationStyle {
    Normal = 0,
    Bold = 1,
    Oblique = 2,
};

struct Sif2CreatorOption {
    std::string FontPath;
    std::string TextSource;
    float Size;
    uint16_t ImageSize;
    FontCreationStyle Style;
};

class Sif2Creator final {
private:
    const uint16_t sif2Magic = 0xA45F;

    FT_Library freetype = nullptr;
    FT_Error error = 0;
    FT_Face face = nullptr;

    uint8_t *faceMemory = nullptr;
    size_t faceMemorySize = 0;

    RectPacker packer;
    std::ofstream sif2Stream;

    uint8_t *bitmapMemory = nullptr;
    uint16_t bitmapWidth = 0;
    uint16_t bitmapHeight = 0;

    uint16_t imageIndex = 0;
    uint32_t writtenGlyphs = 0;
    float currentSize = 0.0f;

    void InitializeFace(const std::string& fontpath);
    void FinalizeFace();
    void RequestFace(float size) const;

    void OpenSif2(boost::filesystem::path sif2Path);
    void PackImageSif2();
    void CloseSif2();

    void NewBitmap(uint16_t width, uint16_t height);
    void SaveBitmapCache(boost::filesystem::path cachepath) const;

    bool RenderGlyph(uint32_t cp);

public:
    Sif2Creator();
    ~Sif2Creator();

    void CreateSif2(const Sif2CreatorOption &option, boost::filesystem::path outputPath);
};
