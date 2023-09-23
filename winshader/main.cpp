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
#include "pch.h"

#include "App.h"

using namespace winrt;
using namespace UI;
using namespace UI::Composition;
using namespace UI::Composition::Desktop;

// Establish global instance of App for the life of the program
std::shared_ptr<App> g_App = std::make_shared<App>();

// Forward declare the windows main function
int CALLBACK WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int);

// Forward declare the windows procedure handler function
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// DX3D11 Requires a dispatcher queue
System::DispatcherQueueController CreateDispatcherQueueController() {
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options {
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_STA
    };

    System::DispatcherQueueController controller{ nullptr };
    check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
    return controller;
}

DesktopWindowTarget CreateDesktopWindowTarget(Compositor const& compositor, HWND hWnd) {
    namespace abi = ABI::Windows::UI::Composition::Desktop;

    auto interop = compositor.as<abi::ICompositorDesktopInterop>();
    DesktopWindowTarget target{ nullptr };
    check_hresult(interop->CreateDesktopWindowTarget(hWnd, true, reinterpret_cast<abi::IDesktopWindowTarget**>(put_abi(target))));
    return target;
}

// Implementation of windows main function. Program entry.
int CALLBACK WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPSTR cmdLine, _In_ int cmdShow) {
    // Initialize WinRT threading model for COM
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    // Setup window class for program display
    WNDCLASSEX wndClass = {};
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = instance;
    wndClass.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_APPLICATION));
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = L"WinShader";
    wndClass.hIconSm = LoadIcon(wndClass.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    WINRT_VERIFY(RegisterClassEx(&wndClass));

    // Build window instance itself
    HWND hWnd = CreateWindow(
        L"WinShader",
        L"WinShader",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        NULL,
        NULL,
        instance,
        NULL
    );
    WINRT_VERIFY(hWnd);

    ShowWindow(hWnd, cmdShow);
    UpdateWindow(hWnd);

    // Create dispatcher queue for DX3D using THIS thread
    auto controller = CreateDispatcherQueueController();

    // Initialize the compositor for capturing
    auto compositor = Compositor();
    auto target = CreateDesktopWindowTarget(compositor, hWnd);
    auto root = compositor.CreateContainerVisual();
    root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    target.Root(root);

    // Add the capture worker to the queue
    auto queue = controller.DispatcherQueue();
    auto success = queue.TryEnqueue([=]() -> void {
            g_App->Initialize(root);
    });
    WINRT_VERIFY(success);

    // Event/Message Loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	return (int)msg.wParam;
}

// Implementation of windows procedure function. Handles event loop messages.
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}