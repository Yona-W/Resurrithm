#pragma once

#include <angelscript.h>
#include <set>

#include "Misc.h"
#include "ScriptSpriteMisc.h"
#include "ScriptResource.h"

#include "Crc32.h"

#define SU_IF_COLOR "Color"
#define SU_IF_TF2D "Transform2D"
#define SU_IF_SHAPETYPE "ShapeType"
#define SU_IF_TEXTALIGN "TextAlign"
#define SU_IF_9TYPE "NinePatchType"

#define SU_IF_MOVER_OBJECT "MoverObject"

#define SU_IF_SPRITE "Sprite"
#define SU_IF_SHAPE "Shape"
#define SU_IF_TXTSPRITE "TextSprite"
#define SU_IF_SYHSPRITE "SynthSprite"
#define SU_IF_CLPSPRITE "ClipSprite"
#define SU_IF_ANIMESPRITE "AnimeSprite"
#define SU_IF_CONTAINER "Container"

class MoverObject;
class SSpriteMover;

//基底がImageSpriteでもいい気がしてるんだよね正直
class SSprite {
public:
    enum class FieldID : unsigned int {
        // 未定義を表す
        Undefined = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "unknown"),

        // 共通
        X = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "x"),
        Y = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "y"),
        Z = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "z"),
        OriginX = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "origX"),
        OriginY = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "origY"),
        Angle = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "angle"),
        Scale = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "scale"),
        ScaleX = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "scaleX"),
        ScaleY = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "scaleY"),
        Alpha = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "alpha"),
        R = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "r"),
        G = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "g"),
        B = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "b"),

        Death = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "death"),

        // Shape
        Width = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "width"),
        Height = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "height"),

        // ClipSprite
        U1 = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "u1"),
        V1 = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "v1"),
        U2 = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "u2"),
        V2 = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "v2"),

        // AnimeSprite
        LoopCount = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "loop"),
        Speed = crc32_constexpr::Crc32Rec(0xFFFFFFFF, "speed")
    };

    static FieldID GetFieldId(const std::string &key) { return static_cast<FieldID>(crc32_constexpr::Crc32Rec(0xFFFFFFFF, key.c_str())); }
    static bool GetField(const SSprite* obj, FieldID id, double &retVal);
    static bool SetField(SSprite* obj, FieldID id, double value);

private:
    virtual void DrawBy(const Transform2D &tf, const ColorTint &ct);

protected:
    int reference;
    SSpriteMover *pMover;

    void CopyParameterFrom(SSprite *original);

public:
    //値(CopyParameterFromで一括)
    Transform2D Transform;
    int32_t ZIndex;
    ColorTint Color;
    bool IsDead;
    bool HasAlpha;

    //参照(手動コピー)
    SImage *Image;

    SSprite();
    virtual ~SSprite();
    void AddRef();
    void Release();
    int GetRefCount() const { return reference; }

    virtual void Dismiss() { IsDead = true; }
    void Revive() { IsDead = false; }

    void SetImage(SImage *img);
    const SImage* GetImage() const;

    bool SetPosX(double value) { if (value < -100000 || 100000 < value) return false; Transform.X = SU_TO_FLOAT(value); return true; }
    bool SetPosY(double value) { if (value < -100000 || 100000 < value) return false; Transform.Y = SU_TO_FLOAT(value); return true; }
    bool SetZIndex(double value) { if (value < -100000 || 100000 < value) return false; ZIndex = SU_TO_INT32(value); return true; }
    bool SetOriginX(double value) { if (value < -100000 || 100000 < value) return false; Transform.OriginX = SU_TO_FLOAT(value); return true; }
    bool SetOriginY(double value) { if (value < -100000 || 100000 < value) return false; Transform.OriginY = SU_TO_FLOAT(value); return true; }
    bool SetAngle(double value) { if (value < -100000 || 100000 < value) return false; Transform.Angle = SU_TO_FLOAT(value); return true; }
    bool SetScaleX(double value) { if (value < 0 || 1024 < value) return false; Transform.ScaleX = SU_TO_FLOAT(value); return true; }
    bool SetScaleY(double value) { if (value < 0 || 1024 < value) return false; Transform.ScaleY = SU_TO_FLOAT(value); return true; }
    bool SetAlpha(double value) { if (value < 0.0 || 1.0 < value) return false; Color.A = SU_TO_UINT8(value * 255); return true; }
    bool SetColorR(double value) { if (value < 0.0 || 255 < value) return false;  Color.R = SU_TO_UINT8(value); return true; }
    bool SetColorG(double value) { if (value < 0.0 || 255 < value) return false;  Color.G = SU_TO_UINT8(value); return true; }
    bool SetColorB(double value) { if (value < 0.0 || 255 < value) return false;  Color.B = SU_TO_UINT8(value); return true; }

    bool Apply(FieldID id, double value);
    void Apply(const std::string &dict);
    void Apply(const CScriptDictionary *dict);
    bool SetPosition(double x, double y) { return SetPosX(x) && SetPosY(y); }
    bool SetOrigin(double x, double y) { return SetOriginX(x) && SetOriginY(y); }
    bool SetScale(double scale) { return SetScaleX(scale) && SetScaleY(scale); }
    bool SetScale(double scaleX, double scaleY) { return SetScaleX(scaleX) && SetScaleY(scaleY); }
    bool SetColor(uint8_t r, uint8_t g, uint8_t b) { Color.R = r; Color.G = g; Color.B = b; return true; }
    bool SetColor(double a, uint8_t r, uint8_t g, uint8_t b) { Color.R = r; Color.G = g; Color.B = b; return SetAlpha(a); }

    void AddMove(const std::string &move);
    void AddMove(const std::string &key, const CScriptDictionary *dict);
    void AddMove(const std::string &key, MoverObject* pMoverObj);
    void AbortMove(bool terminate);

    virtual void Tick(double delta);
    virtual void Draw();
    virtual void Draw(const Transform2D &parent, const ColorTint &color);
    virtual SSprite* Clone();

    static SSprite* Factory();
    static SSprite* Factory(SImage *img);
    static void RegisterType(asIScriptEngine *engine);
    struct Comparator {
        bool operator()(const SSprite* lhs, const SSprite* rhs) const
        {
            return lhs->ZIndex < rhs->ZIndex;
        }
    };
};

