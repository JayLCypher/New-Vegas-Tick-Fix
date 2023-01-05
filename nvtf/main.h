#pragma once
#ifndef MAIN_H
#define MAIN_H

#include <thread>
#include "SafeWrite.h"
#include "hooks.h"
#include "FPSTimer.h"
#include "direct3dhooks.h"
#include "CriticalSections.h"
#include "Utilities.h"

inline float* g_FPSGlobal = reinterpret_cast<float *>(0x11F6398); // float* g_FPSGlobal = (float*)(0x11F6398);
inline UINT32 LastTime;
inline UINT32 OriginalFunction;
inline float* fMaxTime = reinterpret_cast<float*>(0x1267B38); // float* fMaxTime = (float*)0x1267B38;
inline bool initTimeHook = false;
inline double fLowerMaxTimeBoundary = 0.0;
inline DWORD *InterfaceSingleton = reinterpret_cast<DWORD *>(0x11D8A80); // DWORD* InterfSingleton = (DWORD*)0x11D8A80;
inline int g_bGTCFix = 0;
inline int g_bAllowDirectXDebugging = 0;
inline int g_bAllowBrightnessChangeWindowed = 0;
inline int g_bFastExit = 0;
inline int g_bFPSFix = 0;
inline int g_iMaxFPS = 1;
inline int g_iMinFPS = 1;
inline int g_bfMaxTime;
inline int g_bEnableThreadingTweaks = 0;
inline int g_bRemoveRCSafeGuard = 0;
inline int g_bTweakMiscCriticalSections = 0;
inline int g_bReplaceDeadlockCSWithWaitAndSleep = 0;
inline int g_bSpiderHandsFix = 0;
inline double DesiredMax = 1;
inline double DesiredMin = 1000;
inline double HavokMax = 1;
inline double HavokMin = 1;
inline int g_bModifyDirectXBehavior = 1;
inline int g_bRedoHashtables = 0;
inline int g_bResizeHashtables = 1;
inline int g_bAlternateGTCFix = 0;
inline int g_bRemoveGTCLimits = 0;
inline int g_bAutomaticFPSFix = 0;
inline int g_bUseExperimentalCacheForBuffers = 0;
inline int g_bWaterLODPatch = 0;

inline std::map<uintptr_t, int> MapLogger;

enum StartMenuFlags
{
	kHasChangedSettings = 0x2,
	kLoad = 0x4,
	kIsSaveMenuNotLoad = 0x8,
	kIsMainMenuShown = 0x10,
	kAreOptionsInitialised = 0x20,
	kShowDLCPopup = 0x400,
	kIsActionMapping = 0x1000,
	kShowCredits = 0x10000,
	kControllerDisconnected = 0x20000,
	kSomethingSave = 0x40000000,
	kShowMustRestartToSaveSettings = 0x400000,
	kSomething_credits = 0x2000000,
	kDeleteSave = 0x2000,
	kControllerInputDebounce = 0x4000000,
};

static uintptr_t *GetStartMenuSingleton() { return *reinterpret_cast<uintptr_t **>(0x11DAAC0); } // static uintptr_t* GetStartMenuSingleton() { return *(uintptr_t**)0x11DAAC0; };
inline void ResetBrightness() {
	if (foreWindow && D3DHooks::SetGammaRampInit) {
		HDC Devic = ::GetDC(foreWindow);
		SetDeviceGammaRamp(Devic, &D3DHooks::StartingGammaRamp);
		::ReleaseDC(foreWindow, Devic);
	}
}

inline void FastExit() {
	ResetBrightness();
	/*for (auto it = MapLogger.begin(); it != MapLogger.end(); ++it) { _MESSAGE("Address 0x%X called %i times", it->first, it->second); }*/
	if (const auto start = reinterpret_cast<UInt32>(GetStartMenuSingleton())) {
		if (*reinterpret_cast<UInt32 *>(start + 0x1A8) & kHasChangedSettings) { reinterpret_cast<void(*)()>(0x7D6D70)(); } // SaveIniSettings // Fix that ugly cast later.
	}
	TerminateProcess(GetCurrentProcess(), 0);
}

inline int __fastcall hk_OSGlobalsExit(void* thisObj) {
	ResetBrightness();
	return ThisStdCall<int>(0x5B6CB0, thisObj);
}

inline UInt32 RetrieveAddrFromDisp32Opcode(const UInt32 address) { return *reinterpret_cast<UInt32 *>(address + 1 + address + 5); } // return *(UInt32*)(address + 1) + address + 5;

inline UInt32 Calculaterel32(const UInt32 destination, const UInt32 source) {
	return destination - source - 5;
}

//Volatile double Delta;
inline bool* g_DialogMenu = reinterpret_cast<bool *>(0X11D9514); // bool* g_DialogMenu = (bool*)0X11D9514;
inline bool* g_IsMenuMode = reinterpret_cast<bool *>(0x11DEA2B); // inline bool* g_IsMenuMode = (bool*)0x11DEA2B;
inline bool* g_DialogMenu2 = reinterpret_cast<bool *>(0x11DEA2B); // inline bool* g_DialogMenu2 = (bool*)0x11DEA2B;

