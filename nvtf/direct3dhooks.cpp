#pragma once
#include "direct3dhooks.h"
#include "nvse/SafeWrite.h"
#include <mutex>
HWND foreWindow = nullptr;
int g_bToggleTripleBuffering = 0;
int g_bUseFlipExSwapMode = 0;
int g_bD3D9ManageResources = 0;
int g_bUseDefaultPoolForTextures = 0;
int g_iNumBackBuffers  = 0;
int g_bForceD3D9Ex = 0;
int g_bUseDynamicBuffers = 0;

namespace D3DHooks {
	bool* g_DXEx = reinterpret_cast<bool *>(0x126F0D0); // (bool *)
	bool r_d3d9ex = false;
	IDirect3D9** g_D3Device = reinterpret_cast<IDirect3D9 **>(0x126F0D8); // (IDirect3D9 * *)0x126F0D8;

	bool IsD3D9ExAvailable() { return r_d3d9ex; }

	IDirect3D9* D3DAPI CreateD3D9(const bool UseD3D9Ex) {
		HMODULE d3d9 = GetModuleHandleA("d3d9.dll");
		constexpr UINT SDKVersion = 32;
		if (!d3d9) {
			d3d9 = LoadLibraryA("d3d9.dll");
			if (!d3d9) { return nullptr; }
		}
		const auto pDirect3DCreate9 = reinterpret_cast<decltype(&Direct3DCreate9)>(GetProcAddress(d3d9, "Direct3DCreate9")); // const auto pDirect3DCreate9 = (decltype(&Direct3DCreate9))GetProcAddress(d3d9, "Direct3DCreate9");
		const auto pDirect3DCreate9Ex = reinterpret_cast<decltype(&Direct3DCreate9Ex)>(GetProcAddress(d3d9, "Direct3DCreate9Ex")); // const auto pDirect3DCreate9Ex = (decltype(&Direct3DCreate9Ex))GetProcAddress(d3d9, "Direct3DCreate9Ex");

		// Check for Direct3DCreate9Ex first (user must manually enable dvar, autodetection disabled)
		if (pDirect3DCreate9Ex && UseD3D9Ex) {
			IDirect3D9Ex* d3dex = nullptr;
			if (SUCCEEDED(pDirect3DCreate9Ex(SDKVersion, &d3dex))) {
				PrintLog("Using Direct3D9Ex interface\n");
				r_d3d9ex = true;
				return d3dex;
			}
		}
		PrintLog("Using D3D9");
		// Otherwise default to the normal Direct3DCreate9
		if (pDirect3DCreate9) {
			if (IDirect3D9* d3d = pDirect3DCreate9(SDKVersion)) {
				r_d3d9ex = false;
				return d3d;
			}
		}
		r_d3d9ex = false;
		return nullptr;
	}
	DWORD __cdecl hk_Direct3DCreate9(bool UseEx) { // Param never used.
		*g_D3Device = CreateD3D9(g_bForceD3D9Ex);
		*g_DXEx = r_d3d9ex;
		if (!*g_D3Device) { return -1; }
		return 0;
	}
	D3DDISPLAYMODEEX currentDisplayMod;
	bool currentDisplayModExists = false;

	HRESULT D3DAPI hk_CreateDeviceEx(IDirect3D9Ex* This, const UINT Adapter, const D3DDEVTYPE DeviceType, const HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* displayMod, IDirect3DDevice9Ex** ppReturnedDeviceInterface)
	{
		HRESULT hr;
		if (displayMod) {
			currentDisplayMod = *displayMod;
			currentDisplayModExists = true;
		}
		if (g_bToggleTripleBuffering) pPresentationParameters->BackBufferCount = static_cast<UINT>(g_iNumBackBuffers); // (UINT)g_iNumBackBuffers;
		if (pPresentationParameters->Windowed) { foreWindow = hFocusWindow ? hFocusWindow : pPresentationParameters->hDeviceWindow; }
		if (!IsD3D9ExAvailable()) {
			if (g_bD3D9ManageResources) { BehaviorFlags |= D3DCREATE_DISABLE_DRIVER_MANAGEMENT; }
			PrintLog("using CreateDevice for device creation");
			hr = This->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, reinterpret_cast<IDirect3DDevice9 **>(ppReturnedDeviceInterface)); // (IDirect3DDevice9 * *)ppReturnedDeviceInterface
		}
		else {
			if (g_bUseFlipExSwapMode) {
				pPresentationParameters->SwapEffect = D3DSWAPEFFECT_FLIPEX;
				pPresentationParameters->Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
			}
			if (g_bD3D9ManageResources) BehaviorFlags |= D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX;
			hr = This->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, displayMod, (IDirect3DDevice9Ex * *)ppReturnedDeviceInterface);
			