enum class SShapeType {
    Pixel,
    Box,
    BoxFill,
    Oval,
    OvalFill,
};

//任意の多角形などを表示できる
class SShape : public SSprite {
private :
    typedef SSprite Base;

private:
    void DrawBy(const Transform2D &tf, const ColorTint &ct) override;

public:
    SShapeType Type;
    double Width;
    double Height;

    SShape();

    bool SetWidth(double value) { if (value < -100000 || 100000 < value) return false; Width = value; return true; }
    bool SetHeight(double value) { if (value < -100000 || 100000 < value) return false; Height = value; return true; }

    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    SShape* Clone() override;

    static SShape* Factory();
    static void RegisterType(asIScriptEngine *engine);
};

enum class STextAlign {
    Top = 0,
    Center = 1,
    Bottom = 2,
    Left = 0,
    Right = 2
};

//文字列をスプライトとして扱います
class STextSprite : public SSprite {
protected:
    SRenderTarget *target;
    SRenderTarget *scrollBuffer;
    std::tuple<double, double, int> size;
    STextAlign horizontalAlignment;
    STextAlign verticalAlignment;
    bool isScrolling;
    int scrollWidth;
    int scrollMargin;
    double scrollSpeed;
    double scrollPosition;
    bool isRich;

    void Refresh();
    void DrawNormal(const Transform2D &tf, const ColorTint &ct);
    void DrawScroll(const Transform2D &tf, const ColorTint &ct);

public:
    SFont * Font;
    std::string Text;

    void SetFont(SFont* font);
    void SetText(const std::string &txt);
    void SetAlignment(STextAlign hori, STextAlign vert);
    void SetRangeScroll(int width, int margin, double pps);
    void SetRich(bool enabled);
    double GetWidth();
    double GetHeight();

    STextSprite();
    ~STextSprite() override;
    void Tick(double delta) override;
    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    STextSprite *Clone() override;

    static STextSprite* Factory();
    static STextSprite* Factory(SFont *img, const std::string &str);
    static void RegisterType(asIScriptEngine *engine);
};

//文字入力を扱うスプライトです
//他と違ってDXライブラリのリソースをナマで取得するのであんまりボコボコ使わないでください。
class STextInput : public SSprite {
protected:
    int inputHandle = 0;
    SFont *font = nullptr;
    int selectionStart = -1, selectionEnd = -1;
    int cursor = 0;
    std::string currentRawString = "";

public:
    STextInput();
    ~STextInput() override;
    void SetFont(SFont *font);

    void Activate() const;
    void Draw() override;
    void Tick(double delta) override;

    std::string GetUTF8String() const;

    static STextInput* Factory();
    static STextInput* Factory(SFont *img);
    static void RegisterType(asIScriptEngine *engine);
};

//画像を任意のスプライトから合成してウェイできます
class SSynthSprite : public SSprite {
protected:
    SRenderTarget * target;
    int width;
    int height;

    void DrawBy(const Transform2D &tf, const ColorTint &ct) override;

public:
    SSynthSprite(int w, int h);
    ~SSynthSprite() override;

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    void Clear();
    void Transfer(SSprite *sprite);
    void Transfer(SImage *image, double x, double y);
    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    SSynthSprite *Clone() override;

    static SSynthSprite *Factory(int w, int h);
    static void RegisterType(asIScriptEngine *engine);
};

//画像を任意のスプライトから合成してウェイできます
class SClippingSprite : public SSynthSprite {
private:
    typedef SSynthSprite Base;

protected:
    double u1, v1;
    double u2, v2;

    void DrawBy(const Transform2D &tf, const ColorTint &ct) override;

public:
    SClippingSprite(int w, int h);

