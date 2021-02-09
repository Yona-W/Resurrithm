#include "ScriptSprite.h"
#include "ScriptSpriteMover.h"
#include "ExecutionManager.h"
#include "Misc.h"

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

using namespace std;
using namespace crc32_constexpr;
using namespace Rendering;

// Applyのパーサ
namespace parser_impl {
    using namespace boost::spirit;
    namespace phx = boost::phoenix;

    // NOTE: キー:値 のペアを一度vectorにするのではなく逐次適用させようと試行錯誤した結果こうなってしまった
    // マルチスレッドで確実に死ぬ、そうでなくとも外部から触れてしまうのでまずい
    // TODO: グローバル変数になってしまったこのポインタを何とかする
    SSprite* pSprite;
    void Apply(SSprite::FieldID id, double value)
    {
        if (!pSprite->Apply(id, value)) {
            // NOTE: Applyがログ出すからそれでいいかな
        }
    }

    template<typename Iterator>
    struct apply_grammer
        : qi::grammar<Iterator, bool(), ascii::space_type>
    {
        qi::rule<Iterator, SSprite::FieldID(), ascii::space_type> field;
        qi::rule<Iterator, bool(), ascii::space_type> pair, apply;

        apply_grammer() : apply_grammer::base_type(apply)
        {
            field = qi::as_string[qi::alpha >> *qi::alnum][qi::_val = phx::bind(&SSprite::GetFieldId, qi::_1)];
            pair = (field >> ':' >> qi::double_)[phx::bind(&Apply, qi::_1, qi::_2)];
            apply = pair >> *(',' >> pair);
        }
    };

    parser_impl::apply_grammer<std::string::const_iterator> gApply;
    bool Parse(const std::string &expression, SSprite *pSprite)
    {
        bool dummy;
        parser_impl::pSprite = pSprite;
        bool result = boost::spirit::qi::phrase_parse(expression.begin(), expression.end(), gApply, boost::spirit::ascii::space, dummy);
        parser_impl::pSprite = nullptr;
        pSprite->Release();
        return result;
    }
}

// 一般

void RegisterScriptSprite(ExecutionManager *exm)
{
    const auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

    MoverObject::RegisterType(engine);

    SSprite::RegisterType(engine);
    SShape::RegisterType(engine);
    STextSprite::RegisterType(engine);
    SSynthSprite::RegisterType(engine);
    SClippingSprite::RegisterType(engine);
    SAnimeSprite::RegisterType(engine);
    SContainer::RegisterType(engine);
}

//SSprite ------------------

bool SSprite::GetField(const SSprite* pSprite, FieldID id, double &value)
{
    if (!pSprite) return false;

    const SShape *pShape = dynamic_cast<const SShape*>(pSprite);
    const SClippingSprite *pClip = dynamic_cast<const SClippingSprite*>(pSprite);
    const SAnimeSprite *pAnim = dynamic_cast<const SAnimeSprite*>(pSprite);

    switch (id)
    {
    case FieldID::X: value = SU_TO_DOUBLE(pSprite->Transform.X); return true;
    case FieldID::Y: value = SU_TO_DOUBLE(pSprite->Transform.Y); return true;
    case FieldID::Z: value = SU_TO_DOUBLE(pSprite->ZIndex); return true;
    case FieldID::OriginX: value = SU_TO_DOUBLE(pSprite->Transform.OriginX); return true;
    case FieldID::OriginY: value = SU_TO_DOUBLE(pSprite->Transform.OriginY); return true;
    case FieldID::Angle: value = SU_TO_DOUBLE(pSprite->Transform.Angle); return true;
    case FieldID::Scale: value = SU_TO_DOUBLE(pSprite->Transform.ScaleX + pSprite->Transform.ScaleY) / 2.0; return true;
    case FieldID::ScaleX: value = SU_TO_DOUBLE(pSprite->Transform.ScaleX); return true;
    case FieldID::ScaleY: value = SU_TO_DOUBLE(pSprite->Transform.ScaleY); return true;
    case FieldID::Alpha: value = SU_TO_DOUBLE(pSprite->Color.A / 255.0); return true;
    case FieldID::R: value = SU_TO_DOUBLE(pSprite->Color.R); return true;
    case FieldID::G: value = SU_TO_DOUBLE(pSprite->Color.G); return true;
    case FieldID::B: value = SU_TO_DOUBLE(pSprite->Color.B); return true;

    case FieldID::Death: value = pSprite->IsDead ? 1.0 : 0.0; return true;

    case FieldID::Width: if (!pShape) return false; value = SU_TO_DOUBLE(pShape->Width); return true;
    case FieldID::Height: if (!pShape) return false; value = SU_TO_DOUBLE(pShape->Height); return true;

    case FieldID::U1: if (!pClip) return false; value = pClip->GetU1(); return true;
    case FieldID::V1: if (!pClip) return false; value = pClip->GetV1(); return true;
    case FieldID::U2: if (!pClip) return false; value = pClip->GetU2(); return true;
    case FieldID::V2: if (!pClip) return false; value = pClip->GetV2(); return true;

    case FieldID::LoopCount: if (!pAnim) return false; value = pAnim->GetLoopCount(); return true;
    case FieldID::Speed: if (!pAnim) return false; value = pAnim->GetSpeed(); return true;

    default: return false;
    }
}

