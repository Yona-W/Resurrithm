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
    sinks.push_back(make_shared<sinks::basic_file_sink_mt>("Seaurchin.log", true));
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
}

void StandardOutputUnicodeSink::sink_it_(const spdlog::details::log_msg & msg)
{
    using namespace rang;
    switch(msg.level){
        case spdlog::level::trace:
            break;
        case spdlog::level::debug:
            break;
        case spdlog::level::info:
            cout << fg::gray;
            break;
        case spdlog::level::warn:
            cout << fg::yellow;
            break;
        case spdlog::level::err:
            cout << fg::red << style::bold;
            break;
        case spdlog::level::critical:
            cout << fg::gray << bg::red << style::bold;
            break;
    }

    cout << msg.payload.data() << fg::reset << bg::reset << style::reset;
}
