// framework_logger.
#include <fstream>
#include <thread>
#include <queue>
#include <condition_variable>
#include <filesystem>
#include <unordered_map>

#include "framework_logger.hpp"

using namespace std;
namespace PRLC = PSAG_LOGGER::ReadLogCache;

mutex                 LogMutex      = {};
queue<PRLC::LogCache> LogWriteQueue = {};

vector<PRLC::LogCache> GLOBAL_LOG_CACHE = {};
size_t LogWarringLines = NULL, LogErrorLines = NULL;

#include <iomanip>
// time [xxxx.xx.xx.xx:xx:xx:xx ms].
string __get_timestamp(const chrono::system_clock::time_point& time_point) {
	auto timemillisecond = chrono::duration_cast<chrono::milliseconds>(time_point.time_since_epoch()) % 1000;
	auto t = chrono::system_clock::to_time_t(time_point);
	tm _time = {};
#ifdef _WIN32
	localtime_s(&_time, &t);
#else
	localtime_r(&t, &_time);
#endif
	stringstream _sstream;
	_sstream << put_time(&_time, "[%Y.%m.%d %H:%M:%S") << " " << setfill('0') << setw(3) << timemillisecond.count() << " ms]";
	// windows / linux time stamp.
	return _sstream.str();
}

string FMT_TIME_STAMP(const chrono::system_clock::time_point& time_point) {
	return __get_timestamp(time_point);
}

// global const hashlable.
const std::unordered_map<int32_t, std::string> HashLogLable = {
	{LogError,   "\033[31m"},
	{LogWarning, "\033[33m"},
	{LogInfo,    "\033[90m"},
	{LogTrace,   "\033[96m"},
	{LogPerfmac, "\033[35m"}
};

string FMT_NUMBER_FILLZERO(uint32_t number, int32_t digits) {
	// format: %xd.
	string ResultTemp = to_string(number);
	// fill_number zero.
	while (ResultTemp.length() < digits) {
		ResultTemp = "0" + ResultTemp;
	}
	return ResultTemp;
}

namespace PSAG_LOGGER {
#include <io.h>
#include <fcntl.h>
	bool LOG_PRINT_SWITCH = true;

	void PushLogProcess(const LOGLABEL& label, const std::string& module_name, const std::string& logstr_text) {
		unique_lock<mutex> LogThreadLock(LogMutex);

		if (label & LogWarning) { LogWarringLines++; };
		if (label & LogError)   { LogErrorLines++; };

		const char* LogLabelTemp = "[NULL]";
		switch (label) {
		case(LogError):   { LogLabelTemp = "[ERROR]";   break; }
		case(LogWarning): { LogLabelTemp = "[WARNING]"; break; }
		case(LogInfo):    { LogLabelTemp = "[INFO]";    break; }
		case(LogTrace):   { LogLabelTemp = "[TRACE]";   break; }
		case(LogPerfmac): { LogLabelTemp = "[PERF]";    break; }
		}

		string FmtModuleName = "[" + module_name + "]: ";
		string FmtLog = __get_timestamp(chrono::system_clock::now()) + ":" + LogLabelTemp + ":" + FmtModuleName + logstr_text;

		// => read logger chache & logger process(print).
		GLOBAL_LOG_CACHE.push_back(PRLC::LogCache(FmtLog, module_name, label));
		LogWriteQueue.push(PRLC::LogCache(FmtLog, module_name, label));
	}

#define LOGGER_BUFFER_LEN 2048
	void PushLogger(const LOGLABEL& label, const char* module_label, const char* log_text, ...) {
#if PSAG_DEBUG_MODE
		char LoggerStrTemp[LOGGER_BUFFER_LEN] = {};

		va_list ParamArgs;
		va_start(ParamArgs, log_text);
		vsnprintf(LoggerStrTemp, LOGGER_BUFFER_LEN, log_text, ParamArgs);
		va_end(ParamArgs);

		PushLogProcess(label, (string)module_label, LoggerStrTemp);
#endif
	}

	void SET_PRINTLOG_STATE(bool status_flag) {
		unique_lock<mutex> LogThreadLock(LogMutex);
		LOG_PRINT_SWITCH = status_flag; 
	};

