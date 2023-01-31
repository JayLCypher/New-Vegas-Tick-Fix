#include <chrono>
#include "FPSTimer.h"

namespace QPC {
	double FPSTimerFrequency = 0;
	double lastCount = 0;
	signed int GetTickCountBias = 0;
	DWORD ReturnCounter() {
		const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch()));
		return static_cast<unsigned long>(now.count() + GetTickCountBias);
	}
	double GetFPSCounterMilliseconds() {
		const auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()).time_since_epoch()));
		const double currentCount = static_cast<double>(now.count()) / 1000000;
		const double toReturn = currentCount - lastCount;
		lastCount = currentCount;
		return toReturn;
	}
}
namespace tGt {
	double lastCount = 0;
	signed int GetTickCountBias = 0;
	DWORD ReturnCounter() {
		return static_cast<unsigned long>(static_cast<long>(timeGetTime()) + GetTickCountBias);
	}
	double GetFPSCounterMilliseconds() {
		const double currentCount = timeGetTime();
		const double toReturn = currentCount - lastCount;
		lastCount = currentCount;
		return toReturn;
	}
}

void FPSStartCounter() {
	const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch()));
	const long duration = static_cast<long>(now.count());

	QPC::GetTickCountBias = duration - static_cast<long>(GetTickCount64()); //this would proooooooobably fail if your system was started 53 days ago, but at that point idc // GetTickCount() would overflow every 49 days.
	QPC::lastCount = duration;
	tGt::lastCount = timeGetTime(); //this one is for timeGetTime
	tGt::GetTickCountBias = static_cast<long>(tGt::lastCount) - static_cast<long>(GetTickCount64());
}

double GetFPSCounterMilliseconds_WRAP(const bool doUpdate) {
	if (doUpdate) {
		if (!g_bAlternateGTCFix) { return QPC::GetFPSCounterMilliseconds(); }
		return tGt::GetFPSCounterMilliseconds();
	}

	if (!g_bAlternateGTCFix) {
		const auto currentCount = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()).time_since_epoch())).count()) / 1000000;
		return currentCount - QPC::lastCount;
	}

	return (static_cast<double>(timeGetTime()) - tGt::lastCount);
}

DWORD ReturnCounter_WRAP() {
	if (!g_bAlternateGTCFix) { return QPC::ReturnCounter(); }
	return tGt::ReturnCounter();
}