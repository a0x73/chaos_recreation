// slimgui compile-time configuration for the vendored dear imgui sources.
// every translation unit that includes imgui.h must see these same defines.

#pragma once

// assertions are stripped, failures are handled by the caller instead
#define IM_ASSERT(_EXPR) ((void)(_EXPR))

// debugger breaks are stripped
#define IM_DEBUG_BREAK() ((void)0)

// debug and demo tooling is stripped
#define IMGUI_DISABLE_DEBUG_TOOLS
#define IMGUI_DISABLE_DEMO_WINDOWS

// obsolete api is stripped
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

// the built-in Proggy default fonts are dropped (both the vector/ProggyForever
// and the bitmap/ProggyClean variant); slimgui always embeds its own visitor + Fira
#define IMGUI_DISABLE_DEFAULT_FONT_VECTOR
#define IMGUI_DISABLE_DEFAULT_FONT_BITMAP

// imvec2/imvec4 math helpers used by the layout and style code
#define IMGUI_DEFINE_MATH_OPERATORS
