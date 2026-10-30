#define NOMINMAX
#include <Windows.h>
#include <exception>
#include <chrono>
#include <thread>

#include "core/structs.h"
#include "core/data.h"

#include "memory/memory.h"
#include "window/window.h"
#include "rendering/renderer.h"
#include "dependencies/logging/logging.h"

#ifdef NDEBUG
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#else
int main(int argc, char* argv[])
{
#endif 

    LOG_DEBUG("application started");
    std::atomic<bool> application_running = true;

    auto core_data = std::make_unique<c_data>(application_running);
    if (!application_running.load())
    {
        return -1;
    }

    constexpr auto tick_interval = std::chrono::milliseconds(3);
    std::thread tick_thread([&]()
        {
            if (core_data->begin())
            {
                while (application_running.load())
                {
                    core_data->tick();
                    std::this_thread::sleep_for(tick_interval);
                }
            }
        });

    std::unique_ptr<window> main_window;
    try
    {
        main_window = std::make_unique<window>(nullptr, L" ", 300, 80, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, WS_EX_NOREDIRECTIONBITMAP, RGB(13, 13, 15), RGB(13, 13, 15));
    }
    catch (const std::exception& error)
    {
        LOG_ERROR("Window creation failed: %s", error.what());
        application_running.store(false, std::memory_order_release);
        tick_thread.join();
        return -1;
    }
    main_window->expand_to_fullscreen();
    LOG_DEBUG("window created and setup");

    std::unique_ptr<renderer> main_renderer;
    try
    {
        main_renderer = std::make_unique<renderer>(main_window->get_handle(), application_running, *core_data);
    }
    catch (const std::exception& error)
    {
        LOG_ERROR("Renderer creation failed: %s", error.what());
        application_running.store(false, std::memory_order_release);
        tick_thread.join();
        return -1;
    }
    LOG_DEBUG("renderer created and setup");
    main_window->show(SW_SHOW, false);

    while (application_running.load() && main_window->process_messages(application_running))
    {
        if ((GetAsyncKeyState(VK_END) & 1) != 0)
        {
            application_running.store(false, std::memory_order_release);
            PostMessageW(main_window->get_handle(), WM_CLOSE, 0, 0);
            break;
        }
        main_renderer->render_frame();
    }

    LOG_DEBUG("application shutting down");
    application_running.store(false, std::memory_order_release);
    tick_thread.join();
    return 0;
}