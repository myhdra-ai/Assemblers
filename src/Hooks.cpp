#include "pch.h"
#include "Hooks.h"
#include "Menu.h"

#include <d3d11.h>
#include <dxgi.h>
#include <MinHook.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace
{
	using PresentFunction = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT);
	PresentFunction g_originalPresent = nullptr;
	bool g_initialized = false;

	HRESULT STDMETHODCALLTYPE PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
	{
		static bool wasInsertDown = false;
		const bool insertDown = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
		if (insertDown && !wasInsertDown)
			Menu::Toggle();
		wasInsertDown = insertDown;

		Menu::Initialize(swapChain);
		Menu::Render();
		return g_originalPresent(swapChain, syncInterval, flags);
	}

	bool FindPresentAddress(void** address)
	{
		DXGI_SWAP_CHAIN_DESC description{};
		description.BufferCount = 1;
		description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.OutputWindow = GetDesktopWindow();
		description.SampleDesc.Count = 1;
		description.Windowed = TRUE;

		D3D_FEATURE_LEVEL featureLevel{};
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;
		IDXGISwapChain* swapChain = nullptr;

		const HRESULT result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&description,
			&swapChain,
			&device,
			&featureLevel,
			&context);

		if (FAILED(result))
			return false;

		*address = (*reinterpret_cast<void***>(swapChain))[8];

		context->Release();
		device->Release();
		swapChain->Release();
		return *address != nullptr;
	}
}

namespace Hooks
{
	bool Initialize()
	{
		if (g_initialized)
			return true;

		void* presentAddress = nullptr;
		if (!FindPresentAddress(&presentAddress))
			return false;

		if (MH_Initialize() != MH_OK)
			return false;

		if (MH_CreateHook(presentAddress, &PresentHook,
			reinterpret_cast<void**>(&g_originalPresent)) != MH_OK)
		{
			MH_Uninitialize();
			return false;
		}

		if (MH_EnableHook(presentAddress) != MH_OK)
		{
			MH_RemoveHook(presentAddress);
			MH_Uninitialize();
			return false;
		}

		g_initialized = true;
		return true;
	}

	void Shutdown()
	{
		if (!g_initialized)
			return;

		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
		Menu::Shutdown();
		g_originalPresent = nullptr;
		g_initialized = false;
	}
}
