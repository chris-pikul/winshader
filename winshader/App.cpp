#include "App.h"

#include "Capture.h"
#include "d3dhelpers.h"

using namespace winrt;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::Graphics::Capture;

void App::Initialize(const UI::Composition::ContainerVisual& _root) {
	auto queue = DispatcherQueue::GetForCurrentThread();

    compositor = root.Compositor();
    root = compositor.CreateContainerVisual();
    content = compositor.CreateSpriteVisual();
    brush = compositor.CreateSurfaceBrush();

    root.RelativeSizeAdjustment({ 1, 1 });
    _root.Children().InsertAtTop(root);

    content.AnchorPoint({ 0.5f, 0.5f });
    content.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0 });
    content.RelativeSizeAdjustment({ 1, 1 });
    content.Size({ -80, -80 });
    content.Brush(brush);
    brush.HorizontalAlignmentRatio(0.5f);
    brush.VerticalAlignmentRatio(0.5f);
    brush.Stretch(CompositionStretch::Uniform);
    auto shadow = compositor.CreateDropShadow();
    shadow.Mask(brush);
    content.Shadow(shadow);
    root.Children().InsertAtTop(content);

    auto d3dDevice = CreateD3DDevice();
    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    device = CreateDirect3DDevice(dxgiDevice.get());
}

void App::StartCapture(HWND hWnd) {
    // Reload the capture if we already had one
    if (capture) {
        capture->Close();
        capture = nullptr;
    }

    auto item = CreateCaptureItemForWindow(hWnd);
    capture = std::make_unique<Capture>(device, item);
    auto surface = capture->CreateSurface(compositor);
    brush.Surface(surface);

    // Start the capture process
    capture->Start();
}