#include "pch.h"
#include "Menu.h"

#include <d2d1_3.h>
#include <dwrite.h>
#include <wrl/client.h>

#pragma comment(lib, "d2d1.lib")

using Microsoft::WRL::ComPtr;

namespace
{
	ComPtr<ID2D1Factory1> g_factory;
	ComPtr<ID2D1Device> g_device;
	ComPtr<ID2D1DeviceContext> g_context;
	ComPtr<ID2D1Bitmap1> g_target;
	ComPtr<ID2D1SolidColorBrush> g_backgroundBrush;
	ComPtr<ID2D1SolidColorBrush> g_panelBrush;
	ComPtr<ID2D1SolidColorBrush> g_textBrush;
	bool g_visible = false;
}

namespace Menu
{
	bool Initialize(IDXGISwapChain* swapChain)
	{
		if (g_context)
			return true;

		if (!swapChain)
			return false;

		ComPtr<IDXGIDevice> dxgiDevice;
		if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(&dxgiDevice))))
			return false;

		D2D1_FACTORY_OPTIONS options{};
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory1), &options,
			reinterpret_cast<void**>(g_factory.GetAddressOf()))))
			return false;

		if (FAILED(g_factory->CreateDevice(dxgiDevice.Get(), &g_device)))
			return false;

		if (FAILED(g_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &g_context)))
			return false;

		ComPtr<IDXGISurface> surface;
		if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&surface))))
			return false;

		D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

		if (FAILED(g_context->CreateBitmapFromDxgiSurface(surface.Get(), &bitmapProperties, &g_target)))
			return false;

		g_context->SetTarget(g_target.Get());

		if (FAILED(g_context->CreateSolidColorBrush(D2D1::ColorF(0.02f, 0.02f, 0.02f, 0.55f), &g_backgroundBrush)))
			return false;
		if (FAILED(g_context->CreateSolidColorBrush(D2D1::ColorF(0.10f, 0.10f, 0.14f, 0.96f), &g_panelBrush)))
			return false;
		if (FAILED(g_context->CreateSolidColorBrush(D2D1::ColorF(0.25f, 0.75f, 1.0f, 1.0f), &g_textBrush)))
			return false;

		return true;
	}

	void Shutdown()
	{
		g_target.Reset();
		g_textBrush.Reset();
		g_panelBrush.Reset();
		g_backgroundBrush.Reset();
		g_context.Reset();
		g_device.Reset();
		g_factory.Reset();
		g_visible = false;
	}

	void Toggle()
	{
		g_visible = !g_visible;
	}

	bool IsVisible()
	{
		return g_visible;
	}

	void Render()
	{
		if (!g_visible || !g_context || !g_target)
			return;

		const D2D1_SIZE_F size = g_target->GetSize();
		const D2D1_RECT_F panel = D2D1::RectF(32.0f, 32.0f, 332.0f, 190.0f);

		g_context->BeginDraw();
		g_context->FillRectangle(D2D1::RectF(0.0f, 0.0f, size.width, size.height), g_backgroundBrush.Get());
		g_context->FillRectangle(panel, g_panelBrush.Get());
		g_context->DrawRectangle(panel, g_textBrush.Get(), 2.0f);
		g_context->DrawRectangle(D2D1::RectF(56.0f, 86.0f, 76.0f, 106.0f), g_textBrush.Get(), 2.0f);
		g_context->DrawLine(D2D1::Point2F(56.0f, 140.0f), D2D1::Point2F(300.0f, 140.0f), g_textBrush.Get(), 2.0f);
		g_context->EndDraw();
	}
}
