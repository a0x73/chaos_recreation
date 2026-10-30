#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstdint>
#include <optional>
#include <string>
#include <atomic>

class window
{
public:
    window(HINSTANCE instance_handle, const std::wstring& title, std::uint32_t width, std::uint32_t height, std::uint32_t style, std::uint32_t extended_style = 0, std::optional<std::uint32_t> title_colour = RGB(32, 32, 32), std::optional<std::uint32_t> border_colour = RGB(32, 32, 32));
    ~window();

    window(const window&) = delete;
    window& operator=(const window&) = delete;
    window(window&&) = delete;
    window& operator=(window&&) = delete;

    void show(std::uint32_t show_cmd, bool should_center);
    bool process_messages(std::atomic<bool>& application_running);
    void expand_to_fullscreen() const;

    HWND get_handle() const noexcept
    {
        return window_handle;
    }

private:
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    HWND window_handle = nullptr;
    HINSTANCE instance_handle = nullptr;

    const wchar_t* class_name = L"Lh";
};
