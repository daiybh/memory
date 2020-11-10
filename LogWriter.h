//Wrapper based on https://github.com/log4cplus/log4cplus

#pragma once
#include <string>
#include <tchar.h>
#include "Lib.FMT/format.h"
#include "inc/profiler_config.h"
#define SAFEA(str) (SafeA(str).c_str())
#define SAFEW(str) (SafeW(str).c_str())

enum class LOGLEVEL
{
	Debug = 10000,
	Info = 20000,
	Warn = 30000,
	Error = 40000,
	Fatal = 50000,
};

//always use InitializeLogger as the first thing in main();
void InitializeLogger();

//ensures that logger shuts down before the execution leaves the main() function
void ShutdownLogger();

class LoggerInitializer
{
public:
	LoggerInitializer() { InitializeLogger(); }
	~LoggerInitializer() { ShutdownLogger(); }

	LoggerInitializer(LoggerInitializer const&) = delete;
	LoggerInitializer(LoggerInitializer&&) = delete;
	LoggerInitializer& operator =(LoggerInitializer const&) = delete;
	LoggerInitializer& operator =(LoggerInitializer&&) = delete;
};

void WriteLogA(const wchar_t* logPath, LOGLEVEL logLevel, const char* fmt, ...);
void WriteLogW(const wchar_t* logPath, LOGLEVEL logLevel, const wchar_t* fmt, ...);

void WriteLog(const wchar_t* logPath, LOGLEVEL logLevel, std::string_view fmt);
void WriteLog(const wchar_t* logPath, LOGLEVEL logLevel, std::wstring_view fmt);

template <typename T, typename... Args>
void WriteLog(const wchar_t* logPath, LOGLEVEL logLevel, const T* fmt, Args&& ...args) { WriteLog(logPath, logLevel, fmt::format(fmt, std::forward<Args>(args)...)); }

template <typename T, typename... Args>
void info_log(const wchar_t* logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Info, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void info_log(const wchar_t* logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Info, fmt); }
inline void info_log(const wchar_t* logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Info, fmt); }

template <typename T, typename... Args>
void warn_log(const wchar_t* logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Warn, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void warn_log(const wchar_t* logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Warn, fmt); }
inline void warn_log(const wchar_t* logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Warn, fmt); }

template <typename T, typename... Args>
void error_log(const wchar_t* logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Error, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void error_log(const wchar_t* logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Error, fmt); }
inline void error_log(const wchar_t* logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Error, fmt); }

template <typename T, typename... Args>
void fatal_log(const wchar_t* logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Fatal, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void fatal_log(const wchar_t* logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Fatal, fmt); }
inline void fatal_log(const wchar_t* logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Fatal, fmt); }

template <typename T, typename... Args>
void debug_log(const wchar_t* logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Debug, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void debug_log(const wchar_t* logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Debug, fmt); }
inline void debug_log(const wchar_t* logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Debug, fmt); }

std::string  SafeA(char* buf);
std::string  SafeA(const std::string& buf);
std::wstring SafeW(wchar_t* buf);
std::wstring SafeW(const std::wstring& buf);

#define LOG_currentThreadID {SetThreadDescription(GetCurrentThread(), __FUNCTIONW__);WriteLog(L"C:\\Logs\\frame2TCP\\ThreadInfo.log",LOGLEVEL::Info,"{0} @Line:{1:5d} file:{2}",__FUNCTION__,__LINE__,__FILE__);}
