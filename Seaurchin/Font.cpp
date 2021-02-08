#include "Font.h"
#include "Config.h"
#include "Misc.h"
#include "Setting.h"

using namespace std;

void RectPacker::Init(const int w, const int h, int rowh)
{
    width = w;
    height = h;
    row = 0;
    cursorX = 0;
    cursorY = 0;
}

Rect RectPacker::Insert(const int w, const int h)
{
    if (cursorX + w >= width) {
        cursorX = 0;
        cursorY += row;
    }
    if (cursorY + h > height) return {};
    if (w > width) return {};
    const Rect r = {
        cursorX,
        cursorY,
        w,
        h,
    };
    cursorX += w;
    row = max(h, row);
    return r;
}

// Sif2Creator ----------------------------------------------

void Sif2Creator::InitializeFace(const string& fontpath)
{
    // boost::filesystem::path up = ConvertUTF8ToUnicode(fontpath);
    
    if (faceMemory) return;
    ifstream fontfile(fontpath, ios::in | ios::binary);
    fontfile.seekg(0, ios_base::end);
    faceMemorySize = SU_TO_UINT32(fontfile.tellg());
    fontfile.seekg(ios_base::beg);
    faceMemory = new uint8_t[faceMemorySize];
    fontfile.read(reinterpret_cast<char*>(faceMemory), faceMemorySize);
    fontfile.close();

    // error = FT_New_Face(freetype, up.string().c_str(), 0, &face);
    error = FT_New_Memory_Face(freetype, faceMemory, faceMemorySize, 0, &face);
    if (error) {
        spdlog::error(u8"フォント {0} を読み込めませんでした", fontpath);
        delete[] faceMemory;
        faceMemory = nullptr;
        return;
    }
}

void Sif2Creator::FinalizeFace()
{
    if (!faceMemory) return;
    FT_Done_Face(face);
    delete[] faceMemory;
    faceMemory = nullptr;
}

void Sif2Creator::RequestFace(const float size) const
{
    FT_Size_RequestRec req;
    req.width = 0;
    req.height = int(size * 64.0f);
    req.horiResolution = 0;
    req.vertResolution = 0;
    req.type = FT_SIZE_REQUEST_TYPE_BBOX;

    FT_Request_Size(face, &req);
    FT_Select_Charmap(face, FT_ENCODING_UNICODE);
}

void Sif2Creator::OpenSif2(boost::filesystem::path sif2Path)
{
    sif2Stream.open(sif2Path.string(), ios::out | ios::trunc | ios::binary);
    sif2Stream.seekp(sizeof(Sif2Header));
    imageIndex = 0;
    writtenGlyphs = 0;
}

void Sif2Creator::PackImageSif2()
{
    using namespace boost::filesystem;

    Sif2Header header;
    header.Magic = sif2Magic;
    header.FontSize = currentSize;
    header.Images = imageIndex;
    header.Glyphs = writtenGlyphs;

    for (auto i = 0; i < imageIndex; ++i) {
        ostringstream fss;
        fss << "FontCache" << i << ".png";
        auto path = Setting::GetRootDirectory() / SU_DATA_DIR / SU_CACHE_DIR / fss.str();

        std::ifstream fif;
        fif.open(path.string(), ios::binary | ios::in);
        auto fsize = SU_TO_UINT32(fif.seekg(0, ios::end).tellg());

        const auto file = new uint8_t[fsize];
        fif.seekg(ios::beg);
        fif.read(reinterpret_cast<char*>(file), fsize);
        fif.close();

        sif2Stream.write(reinterpret_cast<const char*>(&fsize), sizeof(uint32_t));
        sif2Stream.write(reinterpret_cast<const char*>(file), fsize);
        delete[] file;
    }

    sif2Stream.seekp(0, ios::beg);
    sif2Stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
}

void Sif2Creator::CloseSif2()
{
    sif2Stream.close();
}

void Sif2Creator::NewBitmap(const uint16_t width, const uint16_t height)
{
    bitmapWidth = width;
    bitmapHeight = height;
    delete[] bitmapMemory;
    bitmapMemory = new uint8_t[width * height * 2];
    memset(bitmapMemory, 0, width * height * 2);
    packer.Init(width, height, 0);
}

