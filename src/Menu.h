#pragma once

#include <dxgi.h>

namespace Menu
{
	bool Initialize(IDXGISwapChain* swapChain);
	void Shutdown();
	void Toggle();
	bool IsVisible();
	void Render();
}
