#pragma once
#include <memory>
#include <utility>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

enum class LogLevel
{
	Trace = spdlog::level::trace,
	Debug = spdlog::level::debug,
	Info = spdlog::level::info,
	Warn = spdlog::level::warn,
	Err = spdlog::level::err,
};

class Log
{
public:
	static const std::shared_ptr<spdlog::logger>& SPDLogger() noexcept;

	static void Init() noexcept;

	template<typename... Args>
	static void Message(LogLevel level, spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		SPDLogger()->log(static_cast<spdlog::level::level_enum>(level), fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Trace(spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		SPDLogger()->trace(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		SPDLogger()->debug(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Info(spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		SPDLogger()->info(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		SPDLogger()->warn(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void Error(spdlog::format_string_t<Args...> fmt, Args&&... args)
	{
		SPDLogger()->error(fmt, std::forward<Args>(args)...);
	}

private:
	static std::shared_ptr<spdlog::logger> s_Logger;
};

inline std::shared_ptr<spdlog::logger> Log::s_Logger;

inline const std::shared_ptr<spdlog::logger>& Log::SPDLogger() noexcept
{
	if (!s_Logger)
	{
		Init();
	}
	return s_Logger;
}

inline void Log::Init() noexcept
{
	if (s_Logger)
	{
		return;
	}

	// %^   -> Start color region (for level)
	// [%T] -> Time (HH:MM:SS)
	// [%n] -> Logger name
	// [%l] -> Log level (info, warn, error, etc.)
	// %v   -> The actual log message
	// %$   -> End color region
	spdlog::set_pattern("%^[%T] [%n] [%l] %v%$");

	s_Logger = spdlog::get("Core");
	if (!s_Logger)
	{
		s_Logger = spdlog::stdout_color_mt("Core");
	}
	s_Logger->set_level(spdlog::level::trace);
}
