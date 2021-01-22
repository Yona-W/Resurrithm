#pragma once

#include <cmath>

struct Transform2D {
    float X = 0.0f;
    float Y = 0.0f;
    float Angle = 0.0f;
    float OriginX = 0.0f;
    float OriginY = 0.0f;
    float ScaleX = 1.0f;
    float ScaleY = 1.0f;
    Transform2D ApplyFrom(const Transform2D &parent) const
    {
        Transform2D result;
        // Originは画像上の位置なので変更なし
        result.OriginX = OriginX;
        result.OriginY = OriginY;
        // 正しくないのは百も承知だが許してくれ
        result.ScaleX = parent.ScaleX * ScaleX;
        result.ScaleY = parent.ScaleY * ScaleY;
        // ここからは正しいと思う
        result.Angle = parent.Angle + Angle;
        const auto rx = X * parent.ScaleX;
        const auto ry = Y * parent.ScaleY;
        result.X = parent.X + (rx * cos(parent.Angle) - ry * sin(parent.Angle));
        result.Y = parent.Y + (rx * sin(parent.Angle) + ry * cos(parent.Angle));
        return result;
    }
};

struct Transform3D {
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float AngleX = 0.0f;
    float AngleY = 0.0f;
    float AngleZ = 0.0f;
};

struct ColorTint {
    unsigned char A;
    unsigned char R;
    unsigned char G;
    unsigned char B;
    ColorTint ApplyFrom(const ColorTint &parent) const
    {
        const auto na = (int(A) * int(parent.A)) / 255.0;
        const auto nr = (int(R) * int(parent.R)) / 255.0;
        const auto ng = (int(G) * int(parent.G)) / 255.0;
        const auto nb = (int(B) * int(parent.B)) / 255.0;
        const ColorTint result = {
            static_cast<unsigned char>(na),
            static_cast<unsigned char>(nr),
            static_cast<unsigned char>(ng),
            static_cast<unsigned char>(nb),
        };
        return result;
    }
};

