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
#include "Capture.h"

/**
 * Main application class. Outside of the `main.cpp` file, this holds the shared memory
 * and resources for the application.
 */
class App {
public:
	void Initialize(const UI::Composition::ContainerVisual&);

	void StartCapture(HWND);

	Graphics::DirectX::Direct3D11::IDirect3DDevice getD3DDevice() const { return device; }

protected:
	UI::Composition::Compositor compositor{ nullptr };
	UI::Composition::ContainerVisual root{ nullptr };
	UI::Composition::SpriteVisual content{ nullptr };
	UI::Composition::CompositionSurfaceBrush brush{ nullptr };

private:
	Graphics::DirectX::Direct3D11::IDirect3DDevice device{ nullptr };

	std::unique_ptr<Capture> capture{ nullptr };
};

