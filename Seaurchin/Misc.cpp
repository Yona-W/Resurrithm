#include "Misc.h"

using namespace std;

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

/*
wstring ConvertUTF8ToUnicode(const string &utf8Str)
{
    auto len = utf8Str.length();
    const auto buffer = new wchar_t[len];
    const char *input = utf8Str.c_str();
    UTF8to16(input, buffer);
    wstring ret = buffer;
    delete[] buffer;
    return ret;
}

string ConvertUnicodeToUTF8(const wstring &utf16Str)
{
    auto len = utf16Str.length();
    const auto buffer = new char[len];
    const wchar_t *input = utf16Str.c_str();
    UTF16to8(input, buffer);
    string ret = buffer;
    delete[] buffer;
    return ret;
}
*/

void ScriptSceneWarnOutOf(const string &funcName, const string &type, asIScriptContext *ctx)
{
    const char *secn;
    int col;
    const auto row = ctx->GetLineNumber(0, &col, &secn);
    ctx->GetEngine()->WriteMessage(secn, row, col, asMSGTYPE_WARNING, ("You can call \"" + funcName + "\" Function only from " + type + "!").c_str());
}

double ToDouble(const char *str)
{
    auto result = 0.0, base = 1.0;
    auto sign = 1;
    auto it = str;
    unsigned char ch;

    it = *it == '-' ? (sign = -sign, ++it) : it;
    while (static_cast<unsigned char>(ch = *(it++) - '0') <= 9) result = result * 10 + ch;
    if (*(--it) == '.') while (static_cast<unsigned char>(ch = *(++it) - '0') <= 9) result += (base *= 0.1) * ch;
    return sign * result;
}

double NormalizedFmod(const double x, double y)
{
    if (y < 0) y = -y;
    const int q = SU_TO_INT32(x >= 0 ? x / y : (x / y) - 1);
    return x - q * y;
}

uint32_t ConvertUnsignedInteger(const string &input)
{
    return SU_TO_UINT32(atoi(input.c_str()));
}

int32_t ConvertInteger(const string &input)
{
    return atoi(input.c_str());
}

uint32_t ConvertHexatridecimal(const string &input)
{
    return stoul(input, nullptr, 36);
}

float ConvertFloat(const string &input)
{
    return SU_TO_FLOAT(ToDouble(input.c_str()));
}

bool ConvertBoolean(const string &input)
{
    const auto toLower = [](const char c) { return char(tolower(c)); };
    auto test = input;
    std::transform(test.cbegin(), test.cend(), test.begin(), toLower);
    return
        input == "1"
        || input == "true"
        || input == "y"
        || input == "yes"
        || input == "enable"
        || input == "enabled";
}

void SplitProps(const string &source, PropList &vec)
{
    auto now = 0;
    string pset;
    while (true) {
        const int end = source.find(',', now);
        if (end == string::npos) break;

        pset = source.substr(now, end - now);
        const auto pos = pset.find(':');
        if (pos == string::npos) continue;
        vec.push_back(make_tuple(pset.substr(0, pos), pset.substr(pos + 1)));
        now = end + 1;
    }
    pset = source.substr(now);
    const auto pos = pset.find(':');
    if (pos == string::npos) return;
    vec.push_back(make_tuple(pset.substr(0, pos), pset.substr(pos + 1)));
}
