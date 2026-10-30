#pragma once
#include <cstdint>
#include "../structs.h"
#include "../../dependencies/slimgui/imgui.h"

/*
* 
FBodyInstance::GetWalkableSlopeOverride
40 53 48 83 EC 20 F6 41 3D 20 48 8B D9 75 1D E8 ?? ?? ?? ?? 84 C0 74 14 48 8B CB E8 ?? ?? ?? ?? 48 05 E8 00 00 00 48
83 C4 20 5B C3 48 8D 83 10 01 00 00 48 83 C4 20 5B C3

40 53 48 83 EC ? F6 41 ? ? 48 8B D9 75 ? E8 ? ? ? ? 84 C0

this + 0x30 is chaos tri meshes

constexpr std::uint64_t body_setup_chaos_tri_meshes = 0x118; // UBodySetup::ChaosTriMeshes


// Chaos triangle mesh data.
Chaos::FTriangleMeshImplicitObject::GetFaceNormal(__int64 a1, double *a2, int a3)
4C 8B 59 ? 0F 29 74 24 ? 0F 29 7C 24 ? 44 0F 29 44 24 ? 44 0F 29 0C 24 49 63 C0 74 ? 48 8B 49

constexpr std::uint64_t triangle_mesh_vertices = 0x48; // FTriangleMeshImplicitObject::MParticles (TParticles::X() positions)
constexpr std::uint64_t triangle_mesh_small_indices = 0x70; // FTrimeshIndexBuffer::SmallIdxBuffer
constexpr std::uint64_t triangle_mesh_large_indices = 0x60; // FTrimeshIndexBuffer::LargeIdxBuffer
constexpr std::uint64_t triangle_mesh_requires_large_indices = 0x80; // FTrimeshIndexBuffer::bRequiresLargeIndices

*/ 
class c_chaos
{
private:

    std::int32_t gworld_offset = 0x0;
    std::uint64_t current_world = 0x0;
    t_array<std::uint64_t> current_levels = t_array<std::uint64_t>();


    // chaos specifics, readme has signatures to find these, if not you could bruteforce them.
    std::int32_t chaos_tri_meshes = 0x118;              // UBodySetup::ChaosTriMeshes
    std::int32_t chaos_mesh_vertices = 0x48;            // FTriangleMeshImplicitObject::MParticles

    struct ftri_mesh_index_buffer
    {
        std::int32_t large_idx_buffer = 0x60;           // FTrimeshIndexBuffer::SmallIdxBuffer
        std::int32_t small_idx_buffer = 0x70;           // FTrimeshIndexBuffer::LargeIdxBuffer
        std::int32_t b_requires_large_indices = 0x80;   // FTrimeshIndexBuffer::bRequiresLargeIndices
    } chaos_mesh_offsets;

public:

	c_chaos(std::int32_t gworld_offset);
	~c_chaos();


    void cache_geometry(geometry_scene& scene, game_offsets& offsets);
    static void render(const geometry_scene& scene, const camera_state& camera, ImDrawList* draw_list, std::uint64_t triangle_limit, float max_distance, ImU32 draw_colour, bool filled, std::uint64_t* triangles_drawn);
    void clear_data(geometry_scene& scene);
};