bool SSprite::SetField(SSprite* pSprite, FieldID id, double value)
{
    if (!pSprite) {
        spdlog::critical(u8"なんで");
        return false;
    }

    SShape *pShape = dynamic_cast<SShape*>(pSprite);
    SClippingSprite *pClip = dynamic_cast<SClippingSprite*>(pSprite);
    SAnimeSprite *pAnim = dynamic_cast<SAnimeSprite*>(pSprite);

    switch (id) {
    case FieldID::X: return pSprite->SetPosX(value);
    case FieldID::Y: return pSprite->SetPosY(value);
    case FieldID::Z: return pSprite->SetZIndex(value);
    case FieldID::OriginX: return pSprite->SetOriginX(value);
    case FieldID::OriginY: return pSprite->SetOriginY(value);
    case FieldID::Angle: return pSprite->SetAngle(value);
    case FieldID::Scale: return pSprite->SetScale(value);
    case FieldID::ScaleX: return pSprite->SetScaleX(value);
    case FieldID::ScaleY: return pSprite->SetScaleY(value);
    case FieldID::Alpha: return pSprite->SetAlpha(value);
    case FieldID::R: return pSprite->SetColorR(value);
    case FieldID::G: return pSprite->SetColorG(value);
    case FieldID::B: return pSprite->SetColorB(value);

    case FieldID::Death: pSprite->Dismiss(); return true;

    case FieldID::Width: if (!pShape) return false; return pShape->SetWidth(value);
    case FieldID::Height: if (!pShape) return false; return pShape->SetHeight(value);

    case FieldID::U1: if (!pClip) return false; return pClip->SetU1(value);
    case FieldID::V1: if (!pClip) return false; return pClip->SetV1(value);
    case FieldID::U2: if (!pClip) return false; return pClip->SetU2(value);
    case FieldID::V2: if (!pClip) return false; return pClip->SetV2(value);

    case FieldID::LoopCount: if (!pAnim) return false; return pAnim->SetLoopCount(value);
    case FieldID::Speed: if (!pAnim) return false; return pAnim->SetSpeed(value);

    default: return false;
    }
}

void SSprite::CopyParameterFrom(SSprite * original)
{
    Transform = original->Transform;
    ZIndex = original->ZIndex;
    Color = original->Color;
    IsDead = original->IsDead;
    HasAlpha = original->HasAlpha;
}

void SSprite::SetImage(SImage * img)
{
    if (Image) Image->Release();
    Image = img;
}

const SImage *SSprite::GetImage() const
{
    if (Image) Image->AddRef();
    return Image;
}

SSprite::SSprite()
    : reference(0)
    , pMover(new SSpriteMover(this)) // NOTE: this渡すのはちょっと怖いけど
    , ZIndex(0)
    , Color(Colors::white)
    , IsDead(false)
    , HasAlpha(true)
    , Image(nullptr)
{
}

SSprite::~SSprite()
{
    if (Image) Image->Release();
    Image = nullptr;
    delete pMover;
}

void SSprite::AddRef()
{
    ++reference;
}

void SSprite::Release()
{
    if (--reference == 0) delete this;
}

bool SSprite::Apply(FieldID id, const double value)
{
    return SSprite::SetField(this, id, value);
}

void SSprite::Apply(const string & dict)
{
    this->AddRef();
    bool result = parser_impl::Parse(dict, this);

    return;
}

void SSprite::Apply(const CScriptDictionary *dict)
{
    auto i = dict->begin();
    while (i != dict->end()) {
        const auto key = i.GetKey();
        const auto id = SSprite::GetFieldId(key);
        double dv = 0;
        if(i.GetValue(dv)) if(!Apply(id, dv)) spdlog::warn(u8"プロパティに \"{0}\" は設定できません。", key);
        ++i;
    }

    dict->Release();
}

void SSprite::AddMove(const string & dict)
{
    pMover->AddMove(dict);
}

void SSprite::AddMove(const string &key, const CScriptDictionary *dict)
{
    dict->AddRef();
    pMover->AddMove(key, dict);

    dict->Release();
}

void SSprite::AddMove(const string &key, MoverObject *pMoverObj)
{
    if (!pMoverObj) return;

    MoverObject* pClone = pMoverObj->Clone();

    SSprite::FieldID id = SSprite::GetFieldId(key);
    if (!pClone->RegisterTargetField(id)) {
        pClone->Release();
        pMoverObj->Release();
        return;
    }

    pMover->AddMove(pClone);

    pMoverObj->Release();
}

void SSprite::AbortMove(const bool terminate)
{
    pMover->Abort(terminate);
}

void SSprite::Tick(const double delta)
{
    pMover->Tick(delta);
}

void SSprite::Draw(GPU_Target *target)
{
    DrawBy(Transform, Color, target);
}

void SSprite::Draw(const Transform2D &parent, const ColorTint &color, GPU_Target *target)
{
    const auto tf = Transform.ApplyFrom(parent);
    const auto cl = Color.ApplyFrom(color);
    DrawBy(tf, cl, target);
}

