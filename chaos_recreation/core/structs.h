#pragma once

#include <cstddef>
#include <array>
#include <cmath>
#include <memory>
#include <vector>


struct vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

struct vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct frotator
{
    double pitch = 0.0;
    double yaw = 0.0;
    double roll = 0.0;
};
struct fvector
{
    double x{};
    double y{};
    double z{};
};

struct fname
{
    std::uint32_t comparison_index{};
    std::uint32_t number{};
};

struct fquat
{
    double x{};
    double y{};
    double z{};
    double w{};
};

// UE5 LWC FTransform layout: Rotation, Translation, Scale3D, with native padding.
struct ftransform
{
    fquat rotation{};
    fvector translation{};
    std::uint8_t padding_38[8]{};
    fvector scale3d{};
    std::uint8_t padding_58[8]{};
};

struct fvector3f
{
    float x{};
    float y{};
    float z{};
};

template <typename index_type>
struct ftriangle
{
    index_type a{};
    index_type b{};
    index_type c{};
};

using ftriangle16 = ftriangle<std::uint16_t>;
using ftriangle32 = ftriangle<std::int32_t>;




// CameraCache POV fields previously read as a contiguous prefix.
struct fminimal_view_info
{
    fvector location{};
    frotator rotation{};
    float fov{};
};

static_assert(sizeof(ftransform) == 0x60);
static_assert(offsetof(ftransform, translation) == 0x20);
static_assert(offsetof(ftransform, scale3d) == 0x40);
static_assert(sizeof(fvector3f) == 0x0C);
static_assert(sizeof(fname) == 0x08);
static_assert(sizeof(ftriangle16) == 0x06);
static_assert(sizeof(ftriangle32) == 0x0C);

template <typename value_type>
struct t_array
{
    std::uintptr_t data{};
    std::int32_t size{};
    std::int32_t capacity{};

    constexpr std::uintptr_t at(std::uint32_t index) const noexcept
    {
        return data + static_cast<std::uintptr_t>(index) * sizeof(value_type);
    }
};
static_assert(sizeof(t_array<std::uint64_t>) == 0x10);
static_assert(offsetof(t_array<std::uint64_t>, size) == 0x08);
static_assert(offsetof(t_array<std::uint64_t>, capacity) == 0x0C);

struct camera_state
{
    vec3 location;
    frotator rotation;
    float field_of_view = 90.0f;
    vec2 viewport;

    bool world_to_screen(const vec3& world, vec2& screen) const
    {
        if (viewport.x <= 0.0f || viewport.y <= 0.0f || field_of_view <= 1.0f || field_of_view >= 179.0f)
        {
            return false;
        }

        constexpr float radians = 0.017453292519943295f;
        const float pitch = static_cast<float>(rotation.pitch) * radians;
        const float yaw = static_cast<float>(rotation.yaw) * radians;
        const float roll = static_cast<float>(rotation.roll) * radians;
        const float sp = std::sin(pitch), cp = std::cos(pitch);
        const float sy = std::sin(yaw), cy = std::cos(yaw);
        const float sr = std::sin(roll), cr = std::cos(roll);

        const float dx = world.x - location.x;
        const float dy = world.y - location.y;
        const float dz = world.z - location.z;
        const float camera_x = dx * (sr * sp * cy - cr * sy) + dy * (sr * sp * sy + cr * cy) - dz * sr * cp;
        const float camera_y = dx * -(cr * sp * cy + sr * sy) + dy * (cy * sr - cr * sp * sy) + dz * cr * cp;
        const float depth = dx * cp * cy + dy * cp * sy + dz * sp;
        if (depth <= 0.001f)
        {
            return false;
        }

        const float tangent_fov = std::tan(0.5f * field_of_view * radians);
        if (!std::isfinite(tangent_fov) || std::fabs(tangent_fov) < 1.0e-6f)
        {
            return false;
        }

        const float center_x = viewport.x * 0.5f;
        const float center_y = viewport.y * 0.5f;
        const float scale = center_x / (tangent_fov * depth);
        screen.x = center_x + camera_x * scale;
        screen.y = center_y - camera_y * scale;

        return std::isfinite(screen.x) && std::isfinite(screen.y) && screen.x >= 0.0f && screen.x < viewport.x && screen.y >= 0.0f && screen.y < viewport.y;
    }
};

struct geometry_instance
{
    std::uint32_t first_index{};
    std::uint32_t index_count{};
    ftransform transform{};
};

struct geometry_scene
{
    std::vector<std::array<double, 3>> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<geometry_instance> instances;
    std::uint32_t mesh_count = 0;
    std::uint64_t vertex_count = 0;
    std::uint64_t triangle_count = 0;
};


struct game_offsets
{
    // the holy grail!
    std::uint64_t gworld_rva = 0x095C91C0; // GWorld (global UWorld*; RVA)

    // initial world data
    std::uint64_t world_levels = 0x1C8; // UWorld::Levels
    std::uint64_t world_game_instance = 0x228; // UWorld::OwningGameInstance
    std::uint64_t game_instance_local_players = 0x38; // UGameInstance::LocalPlayers

    // camera stuff
    std::uint64_t local_player_controller = 0x30; // ULocalPlayer::PlayerController
    std::uint64_t controller_camera_manager = 0x368; // APlayerController::PlayerCameraManager
    std::uint64_t camera_manager_view = 0x1550 + 0x10;    // APlayerCameraManager::CameraCachePrivate + POV

    // world stuff
    std::uint64_t level_actors = 0xA0; // ULevel::Actors
    std::uint64_t actor_fname = 0x18; // UObject::NamePrivate (FName)
    std::uint64_t actor_root_component = 0x1B8; // AActor::RootComponent
    std::uint64_t component_to_world = 0x1F0; // USceneComponent::ComponentToWorld
    std::uint64_t component_static_mesh = 0x588; // UStaticMeshComponent::StaticMesh
    std::uint64_t static_mesh_body_setup = 0x1D8; // UStaticMesh::BodySetup
};


struct data_snapshot
{

    camera_state camera;
    std::shared_ptr<const geometry_scene> geometry;
};
