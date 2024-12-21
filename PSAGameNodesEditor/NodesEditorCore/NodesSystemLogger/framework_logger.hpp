// framework_logger. RCSZ 2023_10_11.
// @pomelo_star studio framework universal comp.

#ifndef _FRAMEWORK_LOG_HPP
#define _FRAMEWORK_LOG_HPP
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <mutex>
#include <cstdarg>

#ifndef IS_POMELO_STAR_GAME2D
#define PSAG_DEBUG_MODE true
#define StaticStrLABEL constexpr const char*
// core framework logger label.
enum LOGLABEL {
	LogError   = 1 << 1, // 标签 <错误>
	LogWarning = 1 << 2, // 标签 <警告>
	LogInfo    = 1 << 3, // 标签 <信息>
	LogTrace   = 1 << 4, // 标签 <跟踪>
	LogPerfmac = 1 << 5  // 标签 <性能> (performance)
};

// vector 3d x, y, z.
template <typename mvec>
struct Vector3T {
	mvec vector_x, vector_y, vector_z;
	constexpr Vector3T() : vector_x{}, vector_y{}, vector_z{} {}
	constexpr Vector3T(mvec x, mvec y, mvec z) : vector_x(x), vector_y(y), vector_z(z) {}

	mvec* data() { return &vector_x; }
	const mvec* data() const { return &vector_x; }
};
#else
#include "../LLDefine/framework_def_header.h"
#include "../LLDefine/framework_def_math.hpp"
#endif
StaticStrLABEL PSAG_LOGGER_LABEL = "PSAG_LOGGER";

// format number => string, %d(fill_zero).
std::string FMT_NUMBER_FILLZERO(uint32_t number, int32_t digits);
// format time_point: "[xxxx.xx.xx.xx:xx:xx:xx ms]".
std::string FMT_TIME_STAMP(const std::chrono::system_clock::time_point& time_point);

namespace PSAG_LOGGER {
	// @param "label" (level label), "module_name" (module), "logstr_text" (log information)
	void PushLogProcess(const LOGLABEL& label, const std::string&  module_name, const std::string& logstr_text);
	// @param label, module_label, text, params. [20231205]
	void PushLogger(const LOGLABEL& label, const char* module_label, const char* log_text, ...);

	// false: not printing on the console.
	void SET_PRINTLOG_STATE(bool status_flag);
	
	// @return Vector3T<size_t> (x : lines, y : warring, z : error)
	Vector3T<size_t> LogLinesStatistics();

	namespace ReadLogCache {
		struct LogCache {
			// lv.0: log_label, lv.1: log_model_name, lv.2: log_string.
			std::string LogString;
			std::string LogModuleName;
			LOGLABEL    LogLabel;
			LogCache(const std::string& str, const std::string& name, LOGLABEL lab) :
				LogString(str), LogModuleName(name), LogLabel(lab)
			{}
		};
		// @param  uint32_t, back - lines.
		// @return string
		std::vector<LogCache> ExtractLogSegment(const uint32_t& lines);
	}
	// get src time[nanoseconds].
	size_t GetTimeCountNow();
}

namespace PSAG_LOGGER_PROCESS {
	// async thread process. write folder & print.
	bool StartLogProcessing(const char* folder);
	bool FreeLogProcessing();
}

// framework global generate_key.
class PSAG_SYS_GENERATE_KEY {
private:
	static std::atomic<size_t> UniqueCodeCountTemp;

	static size_t     TimeCountBuffer;
	static std::mutex TimeCountBufferMutex;
public:
	// generate unique number(time:nanoseconds).
	// time_key => count_key. RCSZ [20240825]
	size_t PsagGenUniqueKey() {
		// count: const_add_value: 2.
		return UniqueCodeCountTemp += 2;
	}

	// generate unique number(time:nanoseconds). [OLD]
	size_t PsagGenTimeKey() {
		std::lock_guard<std::mutex> Lock(TimeCountBufferMutex);

		size_t GenNumberTemp = (size_t)std::chrono::duration_cast
			<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

		if (GenNumberTemp == TimeCountBuffer) {
			// prevent duplication.
			std::chrono::nanoseconds ThreadSleep(2);
			std::this_thread::sleep_for(ThreadSleep);
		}
		TimeCountBuffer = GenNumberTemp;
		return GenNumberTemp;
	}
};

#endif