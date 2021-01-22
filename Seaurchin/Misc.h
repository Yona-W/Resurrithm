#pragma once

// #define BEGIN_DRAW_TRANSACTION(h) SetDrawScreen(h)
// #define FINISH_DRAW_TRANSACTION SetDrawScreen(DX_SCREEN_BACK);

#include <vector>
#include <string>
#include <algorithm>
#include <angelscript.h>
#include <boost/tuple/tuple.hpp>

// AngelScriptに登録した値型用の汎用処理アレ

template <typename T>
void AngelScriptValueConstruct(void *address)
{
    new (address) T;
}

template <typename T>
void AngelScriptValueDestruct(void *address)
{
    static_cast<T*>(address)->~T();
}

template<typename From, typename To>
To* CastReferenceType(From *from)
{
    if (!from) return nullptr;
    To* result = dynamic_cast<To*>(from);
    if (result) result->AddRef();
    return result;
}

using PropList = std::vector<std::tuple<std::string, std::string>>;

std::wstring ConvertUTF8ToUnicode(const std::string &utf8Str);
std::string ConvertUnicodeToUTF8(const std::wstring &utf16Str);
void ScriptSceneWarnOutOf(const std::string &funcName, const std::string &type, asIScriptContext *ctx);
double ToDouble(const char *str);
double NormalizedFmod(double x, double y);
uint32_t ConvertUnsignedInteger(const std::string &input);
int32_t ConvertInteger(const std::string &input);
uint32_t ConvertHexatridecimal(const std::string &input);
float ConvertFloat(const std::string &input);
bool ConvertBoolean(const std::string &input);
void SplitProps(const std::string &source, PropList &vec);

#define SU_TO_INT8(value)   static_cast<int8_t>((value))
#define SU_TO_UINT8(value)  static_cast<uint8_t>((value))
#define SU_TO_INT16(value)  static_cast<int16_t>((value))
#define SU_TO_UINT16(value) static_cast<uint16_t>((value))
#define SU_TO_INT32(value)  static_cast<int32_t>((value))
#define SU_TO_UINT32(value) static_cast<uint32_t>((value))
#define SU_TO_FLOAT(value)  static_cast<float>((value))
#define SU_TO_DOUBLE(value) static_cast<double>((value))

typedef unsigned int   uint;
typedef unsigned short word;
typedef unsigned char  byte;

int UTF16to8( const wchar_t* w, char* s ) {
  uint  c;
  word* p = (word*)w;
  char* q = (char*)s; char* q0 = q;
  while( 1 ) {
    c = *p++;
    if( c==0 ) break;
    if( c<0x080 ) *q++ = c; else 
      if( c<0x800 ) *q++ = 0xC0+(c>>6), *q++ = 0x80+(c&63); else 
        *q++ = 0xE0+(c>>12), *q++ = 0x80+((c>>6)&63), *q++ = 0x80+(c&63);
  }
  *q = 0;
  return q-q0;
}

int UTF8to16( const char* s, wchar_t* w ) {
  uint  cache,wait,c;
  char* p = (char*)s;
  word* q = (word*)w; word* q0 = q;
  while(1) {
    c = *p++;
    if( c==0 ) break;
    if( c<0x80 ) cache=c,wait=0; else
      if( (c>=0xC0) && (c<=0xE0) ) cache=c&31,wait=1; else 
        if( (c>=0xE0) ) cache=c&15,wait=2; else
          if( wait ) (cache<<=6)+=c&63,wait--;
    if( wait==0 ) *q++=cache;
  }
  *q = 0;
  return q-q0;
}