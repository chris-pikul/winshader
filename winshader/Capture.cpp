#include "Capture.h"

#include "d3dhelpers.h"

using namespace winrt;
using namespace Windows;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

Capture::Capture(const IDirect3DDevice& _device, const GraphicsCaptureItem& _item) {
	device = _device;
	item = _item;

	// Setup D3D context
	auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(device);
	d3dDevice->GetImmediateContext(d3dContext.put());

	lastSize = item.Size();

	// Make swap chain for DirectX
	swapChain = CreateDXGISwapChain(
		d3dDevice,
		static_cast<uint32_t>(lastSize.Width),
		static_cast<uint32_t>(lastSize.Height),
		static_cast<DXGI_FORMAT>(DirectXPixelFormat::B8G8R8A8UIntNormalized),
		2
	);

	// Create frame pool, declaring the format/size of the frames
	framePool = Direct3D11CaptureFramePool::Create(
		device,
		DirectXPixelFormat::B8G8R8A8UIntNormalized,
		2,
		lastSize
	);
	session = framePool.CreateCaptureSession(item);
	frameArrived = framePool.FrameArrived(auto_revoke, { this, &Capture::OnFrameArrived });
}

void Capture::Close() {
	bool expected = false;
	if (closed.compare_exchange_strong(expected, true)) {
		frameArrived.revoke();
		framePool.Close();
		session.Close();

		swapChain = nullptr;
		framePool = nullptr;
		session = nullptr;
		item = nullptr;
	}
}

void Capture::Start() {
	CheckClosed();

	session.StartCapture();
}

ICompositionSurface Capture::CreateSurface(const UI::Composition::Compositor& _compositor) {
	CheckClosed();

	return CreateCompositionSurfaceForSwapChain(_compositor, swapChain.get());
}

void Capture::OnFrameArrived(const Graphics::Capture::Direct3D11CaptureFramePool& sender, const Foundation::IInspectable& args) {
	bool newSize = false;

	{
		auto frame = sender.TryGetNextFrame();
		auto frameContentSize = frame.ContentSize();

		// Check if the frame size changed
		if (frameContentSize.Width != lastSize.Width || frameContentSize.Height != lastSize.Height) {
			// The thing we have been capturing has changed size.
			// We need to resize our swap chain first, then blit the pixels.
			// After we do that, retire the frame and then recreate our frame pool.
			newSize = true;
			lastSize = frameContentSize;
			swapChain->ResizeBuffers(
				2,
				static_cast<uint32_t>(lastSize.Width),
				static_cast<uint32_t>(lastSize.Height),
				static_cast<DXGI_FORMAT>(DirectXPixelFormat::B8G8R8A8UIntNormalized),
				0
			);
		}

		// Swap the back/front buffers and copy the data, prepping for new frame.
		{
			auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

			com_ptr<ID3D11Texture2D> backBuffer;
			check_hresult(swapChain->GetBuffer(0, guid_of<ID3D11Texture2D>(), backBuffer.put_void()));

			d3dContext->CopyResource(backBuffer.get(), frameSurface.get());
		}
	}

	// Present the frame to the device
	DXGI_PRESENT_PARAMETERS presentParameters = { 0 };
	swapChain->Present1(1, 0, &presentParameters);

	// If the frame size changed, we need to recreate the frame pool
	if (newSize) {
		framePool.Recreate(
			device,
			DirectXPixelFormat::B8G8R8A8UIntNormalized,
			2,
			lastSize
		);
	}
}