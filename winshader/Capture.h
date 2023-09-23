/**
 * WinShader - Shader based windows fun.
 *
 * Copyright 2023 Chris Pikul. All Rights Reserved.
 * This code is licensed under the MIT License (MIT).
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "pch.h"

/**
 * Provides a class to handle the screen capture process
 */
class Capture {
public:
	Capture(const Graphics::DirectX::Direct3D11::IDirect3DDevice&, const Graphics::Capture::GraphicsCaptureItem&);
	~Capture() { Close(); }

	void Close();
	void Start();

	UI::Composition::ICompositionSurface CreateSurface(const UI::Composition::Compositor&);

protected:
	inline void CheckClosed() const { if (closed.load()) throw winrt::hresult_error(RO_E_CLOSED); }

	void OnFrameArrived(const Graphics::Capture::Direct3D11CaptureFramePool&, const Foundation::IInspectable&);

private:
	std::atomic_bool closed = false;

	Graphics::Capture::GraphicsCaptureItem item{ nullptr };
	Graphics::Capture::Direct3D11CaptureFramePool framePool{ nullptr };
	Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker frameArrived;
	Graphics::Capture::GraphicsCaptureSession session{ nullptr };

	Graphics::SizeInt32 lastSize;
	Graphics::DirectX::Direct3D11::IDirect3DDevice device{ nullptr };
	winrt::com_ptr<IDXGISwapChain1> swapChain{ nullptr };
	winrt::com_ptr<ID3D11DeviceContext> d3dContext{ nullptr };
};