class Colors final {
public:
    static constexpr ColorTint aliceBlue = { 0xFF, 0xF0, 0xF8, 0xFF };
    static constexpr ColorTint antiqueWhite = { 0xFF, 0xFA, 0xEB, 0xD7 };
    static constexpr ColorTint aqua = { 0xFF, 0x00, 0xFF, 0xFF };
    static constexpr ColorTint aquamarine = { 0xFF, 0x7F, 0xFF, 0xD4 };
    static constexpr ColorTint azure = { 0xFF, 0xF0, 0xFF, 0xFF };
    static constexpr ColorTint beige = { 0xFF, 0xF5, 0xF5, 0xDC };
    static constexpr ColorTint bisque = { 0xFF, 0xFF, 0xE4, 0xC4 };
    static constexpr ColorTint black = { 0xFF, 0x00, 0x00, 0x00 };
    static constexpr ColorTint blanchedAlmond = { 0xFF, 0xFF, 0xEB, 0xCD };
    static constexpr ColorTint blue = { 0xFF, 0x00, 0x00, 0xFF };
    static constexpr ColorTint blueViolet = { 0xFF, 0x8A, 0x2B, 0xE2 };
    static constexpr ColorTint brown = { 0xFF, 0xA5, 0x2A, 0x2A };
    static constexpr ColorTint burlyWood = { 0xFF, 0xDE, 0xB8, 0x87 };
    static constexpr ColorTint cadetBlue = { 0xFF, 0x5F, 0x9E, 0xA0 };
    static constexpr ColorTint chartreuse = { 0xFF, 0x7F, 0xFF, 0x00 };
    static constexpr ColorTint chocolate = { 0xFF, 0xD2, 0x69, 0x1E };
    static constexpr ColorTint coral = { 0xFF, 0xFF, 0x7F, 0x50 };
    static constexpr ColorTint cornflowerBlue = { 0xFF, 0x64, 0x95, 0xED };
    static constexpr ColorTint cornsilk = { 0xFF, 0xFF, 0xF8, 0xDC };
    static constexpr ColorTint crimson = { 0xFF, 0xDC, 0x14, 0x3C };
    static constexpr ColorTint cyan = { 0xFF, 0x00, 0xFF, 0xFF };
    static constexpr ColorTint darkBlue = { 0xFF, 0x00, 0x00, 0x8B };
    static constexpr ColorTint darkCyan = { 0xFF, 0x00, 0x8B, 0x8B };
    static constexpr ColorTint darkGoldenrod = { 0xFF, 0xB8, 0x86, 0x0B };
    static constexpr ColorTint darkGray = { 0xFF, 0xA9, 0xA9, 0xA9 };
    static constexpr ColorTint darkGreen = { 0xFF, 0x00, 0x64, 0x00 };
    static constexpr ColorTint darkKhaki = { 0xFF, 0xBD, 0xB7, 0x6B };
    static constexpr ColorTint darkMagenta = { 0xFF, 0x8B, 0x00, 0x8B };
    static constexpr ColorTint darkOliveGreen = { 0xFF, 0x55, 0x6B, 0x2F };
    static constexpr ColorTint darkOrange = { 0xFF, 0xFF, 0x8C, 0x00 };
    static constexpr ColorTint darkOrchid = { 0xFF, 0x99, 0x32, 0xCC };
    static constexpr ColorTint darkRed = { 0xFF, 0x8B, 0x00, 0x00 };
    static constexpr ColorTint darkSalmon = { 0xFF, 0xE9, 0x96, 0x7A };
    static constexpr ColorTint darkSeaGreen = { 0xFF, 0x8F, 0xBC, 0x8F };
    static constexpr ColorTint darkSlateBlue = { 0xFF, 0x48, 0x3D, 0x8B };
    static constexpr ColorTint darkSlateGray = { 0xFF, 0x2F, 0x4F, 0x4F };
    static constexpr ColorTint darkTurquoise = { 0xFF, 0x00, 0xCE, 0xD1 };
    static constexpr ColorTint darkViolet = { 0xFF, 0x94, 0x00, 0xD3 };
    static constexpr ColorTint deepPink = { 0xFF, 0xFF, 0x14, 0x93 };
    static constexpr ColorTint deepSkyBlue = { 0xFF, 0x00, 0xBF, 0xFF };
    static constexpr ColorTint dimGray = { 0xFF, 0x69, 0x69, 0x69 };
    static constexpr ColorTint dodgerBlue = { 0xFF, 0x1E, 0x90, 0xFF };
    static constexpr ColorTint firebrick = { 0xFF, 0xB2, 0x22, 0x22 };
    static constexpr ColorTint floralWhite = { 0xFF, 0xFF, 0xFA, 0xF0 };
    static constexpr ColorTint forestGreen = { 0xFF, 0x22, 0x8B, 0x22 };
    static constexpr ColorTint fuchsia = { 0xFF, 0xFF, 0x00, 0xFF };
    static constexpr ColorTint gainsboro = { 0xFF, 0xDC, 0xDC, 0xDC };
    static constexpr ColorTint ghostWhite = { 0xFF, 0xF8, 0xF8, 0xFF };
    static constexpr ColorTint gold = { 0xFF, 0xFF, 0xD7, 0x00 };
    static constexpr ColorTint goldenrod = { 0xFF, 0xDA, 0xA5, 0x20 };
    static constexpr ColorTint gray = { 0xFF, 0x80, 0x80, 0x80 };
    static constexpr ColorTint green = { 0xFF, 0x00, 0x80, 0x00 };
    static constexpr ColorTint greenYellow = { 0xFF, 0xAD, 0xFF, 0x2F };
    static constexpr ColorTint honeydew = { 0xFF, 0xF0, 0xFF, 0xF0 };
    static constexpr ColorTint hotPink = { 0xFF, 0xFF, 0x69, 0xB4 };
    static constexpr ColorTint indianRed = { 0xFF, 0xCD, 0x5C, 0x5C };
    static constexpr ColorTint indigo = { 0xFF, 0x4B, 0x00, 0x82 };
    static constexpr ColorTint ivory = { 0xFF, 0xFF, 0xFF, 0xF0 };
    static constexpr ColorTint khaki = { 0xFF, 0xF0, 0xE6, 0x8C };
    static constexpr ColorTint lavender = { 0xFF, 0xE6, 0xE6, 0xFA };
    static constexpr ColorTint lavenderBlush = { 0xFF, 0xFF, 0xF0, 0xF5 };
    static constexpr ColorTint lawnGreen = { 0xFF, 0x7C, 0xFC, 0x00 };
    static constexpr ColorTint lemonChiffon = { 0xFF, 0xFF, 0xFA, 0xCD };
    static constexpr ColorTint lightBlue = { 0xFF, 0xAD, 0xD8, 0xE6 };
    static constexpr ColorTint lightCoral = { 0xFF, 0xF0, 0x80, 0x80 };
    static constexpr ColorTint lightCyan = { 0xFF, 0xE0, 0xFF, 0xFF };
    static constexpr ColorTint lightGoldenrodYellow = { 0xFF, 0xFA, 0xFA, 0xD2 };
    static constexpr ColorTint lightGray = { 0xFF, 0xD3, 0xD3, 0xD3 };
    static constexpr ColorTint lightGreen = { 0xFF, 0x90, 0xEE, 0x90 };
    static constexpr ColorTint lightPink = { 0xFF, 0xFF, 0xB6, 0xC1 };
    static constexpr ColorTint lightSalmon = { 0xFF, 0xFF, 0xA0, 0x7A };
    static constexpr ColorTint lightSeaGreen = { 0xFF, 0x20, 0xB2, 0xAA };
    static constexpr ColorTint lightSkyBlue = { 0xFF, 0x87, 0xCE, 0xFA };
    static constexpr ColorTint lightSlateGray = { 0xFF, 0x77, 0x88, 0x99 };
    static constexpr ColorTint lightSteelBlue = { 0xFF, 0xB0, 0xC4, 0xDE };
    static constexpr ColorTint lightYellow = { 0xFF, 0xFF, 0xFF, 0xE0 };
    static constexpr ColorTint lime = { 0xFF, 0x00, 0xFF, 0x00 };
    static constexpr ColorTint limeGreen = { 0xFF, 0x32, 0xCD, 0x32 };
    static constexpr ColorTint linen = { 0xFF, 0xFA, 0xF0, 0xE6 };
    static constexpr ColorTint magenta = { 0xFF, 0xFF, 0x00, 0xFF };
    static constexpr ColorTint maroon = { 0xFF, 0x80, 0x00, 0x00 };
    static constexpr ColorTint mediumAquamarine = { 0xFF, 0x66, 0xCD, 0xAA };
    static constexpr ColorTint mediumBlue = { 0xFF, 0x00, 0x00, 0xCD };
    static constexpr ColorTint mediumOrchid = { 0xFF, 0xBA, 0x55, 0xD3 };
    static constexpr ColorTint mediumPurple = { 0xFF, 0x93, 0x70, 0xDB };
    static constexpr ColorTint mediumSeaGreen = { 0xFF, 0x3C, 0xB3, 0x71 };
    static constexpr ColorTint mediumSlateBlue = { 0xFF, 0x7B, 0x68, 0xEE };
    static constexpr ColorTint mediumSpringGreen = { 0xFF, 0x00, 0xFA, 0x9A };
    static constexpr ColorTint mediumTurquoise = { 0xFF, 0x48, 0xD1, 0xCC };
    static constexpr ColorTint mediumVioletRed = { 0xFF, 0xC7, 0x15, 0x85 };
    static constexpr ColorTint midnightBlue = { 0xFF, 0x19, 0x19, 0x70 };
    static constexpr ColorTint mintCream = { 0xFF, 0xF5, 0xFF, 0xFA };
    static constexpr ColorTint mistyRose = { 0xFF, 0xFF, 0xE4, 0xE1 };
    static constexpr ColorTint moccasin = { 0xFF, 0xFF, 0xE4, 0xB5 };
    static constexpr ColorTint navajoWhite = { 0xFF, 0xFF, 0xDE, 0xAD };
    static constexpr ColorTint navy = { 0xFF, 0x00, 0x00, 0x80 };
    static constexpr ColorTint oldLace = { 0xFF, 0xFD, 0xF5, 0xE6 };
    static constexpr ColorTint olive = { 0xFF, 0x80, 0x80, 0x00 };
    static constexpr ColorTint oliveDrab = { 0xFF, 0x6B, 0x8E, 0x23 };
    static constexpr ColorTint orange = { 0xFF, 0xFF, 0xA5, 0x00 };
    static constexpr ColorTint orangeRed = { 0xFF, 0xFF, 0x45, 0x00 };
    static constexpr ColorTint orchid = { 0xFF, 0xDA, 0x70, 0xD6 };
    static constexpr ColorTint paleGoldenrod = { 0xFF, 0xEE, 0xE8, 0xAA };
    static constexpr ColorTint paleGreen = { 0xFF, 0x98, 0xFB, 0x98 };
    static constexpr ColorTint paleTurquoise = { 0xFF, 0xAF, 0xEE, 0xEE };
    static constexpr ColorTint paleVioletRed = { 0xFF, 0xDB, 0x70, 0x93 };
    static constexpr ColorTint papayaWhip = { 0xFF, 0xFF, 0xEF, 0xD5 };
    static constexpr ColorTint peachPuff = { 0xFF, 0xFF, 0xDA, 0xB9 };
    static constexpr ColorTint peru = { 0xFF, 0xCD, 0x85, 0x3F };
    static constexpr ColorTint pink = { 0xFF, 0xFF, 0xC0, 0xCB };
    static constexpr ColorTint plum = { 0xFF, 0xDD, 0xA0, 0xDD };
    static constexpr ColorTint powderBlue = { 0xFF, 0xB0, 0xE0, 0xE6 };
    static constexpr ColorTint purple = { 0xFF, 0x80, 0x00, 0x80 };
    static constexpr ColorTint red = { 0xFF, 0xFF, 0x00, 0x00 };
    static constexpr ColorTint rosyBrown = { 0xFF, 0xBC, 0x8F, 0x8F };
    static constexpr ColorTint royalBlue = { 0xFF, 0x41, 0x69, 0xE1 };
    static constexpr ColorTint saddleBrown = { 0xFF, 0x8B, 0x45, 0x13 };
    static constexpr ColorTint salmon = { 0xFF, 0xFA, 0x80, 0x72 };
    static constexpr ColorTint sandyBrown = { 0xFF, 0xF4, 0xA4, 0x60 };
    static constexpr ColorTint seaGreen = { 0xFF, 0x2E, 0x8B, 0x57 };
    static constexpr ColorTint seaShell = { 0xFF, 0xFF, 0xF5, 0xEE };
    static constexpr ColorTint sienna = { 0xFF, 0xA0, 0x52, 0x2D };
    static constexpr ColorTint silver = { 0xFF, 0xC0, 0xC0, 0xC0 };
    static constexpr ColorTint skyBlue = { 0xFF, 0x87, 0xCE, 0xEB };
    static constexpr ColorTint slateBlue = { 0xFF, 0x6A, 0x5A, 0xCD };
    static constexpr ColorTint slateGray = { 0xFF, 0x70, 0x80, 0x90 };
    static constexpr ColorTint snow = { 0xFF, 0xFF, 0xFA, 0xFA };
    static constexpr ColorTint springGreen = { 0xFF, 0x00, 0xFF, 0x7F };
    static constexpr ColorTint steelBlue = { 0xFF, 0x46, 0x82, 0xB4 };
    static constexpr ColorTint tan = { 0xFF, 0xD2, 0xB4, 0x8C };
    static constexpr ColorTint teal = { 0xFF, 0x00, 0x80, 0x80 };
    static constexpr ColorTint thistle = { 0xFF, 0xD8, 0xBF, 0xD8 };
    static constexpr ColorTint tomato = { 0xFF, 0xFF, 0x63, 0x47 };
    static constexpr ColorTint turquoise = { 0xFF, 0x40, 0xE0, 0xD0 };
    static constexpr ColorTint violet = { 0xFF, 0xEE, 0x82, 0xEE };
    static constexpr ColorTint wheat = { 0xFF, 0xF5, 0xDE, 0xB3 };
    static constexpr ColorTint white = { 0xFF, 0xFF, 0xFF, 0xFF };
    static constexpr ColorTint whiteSmoke = { 0xFF, 0xF5, 0xF5, 0xF5 };
    static constexpr ColorTint yellow = { 0xFF, 0xFF, 0xFF, 0x00 };
    static constexpr ColorTint yellowGreen = { 0xFF, 0x9A, 0xCD, 0x32 };
};
