#include "window.h"

#include <stdexcept>
#include <vector>

#include <dwmapi.h>

#include "../dependencies/slimgui/imgui.h"
#include "../dependencies/slimgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
    constexpr DWORD dwmwa_border_colour = 34;
    constexpr DWORD dwmwa_caption_colour = 35;
    constexpr DWORD dwmwa_use_immersive_dark_mode = 20;
}

window::window(HINSTANCE instance_handle, const std::wstring& title, std::uint32_t width, std::uint32_t height, std::uint32_t style, std::uint32_t extended_style, std::optional<std::uint32_t> title_colour, std::optional<std::uint32_t> border_colour)
    : instance_handle(instance_handle)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window::window_proc;
    wc.hInstance = instance_handle;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = class_name;

    if (!RegisterClassExW(&wc))
    {
        throw std::runtime_error("Unable to register class for window.");
    }

    window_handle = CreateWindowExW(extended_style, wc.lpszClassName, title.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, wc.hInstance, this);

    if (window_handle == nullptr)
    {
        UnregisterClassW(class_name, instance_handle);
        throw std::runtime_error("Window hwnd null.");
    }

    if (title_colour.has_value())
    {
        COLORREF colour = title_colour.value();
        DwmSetWindowAttribute(window_handle, dwmwa_caption_colour, &colour, sizeof(colour));
    }

    if (border_colour.has_value())
    {
        COLORREF colour = border_colour.value();
        DwmSetWindowAttribute(window_handle, dwmwa_border_colour, &colour, sizeof(colour));
    }

    BOOL dark = TRUE;
    DwmSetWindowAttribute(window_handle, dwmwa_use_immersive_dark_mode, &dark, sizeof(dark));

    const LONG_PTR ex_style = GetWindowLongPtrW(window_handle, GWL_EXSTYLE);
    SetWindowLongPtrW(window_handle, GWL_EXSTYLE, ex_style | WS_EX_DLGMODALFRAME);

    SetWindowLongPtrW(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

window::~window()
{
    if (window_handle)
    {
        DestroyWindow(window_handle);
        window_handle = nullptr;
    }
    if (instance_handle)
    {
        UnregisterClassW(class_name, instance_handle);
    }
}

void window::show(std::uint32_t show_cmd, bool should_center)
{
    ShowWindow(window_handle, show_cmd);
    UpdateWindow(window_handle);
    SetForegroundWindow(window_handle);
    SetFocus(window_handle);
}


void window::expand_to_fullscreen() const
{
    const int w = GetSystemMetrics(SM_CXSCREEN);
    const int h = GetSystemMetrics(SM_CYSCREEN);
    SetWindowLongPtrW(window_handle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowLongPtrW(window_handle, GWL_EXSTYLE, WS_EX_NOREDIRECTIONBITMAP);
    SetWindowPos(window_handle, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED | SWP_NOACTIVATE);
}

bool window::process_messages(std::atomic<bool>& application_running)
{
    MSG message = {};
    while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            application_running = false;
            return false;
        }

        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return true;
}

LRESULT CALLBACK window::window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_NCCREATE)
    {
        const auto* create_pointer = reinterpret_cast<CREATESTRUCT*>(lparam);
        auto* window_pointer = reinterpret_cast<window*>(create_pointer->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window_pointer));
    }

    if (message == WM_NCHITTEST && ImGui::GetCurrentContext() != nullptr)
    {
        const ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse ? HTCLIENT : HTTRANSPARENT;
    }

    if (ImGui::GetCurrentContext() != nullptr && ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
        return 0;

    if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}