	Vector3T<size_t> LogLinesStatistics() {
		Vector3T<size_t> ReturnValue = {};
		{
			unique_lock<mutex> LogThreadLock(LogMutex);

			ReturnValue.vector_x = GLOBAL_LOG_CACHE.size(); // total   lines.
			ReturnValue.vector_y = LogWarringLines;         // warning lines.
			ReturnValue.vector_z = LogErrorLines;           // error   lines.
		}
		return ReturnValue;
	}

	namespace ReadLogCache {
		vector<LogCache> ExtractLogSegment(const uint32_t& lines) {
			vector<LogCache> ReturnLogCache = {};
			{
				size_t RetLinesCount =
					((int64_t)GLOBAL_LOG_CACHE.size() - lines - 1) >= 0 ?
					((int64_t)GLOBAL_LOG_CACHE.size() - lines - 1) : 0;

				unique_lock<mutex> LogThreadLock(LogMutex);
				for (size_t i = RetLinesCount; i < GLOBAL_LOG_CACHE.size(); ++i)
					ReturnLogCache.push_back(GLOBAL_LOG_CACHE[i]);
			}
			return ReturnLogCache;
		}
	}

	size_t GetTimeCountNow() {
		return (size_t)chrono::duration_cast
			<chrono::microseconds>(
				chrono::steady_clock::now().time_since_epoch()
			).count();
	}
}

#define LOGFILE_EXTENSION ".log"
namespace PSAG_LOGGER_PROCESS {

	thread* LogProcessThread = {};
	bool    LogProcessFlag   = true;

	void logprocess_write_file_eventloop(const char* folder) {
		// create name: folder + name(time) + ext.
		string LogFileName = 
			folder + 
			to_string(chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count()) +
			LOGFILE_EXTENSION;

		fstream WriteLogFile(LogFileName, ios::out | ios::app);
		PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "create log_file: %s", LogFileName.c_str());
		
		while (LogProcessFlag) {
			unique_lock<mutex> LogThreadLock(LogMutex);

			while (!LogWriteQueue.empty()) {
				const PRLC::LogCache& LogMsgTemp = LogWriteQueue.front();
				
				WriteLogFile << LogMsgTemp.LogString << endl;
				// print.
#if PSAG_DEBUG_MODE
				auto FindLevelColor = HashLogLable.find(LogMsgTemp.LogLabel);
				if (FindLevelColor != HashLogLable.end() && PSAG_LOGGER::LOG_PRINT_SWITCH)
					cout << FindLevelColor->second << LogMsgTemp.LogString << " \033[0m" << endl;
#endif
				// delete logmsg_item.
				LogWriteQueue.pop();
			}
		}
		WriteLogFile.close();
	}

	bool StartLogProcessing(const char* folder) {
		// check folder_path valid ?
		if (!filesystem::exists(folder) || !filesystem::is_directory(folder)) {
			PSAG_LOGGER::PushLogger(LogError, PSAG_LOGGER_LABEL, "invalid folder: %s", folder);
			return false;
		}
		// init logger_thread.
		try {
			LogProcessThread = new thread(logprocess_write_file_eventloop, folder);
			PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "start thread success.");
		}
		catch (const exception& err) {
			PSAG_LOGGER::PushLogger(LogError, PSAG_LOGGER_LABEL, "start thread error: %s", err.what());
			return false;
		}
		return true;
	}

	bool FreeLogProcessing() {
		LogProcessFlag = false;
		try {
			// set_flag  => join_thread.
			LogProcessThread->join();
			PSAG_LOGGER::PushLogger(LogInfo, PSAG_LOGGER_LABEL, "free thread success.");
		}
		catch (const exception& err) {
			PSAG_LOGGER::PushLogger(LogError, PSAG_LOGGER_LABEL, "free thread error: %s", err.what());
			return false;
		}
		return true;
	}
}

std::atomic<size_t> PSAG_SYS_GENERATE_KEY::UniqueCodeCountTemp = NULL;

size_t     PSAG_SYS_GENERATE_KEY::TimeCountBuffer      = {};
std::mutex PSAG_SYS_GENERATE_KEY::TimeCountBufferMutex = {};