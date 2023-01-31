#pragma once
#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

extern ULONG g_iSpinCount;
extern void __stdcall SafeWrite8(UInt32 address, UInt32 data);
extern void PrintLog(const char* fmt, ...);

struct MemoryHeap {
	void* vtable; // AbstractHeap
	UInt32 unk04[0x835];
	CRITICAL_SECTION criticalSection;
};

struct Heaps {
	MemoryHeap* defaultHeap;
	MemoryHeap* staticHeap;
	MemoryHeap* fileHeap;
};

struct HeapManager {
	UInt16 pad00;
	UInt16 unk02;
	UInt32 unk04; // 04
	Heaps* memHeaps; // 08
	static HeapManager* GetSingleton() { return reinterpret_cast<HeapManager *>(0x11F6238); }
};

struct BGSLightCriticalSection {
	uintptr_t OwningThreadId = 0;
	uintptr_t LockCount = 0;
};

void __stdcall InitCriticalSectionHook(LPCRITICAL_SECTION section);
void __stdcall func_InitCSHandler(LPCRITICAL_SECTION cs, DWORD spin);
void __stdcall NiObjectCSHandler(LPCRITICAL_SECTION cs1, LPCRITICAL_SECTION cs2, LPCRITICAL_SECTION cs3);
DWORD NiObjectCriticalSections(DWORD* ecx);
DWORD someOddCSCall(DWORD* ecx);
DWORD someOddCSCall_2(DWORD* ecx);
void MemHeapCSHook(DWORD* memHeap);
void ReturnCSHook();

void DoHeapCriticalSectionSpin();
void TweakRefCountSafeGuard(int mode);
void TweakRendererLockSafeGuard();
void TweakMiscCriticalSections();
void TurnProblematicCSIntoBusyLocks();

#endif