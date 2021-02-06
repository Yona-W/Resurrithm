#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <rang.hpp>

class Logger {
private:
    std::vector<spdlog::sink_ptr> sinks;
    std::shared_ptr<spdlog::logger> loggerMain;

public:
    Logger();

    void Initialize();
    void Terminate() const;

    /*

    void LogTrace(const std::string &message) const { loggerMain->trace(message); }
    void LogDebug(const std::string &message) const { loggerMain->debug(message); }
    void LogInfo(const std::string &message) const { loggerMain->info(message); }
    void LogWarn(const std::string &message) const { loggerMain->warn(message); }
    void LogError(const std::string &message) const { loggerMain->error(message); }
    void LogCritical(const std::string &message) const { loggerMain->critical(message); }

    */

    void LogTrace(const std::string &message) const { std::cout << "[TRC] " << message << "\n"; }
    void LogDebug(const std::string &message) const { std::cout << "[DBG] " << message << "\n"; }
    void LogInfo(const std::string &message) const { std::cout << "[INF] " << message << "\n"; }
    void LogWarn(const std::string &message) const { std::cout << "[WRN] " << message << "\n"; }
    void LogError(const std::string &message) const { std::cout << "[ERR] " << message << "\n"; }
    void LogCritical(const std::string &message) const { std::cout << "[CRT] " << message << "\n"; }
};

class StandardOutputUnicodeSink : public spdlog::sinks::base_sink<std::mutex> {
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    virtual void flush_() override {}
public:
    StandardOutputUnicodeSink();
};
