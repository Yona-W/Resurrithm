#pragma once

#define BEGIN_DRAW_TRANSACTION(h) SetDrawScreen(h)
#define FINISH_DRAW_TRANSACTION SetDrawScreen(DX_SCREEN_BACK);

// AngelScriptに登録した値型用の汎用処理アレ

template <typename T, typename... Args>
void AngelScriptValueConstruct(void* address, Args... args)
{
	new (address) T (args...);
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
double ConvertDouble(const std::string& input);
float ConvertFloat(const std::string& input);
bool ConvertBoolean(const std::string& input);
void SplitProps(const std::string& source, PropList& vec);

#define SU_TO_INT8(value)   static_cast<int8_t>((value))
#define SU_TO_UINT8(value)  static_cast<uint8_t>((value))
#define SU_TO_INT16(value)  static_cast<int16_t>((value))
#define SU_TO_UINT16(value) static_cast<uint16_t>((value))
#define SU_TO_INT32(value)  static_cast<int32_t>((value))
#define SU_TO_UINT32(value) static_cast<uint32_t>((value))
#define SU_TO_INT64(value)  static_cast<int64_t>((value))
#define SU_TO_UINT64(value) static_cast<uint64_t>((value))
#define SU_TO_INT(value)  static_cast<int>((value))
#define SU_TO_UINT(value) static_cast<unsigned int>((value))
#define SU_TO_FLOAT(value)  static_cast<float>((value))
#define SU_TO_DOUBLE(value) static_cast<double>((value))
#define SU_TO_WORD(value) static_cast<WORD>((value))
#define SU_TO_DWORD(value) static_cast<DWORD>((value))

#define SU_INT8_MAX (std::numeric_limits<int8_t>::max())
#define SU_UINT8_MAX (std::numeric_limits<uint8_t>::max())
#define SU_INT16_MAX (std::numeric_limits<int16_t>::max())
#define SU_UINT16_MAX (std::numeric_limits<uint16_t>::max())
#define SU_INT32_MAX (std::numeric_limits<int32_t>::max())
#define SU_UINT32_MAX (std::numeric_limits<uint32_t>::max())
#define SU_INT64_MAX (std::numeric_limits<int64_t>::max())
#define SU_UINT64_MAX (std::numeric_limits<uint64_t>::max())
#define SU_INT_MAX (std::numeric_limits<int>::max())
#define SU_UINT_MAX (std::numeric_limits<unsigned int>::max())
#define SU_FLOAT_MAX (std::numeric_limits<float>::max())
#define SU_DOUBLE_MAX (std::numeric_limits<doube>::max())
#define SU_WORD_MAX (std::numeric_limits<WORD>::max())
#define SU_DWORD_MAX (std::numeric_limits<DWORD>::max())

#define SU_INT8_MIN (std::numeric_limits<int8_t>::min())
#define SU_UINT8_MIN (std::numeric_limits<uint8_t>::min())
#define SU_INT16_MIN (std::numeric_limits<int16_t>::min())
#define SU_UINT16_MIN (std::numeric_limits<uint16_t>::min())
#define SU_INT32_MIN (std::numeric_limits<int32_t>::min())
#define SU_UINT32_MIN (std::numeric_limits<uint32_t>::min())
#define SU_INT64_MIN (std::numeric_limits<int64_t>::min())
#define SU_UINT64_MIN (std::numeric_limits<uint64_t>::min())
#define SU_INT_MIN (std::numeric_limits<int>::min())
#define SU_UINT_MIN (std::numeric_limits<unsigned int>::min())
#define SU_FLOAT_MIN (std::numeric_limits<float>::min())
#define SU_DOUBLE_MIN (std::numeric_limits<doube>::min())
#define SU_WORD_MIN (std::numeric_limits<WORD>::min())
#define SU_DWORD_MIN (std::numeric_limits<DWORD>::min())

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
