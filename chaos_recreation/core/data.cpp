#include "../memory/memory.h"
#include "data.h"
#include "../dependencies/logging/logging.h"
#include "chaos/chaos.h"

c_data::c_data(std::atomic<bool>& application_running) : application_running(application_running)
{
}

c_data::~c_data() = default;

data_snapshot c_data::get_snapshot() const
{
    std::lock_guard lock(snapshot_mutex);
    return snapshot;
}

bool c_data::begin()
{
    if (mem.load(process_name))
    {
        LOG_DEBUG("process loaded: %s", process_name.c_str());
        return true;
    }

    LOG_WARN("process not found: %s", process_name.c_str());
    application_running.store(false, std::memory_order_release);
    return false;
}

void c_data::tick()
{
    static bool scanned = false;
    const auto world = mem.read<std::uint64_t>(mem.get_base() + offsets.gworld_rva);

    const auto levels = mem.read<t_array<std::uint64_t>>(world + offsets.world_levels);
    if (chaos == nullptr)
    {
        chaos = std::make_unique<c_chaos>(offsets.gworld_rva);
    }

    data_snapshot next_snapshot{};
    if (scanned)
    {
        std::lock_guard lock(snapshot_mutex);
        next_snapshot.geometry = snapshot.geometry;
    }

    const auto game_instance = mem.read<std::uint64_t>(world + offsets.world_game_instance);
    const auto local_players = mem.read<t_array<std::uint64_t>>(game_instance + offsets.game_instance_local_players);
    const auto local_player = local_players.data && local_players.size > 0 ? mem.read<std::uint64_t>(local_players.data) : 0;
    const auto controller = local_player ? mem.read<std::uint64_t>(local_player + offsets.local_player_controller) : 0;
    const auto camera_manager = controller ? mem.read<std::uint64_t>(controller + offsets.controller_camera_manager) : 0;

    const auto view = mem.read<fminimal_view_info>(camera_manager + offsets.camera_manager_view);
    auto& camera = next_snapshot.camera;
    camera.location = { static_cast<float>(view.location.x), static_cast<float>(view.location.y), static_cast<float>(view.location.z) };
    camera.rotation = view.rotation;
    camera.field_of_view = view.fov;
    camera.viewport = { static_cast<float>(GetSystemMetrics(SM_CXSCREEN)),static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) };
    

    // this is just for now for testing, one single scan over of everything. realistically this wouldnt be the case. but thats for you to worry about.
    if (scanned)
    {
        std::lock_guard lock(snapshot_mutex);
        snapshot = std::move(next_snapshot);
        return;
    }

    auto scene = std::make_shared<geometry_scene>();
    chaos->cache_geometry(*scene, offsets);

    // in a real scenario you may get different worlds and thus, different geometry across maps or scenarios.
    // for this you would detect when that change happens and just call the clear function.

    // chaos->clear_data();

    // which will then just clear all the vectors and all data stored on the geometry which then your next cache_geometry call will regather and handle all up to date.
    next_snapshot.geometry = std::move(scene);
    {
        std::lock_guard lock(snapshot_mutex);
        snapshot = std::move(next_snapshot);
    }
    scanned = true;
    LOG_DEBUG("singular scan finished");
}
