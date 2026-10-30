#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include "renderer.h"
#include "../core/data.h"
#include <Windows.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include "../core/chaos/chaos.h"
#include "../dependencies/slimgui/imgui_impl_dx11.h"
#include "../dependencies/slimgui/imgui_impl_win32.h"
#include "../dependencies/slimgui/fonts/fonts.h"

#pragma comment(lib, "dcomp.lib")

renderer::renderer(HWND window_handle, std::atomic<bool>& application_running, c_data& data) : window_handle(window_handle), data_source(data)
{
    render_background_colour[0] = 0.025f;
    render_background_colour[1] = 0.025f;
    render_background_colour[2] = 0.03f;
    render_background_colour[3] = 0.0f;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &device_pointer, nullptr, &device_context_pointer)))
    {
        throw std::runtime_error("Unable to create D3D11 device.");
    }

    IDXGIDevice* dxgi_device = nullptr;
    IDXGIAdapter* dxgi_adapter = nullptr;
    IDXGIFactory2* dxgi_factory = nullptr;
    if (FAILED(device_pointer->QueryInterface(IID_PPV_ARGS(&dxgi_device))) || FAILED(dxgi_device->GetAdapter(&dxgi_adapter)) || FAILED(dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory))))
    {
        throw std::runtime_error("Unable to initialize DXGI.");
    }

    RECT client_rect{};
    GetClientRect(window_handle, &client_rect);
    DXGI_SWAP_CHAIN_DESC1 scd = {};
    scd.Width = static_cast<UINT>((std::max)(1L, client_rect.right - client_rect.left));
    scd.Height = static_cast<UINT>((std::max)(1L, client_rect.bottom - client_rect.top));
    scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    scd.BufferCount = 2;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.SampleDesc.Count = 1;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    if (FAILED(dxgi_factory->CreateSwapChainForComposition(device_pointer, &scd, nullptr, &swapchain_pointer)))
    {
        throw std::runtime_error("Unable to create composition swap chain.");
    }

    safe_release(dxgi_factory);
    safe_release(dxgi_adapter);
    safe_release(dxgi_device);

    ID3D11Texture2D* back_buffer = nullptr;
    if (FAILED(swapchain_pointer->GetBuffer(0, IID_PPV_ARGS(&back_buffer))) || FAILED(device_pointer->CreateRenderTargetView(back_buffer, nullptr, &render_target_view_pointer)))
    {
        throw std::runtime_error("Unable to create render target.");
    }

    safe_release(back_buffer);

    if (FAILED(DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&dcomp_device))) || FAILED(dcomp_device->CreateTargetForHwnd(window_handle, TRUE, &dcomp_target)) || FAILED(dcomp_device->CreateVisual(&dcomp_visual)) || FAILED(dcomp_visual->SetContent(swapchain_pointer)) || FAILED(dcomp_target->SetRoot(dcomp_visual)) || FAILED(dcomp_device->Commit()))
    {
        throw std::runtime_error("Unable to initialize composition target.");
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImFont* fira_code = io.Fonts->AddFontFromMemoryCompressedTTF(font::fira_code_retina, font::fira_code_retina_size, 16.0f);
    ImFont* visitor = io.Fonts->AddFontFromMemoryCompressedTTF(font::visitor_tt2_brk, font::visitor_tt2_brk_size, 16.0f);
    if (fira_code == nullptr || visitor == nullptr)
    {
        throw std::runtime_error("Unable to load embedded ImGui fonts.");
    }

    io.FontDefault = fira_code;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.06f, 0.08f, 0.96f);
    ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.0f);
    ImGui_ImplWin32_Init(window_handle);
    ImGui_ImplDX11_Init(device_pointer, device_context_pointer);

}

renderer::~renderer()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    safe_release(dcomp_visual);
    safe_release(dcomp_target);
    safe_release(dcomp_device);
    safe_release(render_target_view_pointer);
    safe_release(swapchain_pointer);
    safe_release(device_context_pointer);
    safe_release(device_pointer);
}

