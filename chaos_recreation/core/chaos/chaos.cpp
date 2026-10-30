#include "chaos.h"
#include "../../memory/memory.h"
#include "../../dependencies/slimgui/imgui.h"
#include <algorithm>
#include <cmath>

c_chaos::c_chaos(std::int32_t gworld_offset) : gworld_offset(gworld_offset)
{
}

c_chaos::~c_chaos()
{
}


void c_chaos::cache_geometry(geometry_scene& scene, game_offsets& offsets)
{
    current_world = mem.read<std::uint64_t>(mem.get_base() + gworld_offset);
    current_levels = mem.read<t_array<std::uint64_t>>(current_world + offsets.world_levels);

    if (current_world == 0)
    {
        return;
    }

    for (std::int32_t level_index = 0; level_index < current_levels.size; ++level_index)
    {
        const auto level = mem.read<std::uint64_t>(current_levels.at(static_cast<std::uint32_t>(level_index)));
        const auto actors = mem.read<t_array<std::uint64_t>>(level + offsets.level_actors);
        for (std::int32_t actor_index = 0; actor_index < actors.size; ++actor_index)
        {
            const auto actor = mem.read<std::uint64_t>(actors.at(static_cast<std::uint32_t>(actor_index)));
            const auto root = mem.read<std::uint64_t>(actor + offsets.actor_root_component);
            std::vector<std::uint64_t> components{ root };
            std::vector<std::uint64_t> visited_components;
            for (std::size_t component_index = 0; component_index < components.size(); ++component_index)
            {
                const auto component = components[component_index];
                if (std::find(visited_components.begin(), visited_components.end(), component) != visited_components.end())
                {
                    continue;
                }
                visited_components.push_back(component);

                const auto children = mem.read<t_array<std::uint64_t>>(component + 0xE8);
                for (std::int32_t child_index = 0; child_index < children.size; ++child_index)
                {
                    components.push_back(mem.read<std::uint64_t>(children.at(static_cast<std::uint32_t>(child_index))));
                }

                const auto mesh = mem.read<std::uint64_t>(component + offsets.component_static_mesh);
                const auto body_setup = mem.read<std::uint64_t>(mesh + offsets.static_mesh_body_setup);
                const auto chaos_meshes = mem.read<t_array<std::uint64_t>>(body_setup + chaos_tri_meshes);
                const auto transform = mem.read<ftransform>(component + offsets.component_to_world);

                if (chaos_meshes.size > 5'000'000)
                {
                    continue;
                }

                for (std::int32_t mesh_index = 0; mesh_index < chaos_meshes.size; ++mesh_index)
                {
                    const auto triangle_mesh = mem.read<std::uint64_t>(chaos_meshes.at(static_cast<std::uint32_t>(mesh_index)));
                    const auto vertex_array = mem.read<t_array<fvector3f>>(triangle_mesh + chaos_mesh_vertices);

                    const auto large_index_flag = mem.read<std::int8_t>(triangle_mesh + chaos_mesh_offsets.b_requires_large_indices);
                    const bool requires_large_indices = large_index_flag != 0;
                    const auto index_array_offset = requires_large_indices ? chaos_mesh_offsets.large_idx_buffer : chaos_mesh_offsets.small_idx_buffer;

                    const auto index_array = mem.read<t_array<std::uint16_t>>(triangle_mesh + index_array_offset);
                    const auto raw_num = index_array.size;
                    const auto triangle_data = index_array.data;

                    if (raw_num <= 0 || raw_num > 7'000'000 || !triangle_data)
                    {
                        continue;
                    }

                    const std::int32_t index_count_remote = raw_num * 3;
                    const auto triangle_count = static_cast<std::uint32_t>(index_count_remote / 3);

                    if (chaos_meshes.size <= 0 || chaos_meshes.size > 64)
                    {
                        continue;
                    }

                    const auto first_vertex = static_cast<std::uint32_t>(scene.vertices.size());
                    const auto first_index = scene.indices.size();
                    std::vector<fvector3f> vertices(static_cast<std::size_t>(vertex_array.size));
                    if (!mem.read(vertex_array.data, vertices.data(), vertices.size() * sizeof(fvector3f)))
                    {
                        continue;
                    }

                    const auto index_count = static_cast<std::size_t>(index_count_remote);
                    std::vector<std::uint32_t> mesh_indices(index_count);
                    if (requires_large_indices)
                    {
                        if (!mem.read(triangle_data, mesh_indices.data(), mesh_indices.size() * sizeof(std::uint32_t)))
                        {
                            continue;
                        }
                    }
                    else
                    {
                        std::vector<std::uint16_t> small_indices(index_count);
                        if (!mem.read(triangle_data, small_indices.data(), small_indices.size() * sizeof(std::uint16_t)))
                        {
                            continue;
                        }
                        for (std::size_t index = 0; index < index_count; ++index)
                        {
                            mesh_indices[index] = small_indices[index];
                        }
                    }

                    if (std::any_of(mesh_indices.begin(), mesh_indices.end(), [&](std::uint32_t index) { return index >= vertices.size(); }))
                    {
                        continue;
                    }

                    for (const auto& vertex : vertices)
                    {
                        scene.vertices.push_back({ vertex.x, vertex.y, vertex.z });
                    }

                    for (const auto index : mesh_indices)
                    {
                        scene.indices.push_back(first_vertex + index);
                    }

                    scene.vertex_count += vertices.size();
                    scene.triangle_count += static_cast<std::uint64_t>(triangle_count);
                    ++scene.mesh_count;
                    scene.instances.push_back
                    ({
                        static_cast<std::uint32_t>(first_index),
                        static_cast<std::uint32_t>(scene.indices.size() - first_index),
                        transform
                    });
                }
            }
        }
    }

}