// uintptr_t OriginalDisplayCall;
inline std::chrono::steady_clock::time_point FPSTargetClock;
inline uintptr_t __fastcall FPSLimit() {
	//bool resResult = ThisStdCall_B(OriginalDisplayCall, thisObj);
	//int FPSTarget = 50;
	//int CurrentFPS = 1000 / GetFPSCounterMiliSeconds_WRAP(false);
	constexpr double FPSTarget = 50.0;
	if (const auto CurrentClock = std::chrono::steady_clock::now(); FPSTargetClock > CurrentClock) {
		std::this_thread::sleep_for(FPSTargetClock - CurrentClock); //std::this_thread::sleep_until(FPSTargetClock);
	}
	FPSTargetClock = std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<unsigned int>(1000 / FPSTarget));
	//auto targetSleep = ((std::chrono::microseconds(std::chrono::seconds(1)) / FPSTarget) - std::chrono::microseconds(std::chrono::seconds(long long((GetFPSCounterMiliSeconds_WRAP(false) + 1) / 1000))));
	//auto target_tp = std::chrono::steady_clock::now() + targetSleep;
	//std::this_thread::sleep_until(target_tp);
	return 0;
}


constexpr float fTimerOffsetMult = 0.9825f; // I have no idea why this works but it does.
inline void __stdcall TimeGlobalHook(void* unused) {
	//FPSLimit(nullptr);
	const double delta = GetFPSCounterMilliseconds_WRAP();
	//FPSLimitClock = std::chrono::steady_clock::now();
	if (g_bfMaxTime) { *fMaxTime = (delta > FLT_EPSILON) ? (delta < DesiredMin ? (delta > DesiredMax ? static_cast<float>(delta / 1000.0) : static_cast<float>(DesiredMax / 1000.0)) : static_cast<float>(DesiredMin / 1000.0)) : static_cast<float>(fLowerMaxTimeBoundary / 1000) ;}

	if (!*g_IsMenuMode || !(!*g_DialogMenu2 && !*g_DialogMenu)) { *g_FPSGlobal = delta > 0.0 ? (delta < DesiredMin ? (delta > DesiredMax ? static_cast<float>(delta) : static_cast<float>(DesiredMax)) : static_cast<float>(DesiredMin)) : 0.0f; }
	else { *g_FPSGlobal = 0; }

	if (g_bSpiderHandsFix > 0 && *g_FPSGlobal > FLT_EPSILON) {
		*g_FPSGlobal = 1000 / (1000 / *g_FPSGlobal * fTimerOffsetMult);
		if (g_bfMaxTime) {
			*fMaxTime = 1000 / ((1000 / *fMaxTime) * fTimerOffsetMult);
			if (*fMaxTime < FLT_EPSILON) { *fMaxTime = FLT_EPSILON; }
			//	if (*fMaxTime > ((DesiredMin / 1000))) *fMaxTime = ((DesiredMin / 1000));
		}
	}
	if (g_bfMaxTime && *fMaxTime > static_cast<float>(fLowerMaxTimeBoundary / 1000.0)) *fMaxTime = static_cast<float>(fLowerMaxTimeBoundary / 1000.0); //clamp to fix, will do properly later
}

inline void __stdcall TimeGlobalHook_NoSafeGuards(void* unused) {
	const double Delta = GetFPSCounterMilliseconds_WRAP();
	if (g_bfMaxTime) { *fMaxTime = Delta > 0 && Delta < fLowerMaxTimeBoundary ? (Delta > DesiredMax ? static_cast<float>(Delta / 1000.0) : static_cast<float>(DesiredMax / 1000.0)) : static_cast<float>(fLowerMaxTimeBoundary / 1000.0); }

	*g_FPSGlobal = Delta > 0.0 ? (Delta < DesiredMin ? (Delta > DesiredMax ? static_cast<float>(Delta) : static_cast<float>(DesiredMax)) : static_cast<float>(DesiredMin)) : 0.0f;

	if (g_bSpiderHandsFix > 0 && *g_FPSGlobal > FLT_EPSILON) {
		*g_FPSGlobal /= fTimerOffsetMult; // Eat it Karut. // *g_FPSGlobal = 1000 / (1000 / *g_FPSGlobal * fTimerOffsetMult);
		//if (g_bfMaxTime) *fMaxTime = 1000 / ((1000 / *fMaxTime) * fTimerOffsetMult);
		*g_FPSGlobal = 1000 / ((1000 / *g_FPSGlobal) * fTimerOffsetMult);
		if (g_bfMaxTime) {
			*fMaxTime = 1000 / ((1000 / *fMaxTime) * fTimerOffsetMult);
			if (*fMaxTime < FLT_EPSILON) { *fMaxTime = FLT_EPSILON; }
		}
	}
	if (g_bfMaxTime && *fMaxTime > static_cast<float>(fLowerMaxTimeBoundary / 1000.0)) *fMaxTime = static_cast<float>(fLowerMaxTimeBoundary / 1000.0); //clamp to fix, will do properly later
}

