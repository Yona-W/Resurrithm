#include "Debug.h"
#include "Misc.h"

using namespace std;

Logger::Logger()
{

}

void Logger::Initialize()
{
	using namespace spdlog;

#ifdef _DEBUG
	AllocConsole();
	sinks.push_back(make_shared<StandardOutputUnicodeSink>());
#endif
	sinks.push_back(make_shared<sinks::simple_file_sink_mt>("Seaurchin.log", true));
	loggerMain = make_shared<logger>("main", begin(sinks), end(sinks));
	loggerMain->set_pattern("[%H:%M:%S.%e] [%L] %v");
#if _DEBUG
	loggerMain->set_level(level::trace);
#else
	loggerMain->set_level(level::info);
#endif
	register_logger(loggerMain);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void Logger::Terminate() const
{
	spdlog::drop_all();
#ifdef _DEBUG
	FreeConsole();
#endif
}

StandardOutputUnicodeSink::StandardOutputUnicodeSink()
{
	using namespace spdlog::level;
	colors[level_enum::trace] = FOREGROUND_INTENSITY;   // 灰色
	colors[level_enum::debug] = FOREGROUND_INTENSITY;   // 灰色
	colors[level_enum::info] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;  // 白
	colors[level_enum::warn] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;    // 黄色
	colors[level_enum::err] = FOREGROUND_RED | FOREGROUND_INTENSITY;    // 赤
	colors[level_enum::critical] =
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
		| BACKGROUND_RED | BACKGROUND_INTENSITY;    // 赤地に白
	colors[level_enum::off] = 0;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

void StandardOutputUnicodeSink::_sink_it(const spdlog::details::log_msg& msg)
{
	const auto color = colors[msg.level];
	auto u16Msg = ConvertUTF8ToUnicode(msg.formatted.str());
	DWORD written;
	SetConsoleTextAttribute(hStdout, color);
	WriteConsoleW(hStdout, u16Msg.c_str(), u16Msg.length(), &written, nullptr);
	SetConsoleTextAttribute(hStdout, 0);
}