void c_chaos::render(const geometry_scene& scene, const camera_state& camera, ImDrawList* draw_list, std::uint64_t triangle_limit, float max_distance, ImU32 draw_colour, bool filled, std::uint64_t* triangles_drawn)
{
    if (triangles_drawn)
    {
        *triangles_drawn = 0;
    }

    if (draw_list == nullptr || triangle_limit == 0 || max_distance <= 0.0f)
    {
        return;
    }

    const double max_distance_squared = static_cast<double>(max_distance) * max_distance;
    std::uint64_t drawn_triangles = 0;
    for (const auto& instance : scene.instances)
    {
        const auto transform_vertex = [&](const std::array<double, 3>& vertex)
        {
            const double x = vertex[0] * instance.transform.scale3d.x;
            const double y = vertex[1] * instance.transform.scale3d.y;
            const double z = vertex[2] * instance.transform.scale3d.z;
            const double qx = instance.transform.rotation.x, qy = instance.transform.rotation.y;
            const double qz = instance.transform.rotation.z, qw = instance.transform.rotation.w;
            const double dot = qx * x + qy * y + qz * z;
            const double norm = qx * qx + qy * qy + qz * qz;
            return vec3
            {
                static_cast<float>(instance.transform.translation.x + 2.0 * qx * dot + (qw * qw - norm) * x + 2.0 * qw * (qy * z - qz * y)),
                static_cast<float>(instance.transform.translation.y + 2.0 * qy * dot + (qw * qw - norm) * y + 2.0 * qw * (qz * x - qx * z)),
                static_cast<float>(instance.transform.translation.z + 2.0 * qz * dot + (qw * qw - norm) * z + 2.0 * qw * (qx * y - qy * x))
            };
        };

        const auto end = (std::min<std::size_t>)(scene.indices.size(), static_cast<std::size_t>(instance.first_index) + instance.index_count);
        for (std::size_t index = instance.first_index; index + 2 < end; index += 3)
        {
            if (drawn_triangles >= triangle_limit)
            {
                break;
            }
            const auto ia = scene.indices[index];
            const auto ib = scene.indices[index + 1];
            const auto ic = scene.indices[index + 2];
            if (ia >= scene.vertices.size() || ib >= scene.vertices.size() || ic >= scene.vertices.size())
            {
                continue;
            }

            const vec3 points[] = 
            {
                transform_vertex(scene.vertices[ia]),
                transform_vertex(scene.vertices[ib]),
                transform_vertex(scene.vertices[ic])
            };

            bool within_distance = true;
            for (const auto& point : points)
            {
                const double dx = static_cast<double>(point.x) - camera.location.x;
                const double dy = static_cast<double>(point.y) - camera.location.y;
                const double dz = static_cast<double>(point.z) - camera.location.z;
                if (dx * dx + dy * dy + dz * dz > max_distance_squared)
                {
                    within_distance = false;
                    break;
                }
            }
            if (!within_distance)
            {
                continue;
            }

            vec2 projected[3]{};
            const bool visible[3] =
            {
                camera.world_to_screen(points[0], projected[0]),
                camera.world_to_screen(points[1], projected[1]),
                camera.world_to_screen(points[2], projected[2])
            };
            if (filled)
            {
                if (!(visible[0] && visible[1] && visible[2]))
                {
                    continue;
                }

                draw_list->AddTriangleFilled(ImVec2(projected[0].x, projected[0].y), ImVec2(projected[1].x, projected[1].y), ImVec2(projected[2].x, projected[2].y), draw_colour);
                ++drawn_triangles;
            }
            else
            {
                bool drew_triangle = false;
                if (visible[0] && visible[1])
                {
                    draw_list->AddLine(ImVec2(projected[0].x, projected[0].y), ImVec2(projected[1].x, projected[1].y), draw_colour);
                    drew_triangle = true;
                }
                if (visible[1] && visible[2])
                {
                    draw_list->AddLine(ImVec2(projected[1].x, projected[1].y), ImVec2(projected[2].x, projected[2].y), draw_colour);
                    drew_triangle = true;
                }
                if (visible[2] && visible[0])
                {
                    draw_list->AddLine(ImVec2(projected[2].x, projected[2].y), ImVec2(projected[0].x, projected[0].y), draw_colour);
                    drew_triangle = true;
                }
                if (drew_triangle)
                {
                    ++drawn_triangles;
                }
            }
        }
        if (drawn_triangles >= triangle_limit)
        {
            break;
        }
    }

    if (triangles_drawn)
    {
        *triangles_drawn = drawn_triangles;
    }
}

void c_chaos::clear_data(geometry_scene& scene)
{
    scene.indices.clear();
    scene.instances.clear();
    scene.mesh_count = 0;
    scene.triangle_count = 0;
    scene.vertex_count = 0;
    scene.vertices.clear();

    current_world = 0x0;
    current_levels = {};
}

