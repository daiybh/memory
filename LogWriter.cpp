#include <Windows.h>
#include "LogWriter.h"
#include "log4cplus/logger.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/asyncappender.h"
#include "log4cplus/helpers/appenderattachableimpl.h"
#include "log4cplus/layout.h"
#include "log4cplus/log4cplus.h"
#include <codecvt>
#include <filesystem>

using namespace log4cplus;
using namespace helpers;
using namespace spi;
#define LOGSIZE         (2*1024)

static long log_size = 0;
static int  log_number = 0;
static bool async_log = true;
static bool enable_debug_log = false;
static std::wstring loggerServerHost = L"";
void load_log_config()
{
	static constexpr wchar_t const* path = LR"(C:\ProgramData\SimplyLive.TV\Vibox\Backend\LogConfig.ini)";
	log_size = GetPrivateProfileIntW(L"Log", L"size_per_file_MB", 20, path);
	log_size *= (1024 * 1024);
	log_number = GetPrivateProfileIntW(L"Log", L"file_num", 5, path);
	enable_debug_log = GetPrivateProfileIntW(L"Log", L"EnableDebug", false, path);
	async_log = GetPrivateProfileIntW(L"Log", L"Async", true, path);

	wchar_t s[1024];
	GetPrivateProfileStringW(L"Log", L"serverHost", L"", s, 1024, path);
	loggerServerHost = s;
}

void add_RollingFileAppender(Logger& logger, const wchar_t* logPath)
{
	auto* rollingFileAppender = new RollingFileAppender(logPath, log_size, log_number, true, true);
	rollingFileAppender->setName(logPath);
	tstring pattern = LOG4CPLUS_TEXT("%D{%y/%m/%d %H:%M:%S.%q} [%-5t][%-5p]- %m%n");
	rollingFileAppender->setLayout(std::unique_ptr<Layout>(std::make_unique<PatternLayout>(pattern)));
	rollingFileAppender->imbue(std::locale(rollingFileAppender->getloc(),
		new std::codecvt_utf8<tchar, 0x10FFFF, static_cast<std::codecvt_mode>(std::consume_header | std::little_endian)>));

	const SharedAppenderPtr smart_ptr(rollingFileAppender);
	SharedAppenderPtr       appenderPtr;
	if (async_log)
		appenderPtr = new AsyncAppender(smart_ptr, 100);
	else
		appenderPtr = smart_ptr;

	logger.addAppender(appenderPtr);
}

void add_SocketAppender(Logger& logger, const wchar_t* logPath)
{
	if (loggerServerHost.empty())return;
	SharedAppenderPtr append_1(new SocketAppender(loggerServerHost, 9998, LOG4CPLUS_TEXT("")));
	append_1->setName(logPath);
	logger.addAppender(append_1);
}

Logger get_logger(const wchar_t* logPath)
{
	static std::map<std::wstring, Logger> logger_list;
	static std::shared_mutex locker;

	{//here can be Concurrency
		std::shared_lock<std::shared_mutex> lock(locker);
		const auto iter = logger_list.find(logPath);
		if (iter != logger_list.end())
			return iter->second;
	}

	std::lock_guard<std::shared_mutex> lock(locker);
	const auto iter = logger_list.find(logPath);
	if (iter != logger_list.end())
		return iter->second;

	static std::once_flag flag;
	std::call_once(flag, [&] { load_log_config(); });

	Logger logger = Logger::getInstance(logPath);
	add_RollingFileAppender(logger, logPath);
	logger_list.emplace(logPath, logger);
	return logger;
}

void internal_writer(const wchar_t* logPath, LOGLEVEL level, std::wstring_view log)
{
	if (logPath == nullptr)
		return;

	Logger logger = get_logger(logPath);
	switch (level)
	{
	case LOGLEVEL::Debug:	LOG4CPLUS_DEBUG(logger, log);	break;
	case LOGLEVEL::Warn:	LOG4CPLUS_WARN(logger, log);	break;
	case LOGLEVEL::Error:	LOG4CPLUS_ERROR(logger, log);	break;
	case LOGLEVEL::Fatal:	LOG4CPLUS_FATAL(logger, log);	break;
	case LOGLEVEL::Info:
	default:				LOG4CPLUS_INFO(logger, log);	break;
	}
}

void WriteLogW(const wchar_t* logPath, LOGLEVEL logLevel, const wchar_t* fmt, ...)
{
	if (logLevel == LOGLEVEL::Debug)
	{
		if (!enable_debug_log)
			return;
	}

	TCHAR szBuffer[LOGSIZE];
	va_list marker;
	va_start(marker, fmt);
	const int ret = _vsntprintf_s(szBuffer, std::size(szBuffer), _TRUNCATE, fmt, marker);
	va_end(marker);

	if (ret < 0)
		return;

	internal_writer(logPath, logLevel, szBuffer);
}

void WriteLog(const wchar_t* logPath, LOGLEVEL logLevel, std::wstring_view fmt)
{
	if (logLevel == LOGLEVEL::Debug)
	{
		if (!enable_debug_log)
			return;
	}
	internal_writer(logPath, logLevel, fmt);
}

void InitializeLogger()
{
	initialize();
}

