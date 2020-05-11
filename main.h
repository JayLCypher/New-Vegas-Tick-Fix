#pragma once

int g_bToggleTripleBuffering;
int g_bUseFlipExSwapMode;
int g_bD3D9ManageResources;
//int g_bDisablePerformanceFix;
int g_iNumBackBuffers;
int g_bForceD3D9Ex;

float* g_FPSGlobal = (float*)(0x11F6398);
UINT32 LastTime;
UINT32 OriginalFunction;
float* fMaxTime = (float*)0x1267B38;
bool initTimeHook = false;
float DefaultMaxTime = 0;
DWORD* InterfSingleton = (DWORD*)0x11D8A80;


int g_bGTCFix = 0;
int g_bFastExit = 0;
int g_bInlineStuff = 0;
int g_bFPSFix = 0;
int g_iMaxFPS = 1;
int g_iMinFPS = 1;
int g_bSpinCriticalSections = 1;
int g_bfMaxTime;
int g_bEnableExperimentalHooks = 0;
int g_bRemoveRCSafeGuard = 0;
int g_bRemove0x80SafeGuard = 0;
float g_iDialogFixMult = 1;
double	DesiredMax = 1;
double	DesiredMin = 1;
double	HavokMax = 1;
double	HavokMin = 1;
int g_bModifyDirectXBehavior = 1;




#include "hooks.h"
#include "FPSTimer.h"
#include "GameUI.h"
#include "mathVegas.h"
#include "direct3dhooks.h"
#include "CriticalSections.h"












int g_bUseDynamicResources;
//volatile double Delta;
 bool* g_DialogMenu = (bool*)0X11D9514;
 bool* g_IsMenuMode = (bool*)0x11DEA2B;
 bool* g_DialogMenu2 = (bool*)0x11DEA2B;



void TimeGlobalHook() {

	double Delta = GetFPSCounterMiliSeconds(); 
	if (*g_IsMenuMode)
	{
		
		if (*g_DialogMenu2 || *g_DialogMenu)
		{
			*g_FPSGlobal = (Delta > 0) ? ((Delta < DesiredMin) ? ((Delta > DesiredMax / g_iDialogFixMult) ? Delta : DesiredMax / g_iDialogFixMult) : DesiredMin) : 0;
			if (g_bfMaxTime)* fMaxTime = ((Delta > 0 && Delta < DefaultMaxTime && Delta > DesiredMax) ? Delta / 1000 : DefaultMaxTime / 1000);
		}
		else
		{
			*g_FPSGlobal = 0;
			*fMaxTime = DefaultMaxTime;
		}
	}
	else
	{
		*g_FPSGlobal = (Delta > 0) ? ((Delta < DesiredMin) ? ((Delta > DesiredMax) ? Delta : DesiredMax) : DesiredMin) : 0;
		if (g_bfMaxTime)* fMaxTime = ((Delta > 0 && Delta < DefaultMaxTime && Delta > DesiredMax) ? Delta / 1000 : DefaultMaxTime / 1000);
	}

}




void FastExit()
{
	TerminateProcess(GetCurrentProcess(), 0);
}



void __declspec(naked) FPSHookHandler()
{

	__asm {
		push ecx
		call TimeGlobalHook
		pop ecx
		jmp OriginalFunction
	}

}
void HookFPSStuff()
{
	SafeWriteBuf(0x86E65A, "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 0x11);
	WriteRelCall((UINT32)0x86E65A, (UInt32)& FPSHookHandler);
	OriginalFunction = 0x86E66C;
}


void DoPatches()
{

	if (g_bSpinCriticalSections) {
		_MESSAGE("CS ENABLED");
		WriteRelJump(0x0A62B08, (UInt32)NiObjectCriticalSections);
		WriteRelJump(0x0AA8D5B, (UInt32)MemHeapCSHook);
		DoHeapCriticalSectionSpin();
		if ((signed int)g_iSpinCount > -1) {
			SafeWrite32(0x0FDF054, (UInt32)InitCriticalSectionHook);
		}



	}
	if (g_bEnableExperimentalHooks) {
		if (g_bRemoveRCSafeGuard)	RemoveRefCountSafeGuard();
		if (g_bRemove0x80SafeGuard) Remove0x80SafeGuard();
	}
	if (g_bInlineStuff) {
		HookInlines(); //	MathHooks();
	}
	//Fast Exit Hook
	if (g_bFastExit) WriteRelJump(0x86B66E, (UInt32)FastExit);
	if (g_bGTCFix) {
		_MESSAGE("TGT ENABLED");
		//timeBeginPeriod(1);
		//SafeWrite32(0xFDF060, (UInt32)timeGetTime);
		FPSStartCounter();
		SafeWrite32(0xFDF060, (UInt32)ReturnCounter);
		if (g_bFPSFix)
		{
			_MESSAGE("FPSFIX ENABLED");

				DesiredMax = 1000 / double(g_iMaxFPS);
				DesiredMin = 1000 / double(g_iMinFPS);
				DefaultMaxTime = (*fMaxTime) * 1000;
			HookFPSStuff();
		}
	}
	if (g_bModifyDirectXBehavior)
		D3DHooks::UseD3D9xPatchMemory(g_bForceD3D9Ex, g_bUseDynamicResources);
}