void renderer::resize_swap_chain()
{
    if (!swapchain_pointer || !device_context_pointer)
    {
        return;
    }

    safe_release(render_target_view_pointer);
    RECT client_rect{};
    GetClientRect(window_handle, &client_rect);
    const UINT width = client_rect.right - client_rect.left;
    const UINT height = client_rect.bottom - client_rect.top;
    if (width == 0 || height == 0)
    {
        return;
    }

    if (FAILED(swapchain_pointer->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)))
    {
        return;
    }

    ID3D11Texture2D* back_buffer = nullptr;
    if (SUCCEEDED(swapchain_pointer->GetBuffer(0, IID_PPV_ARGS(&back_buffer))))
    {
        device_pointer->CreateRenderTargetView(back_buffer, nullptr, &render_target_view_pointer);
        safe_release(back_buffer);
    }
}
void renderer::resize() { resize_swap_chain(); }

void renderer::render_frame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    if ((GetAsyncKeyState(VK_INSERT) & 1) != 0)
    {
        menu_open = !menu_open;
    }

    if (menu_open)
    {
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(440.0f, 330.0f), ImGuiCond_FirstUseEver);
    }

    const auto snapshot = data_source.get_snapshot();
    if (menu_open)
    {
        const bool show_menu = ImGui::Begin("Chaos Recreation", &menu_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
        if (show_menu)
        {
            ImGui::Text("Geometry: %s", snapshot.geometry ? "ready" : "waiting for data");
            ImGui::Text("Meshes: %u | Vertices: %llu | Triangles: %llu", snapshot.geometry ? snapshot.geometry->mesh_count : 0u, snapshot.geometry ? static_cast<unsigned long long>(snapshot.geometry->vertex_count) : 0ull, snapshot.geometry ? static_cast<unsigned long long>(snapshot.geometry->triangle_count) : 0ull);
            ImGui::Text("Camera: %.1f, %.1f, %.1f", snapshot.camera.location.x, snapshot.camera.location.y, snapshot.camera.location.z);
            ImGui::Text("Rotation: %.1f, %.1f, %.1f | FOV: %.1f | Viewport: %.0f x %.0f", snapshot.camera.rotation.pitch, snapshot.camera.rotation.yaw, snapshot.camera.rotation.roll, snapshot.camera.field_of_view, snapshot.camera.viewport.x, snapshot.camera.viewport.y);
            ImGui::SliderInt("Triangle limit", &triangle_limit, 0, 4'000'000, "%d");
            ImGui::SliderFloat("Render distance", &render_distance, 1.0f, 100'000.0f, "%.0f units");
            ImGui::ColorEdit4("Triangle colour", triangle_colour);
            ImGui::Checkbox("Filled triangles", &fill_triangles);
            ImGui::Text("Triangles drawn: %llu", static_cast<unsigned long long>(triangles_drawn));
        }
        ImGui::End();
    }
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    if (snapshot.geometry)
    {
        const auto colour = ImGui::ColorConvertFloat4ToU32(ImVec4(triangle_colour[0], triangle_colour[1], triangle_colour[2], triangle_colour[3]));
        c_chaos::render(*snapshot.geometry, snapshot.camera, draw_list, static_cast<std::uint64_t>(triangle_limit), render_distance, colour, fill_triangles, &triangles_drawn);
    }
    else
    {
        triangles_drawn = 0;
    }
  
    device_context_pointer->OMSetRenderTargets(1, &render_target_view_pointer, nullptr);
    device_context_pointer->ClearRenderTargetView(render_target_view_pointer, render_background_colour);
    ImGui::Render();
    if (ImDrawData* draw_data = ImGui::GetDrawData())
    {
        ImGui_ImplDX11_RenderDrawData(draw_data);
    }
    swapchain_pointer->Present(1, 0);
}