void Sif2Creator::SaveBitmapCache(boost::filesystem::path cachepath) const
{
    FILE *file = fopen(cachepath.string().c_str(), "wb");
    if (file == nullptr) return;
    auto png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    auto info = png_create_info_struct(png);
    png_init_io(png, file);
    png_set_IHDR(png, info, bitmapWidth, bitmapHeight, 8, PNG_COLOR_TYPE_GRAY_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_COMPRESSION_TYPE_BASE);
    const auto rows = new png_byte*[bitmapHeight];
    for (auto i = 0; i < bitmapHeight; i++) rows[i] = bitmapMemory + bitmapWidth * i * 2;
    png_set_rows(png, info, rows);
    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, nullptr);
    png_destroy_write_struct(&png, &info);
    if (file) fclose(file);
    delete[] rows;
}

bool Sif2Creator::RenderGlyph(const uint32_t cp)
{
    Sif2Glyph ginfo;
    const auto gidx = FT_Get_Char_Index(face, cp);
    const int baseline = -face->size->metrics.descender >> 6;

    FT_Load_Glyph(face, gidx, FT_LOAD_DEFAULT);
    const auto gslot = face->glyph;
    FT_Render_Glyph(gslot, FT_RENDER_MODE_NORMAL);
    ginfo.GlyphWidth = gslot->bitmap.width;
    ginfo.GlyphHeight = gslot->bitmap.rows;
    ginfo.WholeAdvance = SU_TO_UINT16(gslot->metrics.horiAdvance >> 6);
    ginfo.BearX = SU_TO_INT16(gslot->metrics.horiBearingX >> 6);
    ginfo.BearY = SU_TO_INT16(currentSize - (baseline + (gslot->metrics.horiBearingY >> 6)));
    ginfo.Codepoint = cp;
    ginfo.ImageNumber = imageIndex;
    ginfo.GlyphX = 0;
    ginfo.GlyphY = 0;

    if (ginfo.GlyphWidth * ginfo.GlyphWidth == 0) {
        //まさか' 'がグリフを持たないとは思わなかった(いや当たり前でしょ)
        sif2Stream.write(reinterpret_cast<const char*>(&ginfo), sizeof(Sif2Glyph));
        writtenGlyphs++;
        return true;
    }

    const auto rect = packer.Insert(ginfo.GlyphWidth, ginfo.GlyphHeight);
    if (rect.Height == 0) return false;
    ginfo.GlyphX = rect.X;
    ginfo.GlyphY = rect.Y;

    const auto buffer = new uint8_t[rect.Width * 2];
    for (auto y = 0u; y < gslot->bitmap.rows; y++) {
        for (auto x = 0; x < rect.Width; x++) {
            buffer[x * 2] = 0xff;
            buffer[x * 2 + 1] = gslot->bitmap.buffer[y * rect.Width + x];
        }
        const auto py = rect.Y + y;

        // ew wtf
        memcpy(bitmapMemory + (py * bitmapHeight * 2 + rect.X * 2), buffer, rect.Width * 2);
    }
    delete[] buffer;

    sif2Stream.write(reinterpret_cast<const char*>(&ginfo), sizeof(Sif2Glyph));
    writtenGlyphs++;
    return true;
}

Sif2Creator::Sif2Creator()
{
    FT_Init_FreeType(&freetype);
}

Sif2Creator::~Sif2Creator()
{
    delete[] bitmapMemory;
    FinalizeFace();
    FT_Done_FreeType(freetype);
}

void Sif2Creator::CreateSif2(const Sif2CreatorOption &option, const boost::filesystem::path outputPath)
{
    using namespace boost::filesystem;
    
    const auto cachepath = Setting::GetRootDirectory() / SU_DATA_DIR / SU_CACHE_DIR;

    InitializeFace(option.FontPath);
    RequestFace(option.Size);
    spdlog::info(u8"フォント\"{0:s}\"内に{1:d}グリフあります", face->family_name, face->num_glyphs);

    currentSize = option.Size;
    OpenSif2(outputPath);
    NewBitmap(option.ImageSize, option.ImageSize);

    if (option.TextSource == "") {
        FT_UInt gidx;
        auto code = FT_Get_First_Char(face, &gidx);
        while (gidx) {
            if (!RenderGlyph(code)) {
                ostringstream fss;
                fss << "FontCache" << imageIndex << ".png";
                SaveBitmapCache(cachepath / fss.str());
                imageIndex++;
                NewBitmap(bitmapWidth, bitmapHeight);
                continue;
            }
            code = FT_Get_Next_Char(face, code, &gidx);
        }
    } else {
        // render in text(utf8)
    }

    ostringstream fss;
    fss << "FontCache" << imageIndex << ".png";
    SaveBitmapCache(cachepath / fss.str());
    imageIndex++;

    PackImageSif2();
    CloseSif2();
    FinalizeFace();
}