void SSprite::DrawBy(const Transform2D &tf, const ColorTint &ct, GPU_Target *target)
{
    if (!Image) return;
    GPU_Image *image = Image->GetTexture();

    GPU_SetColor(image, SDL_Color{ct.R, ct.G, ct.B, ct.A});
    GPU_BlitTransformX(image,
                      NULL,
                      target,
                      tf.X,
                      tf.Y,
                      tf.OriginX,
                      tf.OriginY,
                      tf.Angle,
                      tf.ScaleX,
                      tf.ScaleY);
}

SSprite * SSprite::Clone()
{
    //コピコンで良くないかこれ
    auto clone = new SSprite();
    clone->AddRef();

    clone->CopyParameterFrom(this);

    {
        if(Image) Image->AddRef();
        clone->SetImage(Image);
    }

    BOOST_ASSERT(clone->GetRefCount() == 1);
    return clone;
}

SSprite *SSprite::Factory()
{
    auto result = new SSprite();
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

SSprite * SSprite::Factory(SImage * img)
{
    auto result = new SSprite();
    result->AddRef();

    {
        if(img) img->AddRef();
        result->SetImage(img);
    }

    if(img) img->Release();
    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

ColorTint GetColor(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
    return ColorTint { a, r, g, b };
}

void SSprite::RegisterType(asIScriptEngine * engine)
{
    //Transform2D
    engine->RegisterObjectType(SU_IF_TF2D, sizeof(Transform2D), asOBJ_VALUE | asOBJ_APP_CLASS_CD);
    engine->RegisterObjectBehaviour(SU_IF_TF2D, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(AngelScriptValueConstruct<Transform2D>), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour(SU_IF_TF2D, asBEHAVE_DESTRUCT, "void f()", asFUNCTION(AngelScriptValueDestruct<Transform2D>), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty(SU_IF_TF2D, "float X", asOFFSET(Transform2D, X));
    engine->RegisterObjectProperty(SU_IF_TF2D, "float Y", asOFFSET(Transform2D, Y));
    engine->RegisterObjectProperty(SU_IF_TF2D, "float Angle", asOFFSET(Transform2D, Angle));
    engine->RegisterObjectProperty(SU_IF_TF2D, "float OriginX", asOFFSET(Transform2D, OriginX));
    engine->RegisterObjectProperty(SU_IF_TF2D, "float OriginY", asOFFSET(Transform2D, OriginY));
    engine->RegisterObjectProperty(SU_IF_TF2D, "float ScaleX", asOFFSET(Transform2D, ScaleX));
    engine->RegisterObjectProperty(SU_IF_TF2D, "float ScaleY", asOFFSET(Transform2D, ScaleY));

    //Color
    engine->RegisterObjectType(SU_IF_COLOR, sizeof(ColorTint), asOBJ_VALUE | asOBJ_APP_CLASS_CD);
    engine->RegisterObjectBehaviour(SU_IF_COLOR, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(AngelScriptValueConstruct<ColorTint>), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour(SU_IF_COLOR, asBEHAVE_DESTRUCT, "void f()", asFUNCTION(AngelScriptValueConstruct<ColorTint>), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty(SU_IF_COLOR, "uint8 A", asOFFSET(ColorTint, A));
    engine->RegisterObjectProperty(SU_IF_COLOR, "uint8 R", asOFFSET(ColorTint, R));
    engine->RegisterObjectProperty(SU_IF_COLOR, "uint8 G", asOFFSET(ColorTint, G));
    engine->RegisterObjectProperty(SU_IF_COLOR, "uint8 B", asOFFSET(ColorTint, B));

    //ShapeType
    engine->RegisterEnum(SU_IF_SHAPETYPE);
    engine->RegisterEnumValue(SU_IF_SHAPETYPE, "Pixel", int(SShapeType::Pixel));
    engine->RegisterEnumValue(SU_IF_SHAPETYPE, "Box", int(SShapeType::Box));
    engine->RegisterEnumValue(SU_IF_SHAPETYPE, "BoxFill", int(SShapeType::BoxFill));
    engine->RegisterEnumValue(SU_IF_SHAPETYPE, "Oval", int(SShapeType::Oval));
    engine->RegisterEnumValue(SU_IF_SHAPETYPE, "OvalFill", int(SShapeType::OvalFill));

    //ShapeType
    engine->RegisterEnum(SU_IF_TEXTALIGN);
    engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Top", int(STextAlign::Top));
    engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Bottom", int(STextAlign::Bottom));
    engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Left", int(STextAlign::Left));
    engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Right", int(STextAlign::Right));
    engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Center", int(STextAlign::Center));

    RegisterSpriteBasic<SSprite>(engine, SU_IF_SPRITE);
    engine->RegisterObjectBehaviour(SU_IF_SPRITE, asBEHAVE_FACTORY, SU_IF_SPRITE "@ f()", asFUNCTIONPR(SSprite::Factory, (), SSprite*), asCALL_CDECL);
    engine->RegisterObjectBehaviour(SU_IF_SPRITE, asBEHAVE_FACTORY, SU_IF_SPRITE "@ f(" SU_IF_IMAGE "@)", asFUNCTIONPR(SSprite::Factory, (SImage*), SSprite*), asCALL_CDECL);
}

// Shape -----------------

SShape::SShape()
    : Type(SShapeType::BoxFill)
    , Width(32)
    , Height(32)
{
}

void SShape::DrawBy(const Transform2D &tf, const ColorTint &ct, GPU_Target *target)
{
    auto color = GPU_MakeColor(ct.R, ct.G, ct.B, ct.A);
    GPU_SetShapeBlendMode(GPU_BlendPresetEnum::GPU_BLEND_NORMAL_ADD_ALPHA);
    std::vector<float> vertices;

    switch (Type)
    {
    case SShapeType::Pixel:
        GPU_Pixel(target, tf.X, tf.Y, color);
        break;
    case SShapeType::BoxFill:
    case SShapeType::Box:
    {
        const glm::vec2 points[] = {
            glm::rotate(glm::vec2(float(+Width * tf.ScaleX / 2.0f), float(+Height * tf.ScaleY / 2.0f)), tf.Angle),
            glm::rotate(glm::vec2(float(-Width * tf.ScaleX / 2.0f), float(+Height * tf.ScaleY / 2.0f)), tf.Angle),
            glm::rotate(glm::vec2(float(-Width * tf.ScaleX / 2.0f), float(-Height * tf.ScaleY / 2.0f)), tf.Angle),
            glm::rotate(glm::vec2(float(+Width * tf.ScaleX / 2.0f), float(-Height * tf.ScaleY / 2.0f)), tf.Angle)};

        for (auto point : points)
        {
            vertices.push_back(point.x);
            vertices.push_back(point.y);
        }
        if (Type == SShapeType::Box)
        {
            GPU_Polygon(target, 4, vertices.data(), color);
        }
        else
        {
            GPU_PolygonFilled(target, 4, vertices.data(), color);
        }
    }
    break;
    case SShapeType::Oval:
    case SShapeType::OvalFill:
    {
        for (auto i = 0; i <= 256; ++i)
        {
            const auto angle = 2.0 * glm::pi<double>() / 256.0 * i;
            const auto next = glm::rotate(
                glm::vec2(Width * tf.ScaleX / 2.0 * glm::cos(angle), Height * tf.ScaleY / 2.0 * glm::sin(angle)),
                float(tf.Angle));
            vertices.push_back(next.x);
            vertices.push_back(next.y);
        }
        if (Type == SShapeType::Oval)
        {
            GPU_Polygon(target, 256, vertices.data(), color);
        }
        else
        {
            GPU_PolygonFilled(target, 256, vertices.data(), color);
        }
    }
    break;
    }
}

void SShape::Draw(GPU_Target *target)
{
    DrawBy(Transform, Color, target);
}

void SShape::Draw(const Transform2D &parent, const ColorTint &color, GPU_Target *target)
{
    const auto tf = Transform.ApplyFrom(parent);
    const auto cl = Color.ApplyFrom(color);
    DrawBy(tf, cl, target);
}

SShape * SShape::Clone()
{
    //やっぱりコピコンで良くないかこれ
    auto clone = new SShape();
    clone->AddRef();

    clone->CopyParameterFrom(this);

    clone->Width = Width;
    clone->Height = Height;

    BOOST_ASSERT(clone->GetRefCount() == 1);
    return clone;
}

SShape * SShape::Factory()
{
    auto result = new SShape();
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

void SShape::RegisterType(asIScriptEngine * engine)
{
    RegisterSpriteBasic<SShape>(engine, SU_IF_SHAPE);
    engine->RegisterObjectBehaviour(SU_IF_SHAPE, asBEHAVE_FACTORY, SU_IF_SHAPE "@ f()", asFUNCTIONPR(SShape::Factory, (), SShape*), asCALL_CDECL);
    engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_SHAPE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SShape>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_SHAPE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SShape, SSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectProperty(SU_IF_SHAPE, "double Width", asOFFSET(SShape, Width));
    engine->RegisterObjectProperty(SU_IF_SHAPE, "double Height", asOFFSET(SShape, Height));
    engine->RegisterObjectProperty(SU_IF_SHAPE, SU_IF_SHAPETYPE " Type", asOFFSET(SShape, Type));
}

// TextSprite -----------

void STextSprite::Refresh()
{
    delete target;
    delete scrollBuffer;
    if (!Font) {
        size = std::make_tuple<double, double, int>(0.0, 0.0, 0);
        return;
    }

    size = isRich ? Font->RenderRich(nullptr, Text, Color) : Font->RenderRaw(nullptr, Text);
    if (isScrolling) {
        scrollBuffer = new SRenderTarget(scrollWidth, int(get<1>(size)));
    }

    target = new SRenderTarget(int(get<0>(size)), int(get<1>(size)));
    if (isRich) {
        Font->RenderRich(target, Text, Color);
    } else {
        Font->RenderRaw(target, Text);
    }
}

void STextSprite::DrawNormal(const Transform2D &tf, const ColorTint &ct, GPU_Target *drawTarget)
{
    const auto tox = SU_TO_FLOAT(get<0>(size) / 2 * int(horizontalAlignment));
    const auto toy = SU_TO_FLOAT(get<1>(size) / 2 * int(verticalAlignment));

    GPU_Image *image = target->GetImage();

    GPU_SetBlendMode(image, GPU_BLEND_NORMAL_ADD_ALPHA);

    if(!isRich){
        GPU_SetColor(image, SDL_Color{ct.R, ct.G, ct.B, ct.A});
    }

    GPU_BlitTransformX(image,
                      NULL,
                      drawTarget,
                      tf.X,
                      tf.Y,
                      tf.OriginX + tox,
                      tf.OriginY + toy,
                      tf.Angle,
                      tf.ScaleX,
                      tf.ScaleY);
}

void STextSprite::DrawScroll(const Transform2D &tf, const ColorTint &ct, GPU_Target *drawTarget)
{
    GPU_Clear(scrollBuffer->GetTarget());
    if (scrollSpeed >= 0) {
        auto reach = -scrollPosition + int(scrollPosition / (get<0>(size) + scrollMargin)) * (get<0>(size) + scrollMargin);
        while (reach < scrollWidth) {
            GPU_Rect srcRect = {0, 0, static_cast<float>(get<0>(size)), static_cast<float>(get<1>(size))};
            GPU_Rect dstRect = {static_cast<float>(reach), 0, srcRect.w, srcRect.h};
            GPU_BlitRect(
                target->GetTexture(),
                &srcRect,
                scrollBuffer->GetTarget(),
                &dstRect
            );
            reach += get<0>(size) + scrollMargin;
        }
    } else {
        auto reach = -scrollPosition - int(scrollPosition / (get<0>(size) + scrollMargin)) * (get<0>(size) + scrollMargin);
        while (reach > 0) {
            GPU_Rect srcRect = {0, 0, static_cast<float>(get<0>(size)), static_cast<float>(get<1>(size))};
            GPU_Rect dstRect = {static_cast<float>(reach), 0, srcRect.w, srcRect.h};
            GPU_BlitRect(
                target->GetTexture(),
                &srcRect,
                scrollBuffer->GetTarget(),
                &dstRect
            );
            reach -= get<0>(size) + scrollMargin;
        }
    }

    const auto tox = SU_TO_FLOAT(scrollWidth / 2 * int(horizontalAlignment));
    const auto toy = SU_TO_FLOAT(get<1>(size) / 2 * int(verticalAlignment));
    GPU_SetColor(scrollBuffer->GetTexture(), GPU_MakeColor(ct.R, ct.G, ct.B, ct.A));

    GPU_BlitTransformX(scrollBuffer->GetTexture(),
                      NULL,
                      drawTarget,
                      tf.X,
                      tf.Y,
                      tf.OriginX + tox,
                      tf.OriginY + toy,
                      tf.Angle,
                      tf.ScaleX,
                      tf.ScaleY);
}

void STextSprite::SetFont(SFont * font)
{
    if (Font) Font->Release();
    Font = font;
    Refresh();
}

void STextSprite::SetText(const string & txt)
{
    Text = txt;
    Refresh();
}

void STextSprite::SetAlignment(const STextAlign hori, const STextAlign vert)
{
    horizontalAlignment = hori;
    verticalAlignment = vert;
}

void STextSprite::SetRangeScroll(const int width, const int margin, const double pps)
{
    isScrolling = width > 0;
    scrollWidth = width;
    scrollMargin = margin;
    scrollSpeed = pps;
    scrollPosition = 0;
    Refresh();
}

void STextSprite::SetRich(const bool enabled)
{
    isRich = enabled;
    Refresh();
}

double STextSprite::GetWidth()
{
    return get<0>(size);
}

double STextSprite::GetHeight()
{
    return get<1>(size);
}

STextSprite::~STextSprite()
{
    if (Font) Font->Release();
    delete target;
    delete scrollBuffer;
}

void STextSprite::Tick(const double delta)
{
    SSprite::Tick(delta);
    if (isScrolling) scrollPosition += scrollSpeed * delta;
}

void STextSprite::Draw(GPU_Target *drawTarget)
{
    if (!target) return;
    if (isScrolling && target->GetWidth() >= scrollWidth) {
        DrawScroll(Transform, Color, drawTarget);
    } else {
        DrawNormal(Transform, Color, drawTarget);
    }
}

void STextSprite::Draw(const Transform2D & parent, const ColorTint & color, GPU_Target *drawTarget)
{
    const auto tf = Transform.ApplyFrom(parent);
    const auto cl = Color.ApplyFrom(color);
    if (!target) return;
    if (isScrolling && target->GetWidth() >= scrollWidth) {
        DrawScroll(tf, cl, drawTarget);
    } else {
        DrawNormal(tf, cl, drawTarget);
    }
}

STextSprite::STextSprite()
    : target(nullptr)
    , scrollBuffer(nullptr)
    , size(0.0, 0.0, 0)
    , horizontalAlignment(STextAlign::Left)
    , verticalAlignment(STextAlign::Top)
    , isScrolling(false)
    , scrollWidth(0)
    , scrollMargin(0)
    , scrollSpeed(0.0)
    , scrollPosition(0.0)
    , isRich(false)
    , Font(nullptr)
    , Text("")
{
}

STextSprite * STextSprite::Clone()
{
    //やっぱりコピコンで良くないかこれ
    auto clone = new STextSprite();
    clone->AddRef();

    clone->CopyParameterFrom(this);

    if (target) {
        clone->SetText(Text);
    }
    if (Font) {
        Font->AddRef();
        clone->SetFont(Font);
    }

    BOOST_ASSERT(clone->GetRefCount() == 1);
    return clone;
}

STextSprite *STextSprite::Factory()
{
    auto result = new STextSprite();
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

STextSprite *STextSprite::Factory(SFont *img, const string &str)
{
    auto result = new STextSprite();
    result->AddRef();

    result->SetText(str.length() == 0 ? " " : str);
    {
        if(img) img->AddRef();
        result->SetFont(img);
    }

    if (img) img->Release();
    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

void STextSprite::RegisterType(asIScriptEngine *engine)
{
    RegisterSpriteBasic<STextSprite>(engine, SU_IF_TXTSPRITE);
    engine->RegisterObjectBehaviour(SU_IF_TXTSPRITE, asBEHAVE_FACTORY, SU_IF_TXTSPRITE "@ f()", asFUNCTIONPR(STextSprite::Factory, (), STextSprite*), asCALL_CDECL);
    engine->RegisterObjectBehaviour(SU_IF_TXTSPRITE, asBEHAVE_FACTORY, SU_IF_TXTSPRITE "@ f(" SU_IF_FONT "@, const string &in)", asFUNCTIONPR(STextSprite::Factory, (SFont*, const string&), STextSprite*), asCALL_CDECL);
    engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_TXTSPRITE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, STextSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_TXTSPRITE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<STextSprite, SSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetFont(" SU_IF_FONT "@)", asMETHOD(STextSprite, SetFont), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetText(const string &in)", asMETHOD(STextSprite, SetText), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetAlignment(" SU_IF_TEXTALIGN ", " SU_IF_TEXTALIGN ")", asMETHOD(STextSprite, SetAlignment), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetRangeScroll(int, int, double)", asMETHOD(STextSprite, SetRangeScroll), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetRich(bool)", asMETHOD(STextSprite, SetRich), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "double get_Width()", asMETHOD(STextSprite, GetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "double get_Height()", asMETHOD(STextSprite, GetHeight), asCALL_THISCALL);
}

// STextInput ---------------------------------------


/* lol screw this

STextInput::STextInput()
{
    //TODO: デフォルト値の引数化
    inputHandle = MakeKeyInput(1024, TRUE, FALSE, FALSE, FALSE, FALSE);
}

STextInput::~STextInput()
{
    if (inputHandle) DeleteKeyInput(inputHandle);
}

void STextInput::SetFont(SFont *sfont)
{
    if (font) font->Release();
    font = sfont;
}

void STextInput::Activate() const
{
    SetActiveKeyInput(inputHandle);
}

void STextInput::Draw()
{
    const auto wcc = MultiByteToWideChar(CP_OEMCP, 0, currentRawString.c_str(), 1024, nullptr, 0);
    const auto widestr = new wchar_t[wcc];
    MultiByteToWideChar(CP_OEMCP, 0, currentRawString.c_str(), 1024, widestr, 0);
    delete[] widestr;
    // TODO wtf is this?
}

void STextInput::Tick(double delta)
{
    char buffer[1024] = { 0 };
    switch (CheckKeyInput(inputHandle)) {
        case 0:
            GetKeyInputString(buffer, inputHandle);
            currentRawString = buffer;
            cursor = GetKeyInputCursorPosition(inputHandle);
            GetKeyInputSelectArea(&selectionStart, &selectionEnd, inputHandle);
            return;
        case 1:
        case 2:
            selectionStart = selectionEnd = cursor = -1;
            return;
        default: break;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
std::string STextInput::GetUTF8String() const
{
    return std::string();
}

STextInput *STextInput::Factory()
{
    return nullptr;
}

STextInput *STextInput::Factory(SFont *img)
{
    return nullptr;
}

void STextInput::RegisterType(asIScriptEngine *engine)
{}

*/

// SSynthSprite -------------------------------------

void SSynthSprite::DrawBy(const Transform2D & tf, const ColorTint & ct, GPU_Target *drawTarget)
{
    if (!target) return;
    GPU_Image *image = target->GetTexture();
    GPU_SetBlendMode(image, GPU_BLEND_NORMAL_ADD_ALPHA);
    GPU_SetColor(image, SDL_Color{ct.R, ct.G, ct.B, ct.A});

    GPU_BlitTransformX(image,
                      NULL,
                      drawTarget,
                      tf.X,
                      tf.Y,
                      tf.OriginX,
                      tf.OriginY,
                      tf.Angle,
                      tf.ScaleX,
                      tf.ScaleY);
}

SSynthSprite::SSynthSprite(const int w, const int h)
    : target(nullptr)
    , width(w)
    , height(h)
{
}

SSynthSprite::~SSynthSprite()
{
    delete target;
}

void SSynthSprite::Clear()
{
    delete target;
    target = new SRenderTarget(width, height);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SSynthSprite::Transfer(SSprite *sprite)
{
    if (!sprite) return;

    sprite->Draw(target->GetTarget());

    //sprite->Release();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SSynthSprite::Transfer(SImage * image, const double x, const double y)
{
    if (!image) return;

    GPU_Image *texture = image->GetTexture();
    GPU_SetBlendMode(texture, GPU_BlendPresetEnum::GPU_BLEND_NORMAL_ADD_ALPHA);
    GPU_Blit(texture, NULL, target->GetTarget(), x, y);

    //image->Release();
}

void SSynthSprite::Draw(GPU_Target *target)
{
    DrawBy(Transform, Color, target);
}

void SSynthSprite::Draw(const Transform2D & parent, const ColorTint & color, GPU_Target *target)
{
    const auto tf = Transform.ApplyFrom(parent);
    const auto cl = Color.ApplyFrom(color);
    DrawBy(tf, cl, target);
}

SSynthSprite* SSynthSprite::Clone()
{
    auto clone = new SSynthSprite(width, height);
    clone->AddRef();

    clone->CopyParameterFrom(this);

    if (target) {
        this->AddRef();
        clone->Transfer(this);
    }

    BOOST_ASSERT(clone->GetRefCount() == 1);
    return clone;
}

SSynthSprite *SSynthSprite::Factory(const int w, const int h)
{
    auto result = new SSynthSprite(w, h);
    result->AddRef();

    result->Clear();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

void SSynthSprite::RegisterType(asIScriptEngine * engine)
{
    RegisterSpriteBasic<SSynthSprite>(engine, SU_IF_SYHSPRITE);
    engine->RegisterObjectBehaviour(SU_IF_SYHSPRITE, asBEHAVE_FACTORY, SU_IF_SYHSPRITE "@ f(int, int)", asFUNCTION(SSynthSprite::Factory), asCALL_CDECL);
    engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_SYHSPRITE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SSynthSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_SYHSPRITE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SSynthSprite, SSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "int get_Width()", asMETHOD(SSynthSprite, GetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "int get_Height()", asMETHOD(SSynthSprite, GetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "void Clear()", asMETHOD(SSynthSprite, Clear), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "void Transfer(" SU_IF_SPRITE "@)", asMETHODPR(SSynthSprite, Transfer, (SSprite*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "void Transfer(" SU_IF_IMAGE "@, double, double)", asMETHODPR(SSynthSprite, Transfer, (SImage*, double, double), void), asCALL_THISCALL);
}

// SClippingSprite ------------------------------------------

void SClippingSprite::DrawBy(const Transform2D & tf, const ColorTint & ct, GPU_Target *drawTarget)
{
    if (!target) return;
    const auto x = SU_TO_INT32(width * u1);
    const auto y = SU_TO_INT32(height * v1);
    const auto w = SU_TO_INT32(width * u2);
    const auto h = SU_TO_INT32(height * v2);

    auto texture = target->GetTexture();
    GPU_SetColor(texture, SDL_Color{ct.R, ct.G, ct.B, ct.A});

    GPU_BlitTransformX(texture,
                      NULL,
                      drawTarget,
                      tf.X,
                      tf.Y,
                      tf.OriginX,
                      tf.OriginY,
                      tf.Angle,
                      tf.ScaleX,
                      tf.ScaleY);
}

SClippingSprite::SClippingSprite(const int w, const int h)
    : SSynthSprite(w, h)
    , u1(0), v1(0)
    , u2(1), v2(0)
{}

void SClippingSprite::SetRange(const double tx, const double ty, const double w, const double h)
{
    u1 = tx;
    v1 = ty;
    u2 = w;
    v2 = h;
}

void SClippingSprite::Draw(GPU_Target *target)
{
    DrawBy(Transform, Color, target);
}

void SClippingSprite::Draw(const Transform2D & parent, const ColorTint & color, GPU_Target *target)
{
    const auto tf = Transform.ApplyFrom(parent);
    const auto cl = Color.ApplyFrom(color);
    DrawBy(tf, cl, target);
}

SClippingSprite *SClippingSprite::Clone()
{
    auto clone = new SClippingSprite(width, height);
    clone->AddRef();

    clone->CopyParameterFrom(this);

    if (target) {
        this->AddRef();
        clone->Transfer(this);
    }

    clone->u1 = u1;
    clone->v1 = v1;
    clone->u2 = u2;
    clone->v2 = v2;

    BOOST_ASSERT(clone->GetRefCount() == 1);
    return clone;
}

SClippingSprite *SClippingSprite::Factory(const int w, const int h)
{
    auto result = new SClippingSprite(w, h);
    result->AddRef();

    result->Clear();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

void SClippingSprite::RegisterType(asIScriptEngine * engine)
{
    RegisterSpriteBasic<SClippingSprite>(engine, SU_IF_CLPSPRITE);
    engine->RegisterObjectBehaviour(SU_IF_CLPSPRITE, asBEHAVE_FACTORY, SU_IF_CLPSPRITE "@ f(int, int)", asFUNCTION(SClippingSprite::Factory), asCALL_CDECL);
    engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_CLPSPRITE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SClippingSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_CLPSPRITE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SClippingSprite, SSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "int get_Width()", asMETHOD(SClippingSprite, GetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "int get_Height()", asMETHOD(SClippingSprite, GetWidth), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "void Clear()", asMETHOD(SClippingSprite, Clear), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "void Transfer(" SU_IF_SPRITE "@)", asMETHODPR(SClippingSprite, Transfer, (SSprite*), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "void Transfer(" SU_IF_IMAGE "@, double, double)", asMETHODPR(SClippingSprite, Transfer, (SImage*, double, double), void), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "void SetRange(double, double, double, double)", asMETHOD(SClippingSprite, SetRange), asCALL_THISCALL);
}

// SAnimeSprite -------------------------------------------
void SAnimeSprite::DrawBy(const Transform2D &tf, const ColorTint &ct, GPU_Target *target)
{
    const auto at = images->GetCellTime() * images->GetFrameCount() - time;
    GPU_Image *image = images->GetTexture();

    GPU_Rect srcRect = images->GetRectAt(at);

    GPU_BlitTransformX(image,
                      &srcRect,
                      target,
                      tf.X,
                      tf.Y,
                      tf.OriginX,
                      tf.OriginY,
                      tf.Angle,
                      tf.ScaleX,
                      tf.ScaleY);
}

SAnimeSprite::SAnimeSprite(SAnimatedImage * img)
    : images(img)
    , loopCount(1)
    , count(0)
    , speed(1)
    , time(img ? (img->GetCellTime() * img->GetFrameCount()) : 0)
{
}

SAnimeSprite::~SAnimeSprite()
{
    if (images) images->Release();
}

void SAnimeSprite::Draw(GPU_Target *target)
{
    DrawBy(Transform, Color, target);
}

void SAnimeSprite::Draw(const Transform2D &parent, const ColorTint &color, GPU_Target *target)
{
    const auto tf = Transform.ApplyFrom(parent);
    const auto cl = Color.ApplyFrom(color);
    DrawBy(tf, cl, target);
}

void SAnimeSprite::Tick(const double delta)
{
    pMover->Tick(delta);

    time -= delta * speed;
    if (time > 0) return;
    if (loopCount > 0 && ++count == loopCount) Dismiss();
    time = images->GetCellTime() * images->GetFrameCount();
}

SAnimeSprite *SAnimeSprite::Clone()
{
    if (images) images->AddRef();
    auto clone = new SAnimeSprite(images);
    clone->AddRef();

    clone->CopyParameterFrom(this);

    clone->loopCount = loopCount;
    clone->speed = speed;

    BOOST_ASSERT(clone->GetRefCount() == 1);
    return clone;
}

SAnimeSprite * SAnimeSprite::Factory(SAnimatedImage *image)
{
    if (image) image->AddRef();
    auto result = new SAnimeSprite(image);
    result->AddRef();

    if (image) image->Release();
    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

void SAnimeSprite::RegisterType(asIScriptEngine *engine)
{
    RegisterSpriteBasic<SAnimeSprite>(engine, SU_IF_ANIMESPRITE);
    engine->RegisterObjectBehaviour(SU_IF_ANIMESPRITE, asBEHAVE_FACTORY, SU_IF_ANIMESPRITE "@ f(" SU_IF_ANIMEIMAGE "@)", asFUNCTIONPR(SAnimeSprite::Factory, (SAnimatedImage*), SAnimeSprite*), asCALL_CDECL);
    engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_ANIMESPRITE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SAnimeSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_ANIMESPRITE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SAnimeSprite, SSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_ANIMESPRITE, "void SetSpeed(double)", asMETHOD(SAnimeSprite, SetSpeed), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_ANIMESPRITE, "void SetLoopCount(int)", asMETHOD(SAnimeSprite, SetLoopCount), asCALL_THISCALL);
}

SContainer::SContainer() : SSprite()
{}

SContainer::~SContainer()
{
    for (const auto &s : children) if (s) s->Release();
}

void SContainer::AddChild(SSprite *child)
{
    if (!child) return;
    children.emplace(child);
}

void SContainer::Dismiss()
{
    for (const auto &child : children) {
        child->Dismiss();
    }
    SSprite::Dismiss();
}

void SContainer::Tick(const double delta)
{
    pMover->Tick(delta);
    for (const auto &s : children) s->Tick(delta);
}

void SContainer::Draw(GPU_Target *target)
{
    for (const auto &s : children) s->Draw(Transform, Color, target);
}

void SContainer::Draw(const Transform2D & parent, const ColorTint &color, GPU_Target *target)
{
    const auto tf = Transform.ApplyFrom(parent);
    const auto cl = Color.ApplyFrom(color);
    for (const auto &s : children) s->Draw(tf, cl, target);
}

SContainer *SContainer::Clone()
{
    auto clone = new SContainer();
    clone->AddRef();

    clone->CopyParameterFrom(this);
    for (const auto &s : children) if (s) clone->AddChild(s->Clone());

    BOOST_ASSERT(clone->GetRefCount() == 1);
    return clone;
}

SContainer* SContainer::Factory()
{
    auto result = new SContainer();
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

void SContainer::RegisterType(asIScriptEngine *engine)
{
    RegisterSpriteBasic<SContainer>(engine, SU_IF_CONTAINER);
    engine->RegisterObjectBehaviour(SU_IF_CONTAINER, asBEHAVE_FACTORY, SU_IF_CONTAINER "@ f()", asFUNCTION(SContainer::Factory), asCALL_CDECL);
    engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_CONTAINER "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SContainer>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_CONTAINER, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SContainer, SSprite>)), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectMethod(SU_IF_CONTAINER, "void AddChild(" SU_IF_SPRITE "@)", asMETHOD(SContainer, AddChild), asCALL_THISCALL);
}
 