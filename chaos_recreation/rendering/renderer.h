#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#include <d3d11.h>
#include <dcomp.h>
#include <dxgi1_2.h>
#include "../dependencies/slimgui/imgui.h"
struct HWND__;
using HWND = HWND__*;
class c_data;

class renderer
{
private:
    ID3D11Device* device_pointer = nullptr;
    ID3D11DeviceContext* device_context_pointer = nullptr;
    IDXGISwapChain1* swapchain_pointer = nullptr;
    ID3D11RenderTargetView* render_target_view_pointer = nullptr;

    IDCompositionDevice* dcomp_device = nullptr;
    IDCompositionTarget* dcomp_target = nullptr;
    IDCompositionVisual* dcomp_visual = nullptr;

    HWND window_handle = nullptr;
    c_data& data_source;
    float render_background_colour[4] = { 19 / 255.f, 19 / 255.f, 19 / 255.f, 1.0f };
    std::uint64_t triangles_drawn = 0;
    int triangle_limit = 10'000;
    float render_distance = 50'000.0f;
    float triangle_colour[4] = { 79 / 255.0f, 199 / 255.0f, 255 / 255.0f, 1.0f };
    bool fill_triangles = false;
    bool menu_open = true;
    void resize_swap_chain();

    template <typename T>
    static void safe_release(T*& pointer)
    {
        if (pointer)
        {
            pointer->Release();
            pointer = nullptr;
        }
    }


public:
    renderer(HWND window_handle, std::atomic<bool>& application_running, c_data& data);
    renderer(const renderer&) = delete;
    renderer& operator=(const renderer&) = delete;
    renderer(renderer&&) = delete;
    renderer& operator=(renderer&&) = delete;
    ~renderer();

    void render_frame();
    void resize();
};
