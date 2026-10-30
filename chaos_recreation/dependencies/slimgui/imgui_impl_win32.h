// dear imgui v1.92.9b backend - slim copy for slimgui (comments stripped)
// Copyright (c) 2014-2026 Omar Cornut - MIT License, see LICENSE.txt

#pragma once
#include "imgui.h"
#ifndef IMGUI_DISABLE

IMGUI_IMPL_API bool ImGui_ImplWin32_Init(void *hwnd);
IMGUI_IMPL_API bool ImGui_ImplWin32_InitForOpenGL(void *hwnd);
IMGUI_IMPL_API void ImGui_ImplWin32_Shutdown();
IMGUI_IMPL_API void ImGui_ImplWin32_NewFrame();

IMGUI_IMPL_API void ImGui_ImplWin32_EnableDpiAwareness();
IMGUI_IMPL_API float ImGui_ImplWin32_GetDpiScaleForHwnd(void *hwnd);
IMGUI_IMPL_API float ImGui_ImplWin32_GetDpiScaleForMonitor(void *monitor);

IMGUI_IMPL_API void ImGui_ImplWin32_EnableAlphaCompositing(void *hwnd);

#endif