void WriteLogA(const wchar_t* logPath, LOGLEVEL logLevel, const char* fmt, ...)
{
	if (logLevel == LOGLEVEL::Debug)
	{
		if (!enable_debug_log)
			return;
	}

	va_list marker;
	char szBuffer[LOGSIZE / 2] = { 0 };

	va_start(marker, fmt);
	int ret = _vsnprintf_s(szBuffer, std::size(szBuffer), _TRUNCATE, fmt, marker);
	va_end(marker);

	if (ret < 0)
		return;

	wchar_t wLogBuf[LOGSIZE];
	size_t size = 0;
	if (mbstowcs_s(&size, wLogBuf, szBuffer, strlen(szBuffer)) == 0)
		WriteLog(logPath, logLevel, std::wstring_view(wLogBuf));
}

void WriteLog(const wchar_t* logPath, LOGLEVEL logLevel, std::string_view fmt)
{
	if (logLevel == LOGLEVEL::Debug)
	{
		if (!enable_debug_log)
			return;
	}
	wchar_t wLogBuf[LOGSIZE];
	size_t size = 0;
	if (mbstowcs_s(&size, wLogBuf, fmt.data(), fmt.size()) == 0)
		WriteLog(logPath, logLevel, std::wstring_view(wLogBuf));
}

void ShutdownLogger()
{
	deinitialize();
}

std::string SafeA(char* buf)
{
	if (buf == nullptr)
		return "";

	std::string s1(buf);
	std::string::size_type pos = 0;
	std::string_view toPlace("%%");
	std::string_view toFind("%");
	while ((pos = s1.find(toFind, pos)) != std::string::npos)
	{
		s1.replace(pos, 1, toPlace);
		pos += 2;
	}
	return s1;
}

std::string SafeA(const std::string& buf)
{
	if (buf.empty())
		return "";

	std::string s1(buf);
	std::string::size_type pos = 0;
	std::string_view toPlace("%%");
	std::string_view toFind("%");
	while ((pos = s1.find(toFind, pos)) != std::string::npos)
	{
		s1.replace(pos, 1, toPlace);
		pos += 2;
	}
	return s1;
}

std::wstring SafeW(wchar_t* buf)
{
	if (buf == nullptr)
		return L"";

	std::wstring s1(buf);
	std::wstring::size_type pos = 0;
	std::wstring_view toPlace(L"%%");
	std::wstring_view toFind(L"%");
	while ((pos = s1.find(toFind, pos)) != std::wstring::npos)
	{
		s1.replace(pos, 1, toPlace);
		pos += 2;
	}
	return s1;
}

std::wstring SafeW(const std::wstring& buf)
{
	if (buf.empty())
		return L"";

	std::wstring s1(buf);
	std::wstring::size_type pos = 0;
	std::wstring_view toPlace(L"%%");
	std::wstring_view toFind(L"%");
	while ((pos = s1.find(toFind, pos)) != std::wstring::npos)
	{
		s1.replace(pos, 1, toPlace);
		pos += 2;
	}
	return s1;
}


#include <psapi.h>
#pragma comment(lib,"psapi.lib")
#include <iostream>
#include <fstream>


#include <iomanip>
void showMemoryInfo(const char* info, const char* funcName, int line,bool bForce)
{
	static int logCount = 0;
	static PROCESS_MEMORY_COUNTERS lastpmc;
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	/*FILE* file = nullptr;
	fopen_s(&file, R"(C:\Logs\frame2TCP\memory.log)", "a");

	fprintf_s(file, "memory useage: %I64dK/%I64dK %I64dK/%I64dK",
		pmc.WorkingSetSize/1000,
		pmc.PeakWorkingSetSize/1000,
		pmc.PagefileUsage / 1000 ,
		pmc.PeakPagefileUsage / 1000);
	fclose(file);*/
#define BBB 1024

	size_t offset = (pmc.PeakPagefileUsage - lastpmc.PeakPagefileUsage) / BBB / BBB;
	if (!bForce && offset<10)return;

	std::ofstream xFile;
	xFile.open(R"(C:\Logs\frame2TCP\memory.log)", std::ios_base::app);
	if (logCount++ == 0)
		xFile << "###############################" << std::endl;

	xFile << std::setw(4) << logCount <<",  "
		//<< " WorkingSetSize:" << std::setw(6) << pmc.WorkingSetSize / BBB / BBB << "/M" << " increase:" << std::setw(5) << (pmc.WorkingSetSize - lastpmc.WorkingSetSize) / BBB / BBB << " "
		//<< " PeakWorkingSetSize:" << std::setw(6) << pmc.PeakWorkingSetSize / BBB / BBB << "/M  " << " increase:" << std::setw(5) << (pmc.PeakWorkingSetSize - lastpmc.PeakWorkingSetSize) / BBB / BBB << " "
		//<< " PagefileUsage:" << std::setw(6) << pmc.PagefileUsage / BBB / BBB << "/M" << " increase:" << std::setw(5) << (pmc.PagefileUsage - lastpmc.PagefileUsage) / BBB / BBB << " "
		//<< " PeakPagefileUsage :" << std::setw(6) << pmc.PeakPagefileUsage / BBB / BBB << "/M" << " increase:" 
		<< std::setw(5) << (pmc.PeakPagefileUsage - lastpmc.PeakPagefileUsage) / BBB / BBB << " ,"
		<< " line:" << line << "@" << funcName
		<< "  ," << info
		<<"  ," << std::endl;
	xFile.close();


	lastpmc = pmc;
}