//test function, can be left out
/*
DWORD hk_GetTickCount()
{

	if (InterfaceManager::GetSingleton()->currentMode != 2)
	{
		auto retAddr = (uintptr_t)_ReturnAddress();
		auto it = MapLogger.find(retAddr);
		if (it != MapLogger.end())
		{
			it->second += 1;
		}
		else
		{
			MapLogger.insert(std::pair<uintptr_t, int>(retAddr, 1));
		}
	}
	return GetTickCount();
}
*/

inline __declspec (naked) void asm_FPSTrailHook()
{
	__asm {
		pop esi
		mov esp, ebp
		pop ebp
		jmp FPSLimit
	}
}

static uintptr_t FPSFix_TimeHookCall = NULL;
inline void HookFPSStuff() {

	//WriteRelJump(0x086EF2B, (uintptr_t)asm_FPSTrailHook);
	//OriginalDisplayCall = (*(uintptr_t*)0x10EE640);
	//SafeWrite32(0x10EE640, (uintptr_t)FPSLimt);
	//SafeWriteBuf(0x86E65A, "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 0x11);
	if (!g_bRemoveGTCLimits) { FPSFix_TimeHookCall = reinterpret_cast<uintptr_t>(TimeGlobalHook); }
	else { FPSFix_TimeHookCall = reinterpret_cast<uintptr_t>(TimeGlobalHook_NoSafeGuards); }
	if (FPSFix_TimeHookCall) WriteRelCall(static_cast<uintptr_t>(0x086E667), FPSFix_TimeHookCall);
}

inline void __stdcall SleepHook(DWORD dwMilliseconds) {
	if (dwMilliseconds <= 0) { SwitchToThread(); }
	Sleep(dwMilliseconds);
}

inline void DoPatches() {
	if (g_bAllowDirectXDebugging) { SafeWriteBuf(0x09F9968, "\xC2\x04\x00\xCC\xCC\xCC", 6); } //SafeWriteBuf(0x4DAD61, "\x90\x90\x90\x90\x90\x90\x90", 7);
	if (g_bEnableThreadingTweaks) {
		if (g_bRemoveRCSafeGuard)	RemoveRefCountSafeGuard();
		//if (g_bRemoveRendererLockSafeGuard) RemoveRendererLockSafeGuard();
		if (g_bTweakMiscCriticalSections) TweakMiscCriticalSections();
		//	SafeWriteBuf(0x8728D7, "\x8B\xE5\x5D\xC3\x90\x90", 6);
		if (g_bReplaceDeadlockCSWithWaitAndSleep) TurnProblematicCSIntoBusyLocks();
	}

	//Fast Exit Hook
	if (g_bFastExit) { WriteRelJump(0x86B66E, reinterpret_cast<UInt32>(FastExit)); } // WriteRelJump(0x86B66E, (UInt32)FastExit);
	if (g_bRedoHashtables) { DoHashTableStuff(); }

	if (g_bAllowBrightnessChangeWindowed)
	{
		WriteRelCall(0x4DD119, reinterpret_cast<UInt32>(D3DHooks::hk_SetGammaRamp));
		for (UInt32 patchAddr : {0x5B6CA6, 0x7D0C3E, 0x86A38B}) { WriteRelCall(patchAddr, reinterpret_cast<UInt32>(hk_OSGlobalsExit)); }
	}
	if (g_bGTCFix) {
		PrintLog("TGT ENABLED");
		//timeBeginPeriod(1);
		//SafeWrite32(0xFDF060, (UInt32)timeGetTime);
		FPSStartCounter();
		const auto TargetGTC = reinterpret_cast<UInt32>(ReturnCounter_WRAP);
		if (g_bAlternateGTCFix) { timeBeginPeriod(1); }
		SafeWrite32(0xFDF060, TargetGTC);
		//SafeWrite32(0xFDF060, (UInt32)hk_GetTickCount);
		if (g_bFPSFix) {
			PrintLog("FPSFIX ENABLED");
			DesiredMax = 1000.0 / static_cast<double>(g_iMaxFPS);
			DesiredMin = 1000.0 / static_cast<double>(g_iMinFPS);
			fLowerMaxTimeBoundary = static_cast<double>(*fMaxTime * 1000.0f);
			HookFPSStuff();
		}
	}
	if (g_bModifyDirectXBehavior) { D3DHooks::UseD3D9xPatchMemory(g_bUseDefaultPoolForTextures); }
	if (g_bWaterLODPatch) { WriteRelCall(static_cast<UInt32>(0x6FD0D4), reinterpret_cast<UInt32>(FakeFrustumHook)); } // WriteRelCall((uintptr_t)0x6FD0D4, (uintptr_t)FakeFrustumHook);
	//for (uintptr_t hookPtr : {0x045B1D4, 0x045B37F, 0x045B41B, 0x0045B4ED, 0x0045B69B, 0x045B7E0, 0x06FA772, 0x06FA7B5, 0x06FA7F8})
/*	for (uintptr_t hookPtr : {0x06FA772, 0x06FA7B5, 0x06FA7F8}) {
		//WriteRelCall((uintptr_t)hookPtr, (uintptr_t)hook_WaterCullCheck);
	}*/
}

#endif