    bool SetU1(double value) { if (value < 0 || 1 < value) return false; u1 = value; return true; }
    bool SetV1(double value) { if (value < 0 || 1 < value) return false; v1 = value; return true; }
    bool SetU2(double value) { if (value < 0 || 1 < value) return false; u2 = value; return true; }
    bool SetV2(double value) { if (value < 0 || 1 < value) return false; v2 = value; return true; }

    double GetU1() const { return u1; }
    double GetV1() const { return v1; }
    double GetU2() const { return u2; }
    double GetV2() const { return v2; }

    void SetRange(double tx, double ty, double w, double h);
    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    SClippingSprite *Clone() override;

    static SClippingSprite *Factory(int w, int h);
    static void RegisterType(asIScriptEngine *engine);
};

class SAnimeSprite : public SSprite {
private:
    typedef SSprite Base;

protected:
    SAnimatedImage *images;
    int loopCount, count;
    double speed;
    double time;

    void DrawBy(const Transform2D &tf, const ColorTint &ct) override;

public:
    SAnimeSprite(SAnimatedImage *img);
    ~SAnimeSprite() override;

    bool SetLoopCount(double value) { if (value < -1 || 10000000 < value) return false; loopCount = SU_TO_INT32(value); return true; }
    bool SetSpeed(double value) { if (value < 0) return false; speed = value; return true; }

    double GetLoopCount() const { return SU_TO_DOUBLE(loopCount); }
    double GetSpeed() const { return speed; }

    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    void Tick(double delta) override;
    SAnimeSprite *Clone() override;

    static SAnimeSprite* Factory(SAnimatedImage *image);
    static void RegisterType(asIScriptEngine *engine);
};

enum NinePatchType : uint32_t {
    StretchByRatio = 1,
    StretchByPixel,
    Repeat,
    RepeatAndStretch,
};

class SContainer : public SSprite {
protected:
    std::multiset<SSprite*, SSprite::Comparator> children;

public:
    SContainer();
    ~SContainer() override;

    void AddChild(SSprite *child);
    void Dismiss() override;
    void Tick(double delta) override;
    void Draw() override;
    void Draw(const Transform2D &parent, const ColorTint &color) override;
    SContainer *Clone() override;

    static SContainer* Factory();
    static void RegisterType(asIScriptEngine *engine);
};

template<typename T>
void RegisterSpriteBasic(asIScriptEngine *engine, const char *name)
{
    using namespace std;
    engine->RegisterObjectType(name, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(name, asBEHAVE_ADDREF, "void f()", asMETHOD(T, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(name, asBEHAVE_RELEASE, "void f()", asMETHOD(T, Release), asCALL_THISCALL);

    engine->RegisterObjectProperty(name, SU_IF_COLOR " Color", asOFFSET(T, Color));
    engine->RegisterObjectProperty(name, "bool HasAlpha", asOFFSET(T, HasAlpha));
    engine->RegisterObjectProperty(name, "int Z", asOFFSET(T, ZIndex));
    engine->RegisterObjectProperty(name, SU_IF_TF2D " Transform", asOFFSET(T, Transform));
    engine->RegisterObjectMethod(name, "void SetImage(" SU_IF_IMAGE "@)", asMETHOD(T, SetImage), asCALL_THISCALL);
    //engine->RegisterObjectMethod(name, SU_IF_IMAGE "@ get_Image()", asMETHOD(T, get_Image), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void Dismiss()", asMETHOD(T, Dismiss), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void Apply(const string &in)", asMETHODPR(T, Apply, (const std::string&), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void Apply(const dictionary@)", asMETHODPR(T, Apply, (const CScriptDictionary*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool SetPosition(double, double)", asMETHOD(T, SetPosition), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool SetOrigin(double, double)", asMETHOD(T, SetOrigin), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool SetAngle(double)", asMETHOD(T, SetAngle), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool SetScale(double)", asMETHODPR(T, SetScale, (double), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool SetScale(double, double)", asMETHODPR(T, SetScale, (double, double), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool SetAlpha(double)", asMETHOD(T, SetAlpha), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool SetColor(uint8, uint8, uint8)", asMETHODPR(T, SetColor, (uint8_t, uint8_t, uint8_t), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "bool SetColor(double, uint8, uint8, uint8)", asMETHODPR(T, SetColor, (double, uint8_t, uint8_t, uint8_t), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void AddMove(const string &in)", asMETHODPR(T, AddMove, (const std::string &), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void AddMove(const string &in, const dictionary@)", asMETHODPR(T, AddMove, (const std::string &, const CScriptDictionary *), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void AddMove(const string &in, " SU_IF_MOVER_OBJECT "@)", asMETHODPR(T, AddMove, (const std::string &, MoverObject *), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, "void AbortMove(bool = true)", asMETHOD(T, AbortMove), asCALL_THISCALL);
    engine->RegisterObjectMethod(name, (std::string(name) + "@ Clone()").c_str(), asMETHOD(T, Clone), asCALL_THISCALL);
}



class ExecutionManager;
void RegisterScriptSprite(ExecutionManager *exm);
