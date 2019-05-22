#pragma once

#define BEGIN_DRAW_TRANSACTION(h) SetDrawScreen(h)
#define FINISH_DRAW_TRANSACTION SetDrawScreen(DX_SCREEN_BACK);

// AngelScriptに登録した値型用の汎用処理アレ

template <typename T>
void AngelScriptValueConstruct(void* address)
{
	new (address) T;
}

template <typename T>
void AngelScriptValueDestruct(void* address)
{
	static_cast<T*>(address)->~T();
}

template<typename From, typename To>
To* CastReferenceType(From* from)
{
	if (!from) return nullptr;
	To* result = dynamic_cast<To*>(from);
	if (result) result->AddRef();
	return result;
}

using PropList = std::vector<std::tuple<std::string, std::string>>;

std::wstring ConvertUTF8ToUnicode(const std::string& utf8Str);
std::string ConvertUnicodeToUTF8(const std::wstring& utf16Str);
void ScriptSceneWarnOutOf(const std::string& funcName, const std::string& type, asIScriptContext* ctx);
double ToDouble(const char* str);
double NormalizedFmod(double x, double y);
uint32_t ConvertUnsignedInteger(const std::string& input);
int32_t ConvertInteger(const std::string& input);
uint32_t ConvertHexatridecimal(const std::string& input);
float ConvertFloat(const std::string& input);
bool ConvertBoolean(const std::string& input);
void SplitProps(const std::string& source, PropList& vec);

#define SU_TO_INT8(value)   static_cast<int8_t>((value))
#define SU_TO_UINT8(value)  static_cast<uint8_t>((value))
#define SU_TO_INT16(value)  static_cast<int16_t>((value))
#define SU_TO_UINT16(value) static_cast<uint16_t>((value))
#define SU_TO_INT32(value)  static_cast<int32_t>((value))
#define SU_TO_UINT32(value) static_cast<uint32_t>((value))
#define SU_TO_FLOAT(value)  static_cast<float>((value))
#define SU_TO_DOUBLE(value) static_cast<double>((value))

#ifdef _DEBUG 
#define INPLEMENT_REF_COUNTER \
private: \
	int32_t refcount; \
public: \
	void AddRef() { ++refcount; } \
	void Release() { if (--refcount == 0) delete this; } \
	int32_t GetRefCount() const { return refcount; }
#define IS_REFCOUNT(obj, count) ((obj)->GetRefCount() == (count))
#else
#define INPLEMENT_REF_COUNTER \
private: \
	int32_t refcount; \
public: \
	void AddRef() { ++refcount; } \
	void Release() { if (--refcount == 0) delete this; }
#define IS_REFCOUNT(obj, count) (true)
#endif