			PrintLog("Using CreateDeviceEx for device creation\n");


		}

		return hr;
	}


	__declspec(naked) void AsmHandleDirectXExCreation()
	{
		static const uintptr_t retAddr = 0xE731CA;
		__asm {
			mov ecx, dword ptr ds : [esi + 0x5C4]
			push ecx
			push eax
			call hk_CreateDeviceEx
			jmp retAddr
		}
	}

	bool Game_GetIsMenuMode() {
		return ((bool(__cdecl*)())(0x0702360))(); // Bruh... What a C-style cast.
		return true; // Nice fallback bro
	}

	//replace our managed pool for DirectX9 DEFAULT pool, this is bound to cause a significant memory reduction
	HRESULT __stdcall CreateCubeTextureFromFileInMemoryHookForD3D9(const LPDIRECT3DDEVICE9 pDevice, const LPCVOID pSrcData, const UINT SrcDataSize, LPDIRECT3DCUBETEXTURE9* ppCubeTexture) {
		return D3DXCreateCubeTextureFromFileInMemoryEx(pDevice, pSrcData, SrcDataSize, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, nullptr, nullptr, ppCubeTexture);
	}
	HRESULT __stdcall CreateTextureFromFileInMemoryHookForD3D9(const LPDIRECT3DDEVICE9 pDevice, const LPCVOID pSrcData, const UINT SrcDataSize, LPDIRECT3DTEXTURE9* ppTexture) {
		return D3DXCreateTextureFromFileInMemoryEx(pDevice, pSrcData, SrcDataSize, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, nullptr, nullptr, ppTexture);
	}
	HRESULT __stdcall CreateVolumeTextureFromFileInMemoryHookForD3D9(const LPDIRECT3DDEVICE9 pDevice, const LPCVOID pSrcFile, const UINT SrcData, LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture) {
		return D3DXCreateVolumeTextureFromFileInMemoryEx(pDevice, pSrcFile, SrcData, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, nullptr, nullptr, ppVolumeTexture);
	}

	bool SetGammaRampInit = false;
	D3DGAMMARAMP StartingGammaRamp;
	void hk_SetGammaRamp(const LPDIRECT3DDEVICE9 pDevice, const UINT iSwapChain, const DWORD flags, D3DGAMMARAMP* pRamp) {
		if (!foreWindow) { pDevice->SetGammaRamp(iSwapChain, flags, pRamp); }
		else {
			const HDC device = ::GetDC(foreWindow);
			if (!SetGammaRampInit) { GetDeviceGammaRamp(device, &StartingGammaRamp); SetGammaRampInit = true; }
			SetDeviceGammaRamp(device, pRamp);
			::ReleaseDC(foreWindow, device);
		}
	}

	void UseD3D9xPatchMemory(const bool bUseDefaultPoolForTextures) {
		g_iNumBackBuffers = g_iNumBackBuffers > 4 ? 4 : (g_iNumBackBuffers < 1 ? 1 : g_iNumBackBuffers);
		//SafeWriteBuf(0xE69482, "\x90\x90\x90\x90\x90\x90\x90", 7);
		WriteRelJump(0xE731C1, reinterpret_cast<UInt32>(AsmHandleDirectXExCreation));
		SafeWriteBuf(0xE731C6, "\x90\x90\x90\x90\x90", 5);
		SafeWrite16(0x0E73191, 0x9090);
		WriteRelCall(0xE76215, reinterpret_cast<uintptr_t>(hk_Direct3DCreate9));
		if (bUseDefaultPoolForTextures) {
			SafeWrite32(0xFDF3FC, reinterpret_cast<UInt32>(CreateTextureFromFileInMemoryHookForD3D9));
			SafeWrite32(0xFDF400, reinterpret_cast<UInt32>(CreateCubeTextureFromFileInMemoryHookForD3D9));
			SafeWrite32(0xFDF404, reinterpret_cast<UInt32>(CreateVolumeTextureFromFileInMemoryHookForD3D9));
		}
	}
}