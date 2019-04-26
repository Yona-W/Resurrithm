#pragma once

class Logger {
private:
	std::vector<spdlog::sink_ptr> sinks;
	std::shared_ptr<spdlog::logger> loggerMain;

public:
	Logger();

	void Initialize();
	void Terminate() const;

	void LogTrace(const std::string& message) const { loggerMain->trace(message); }
	void LogDebug(const std::string& message) const { loggerMain->debug(message); }
	void LogInfo(const std::string& message) const { loggerMain->info(message); }
	void LogWarn(const std::string& message) const { loggerMain->warn(message); }
	void LogError(const std::string& message) const { loggerMain->error(message); }
	void LogCritical(const std::string& message) const { loggerMain->critical(message); }
};

class StandardOutputUnicodeSink : public spdlog::sinks::base_sink<std::mutex> {
private:
	HANDLE hStdout;
	std::map<spdlog::level::level_enum, WORD> colors;

protected:
	void _sink_it(const spdlog::details::log_msg& msg) override;
	virtual void _flush() override {}

public:
	StandardOutputUnicodeSink();
};
