#include "renderer/shadow.hpp"
#include "graphics/corner.hpp"
#include "graphics/culling.hpp"
#include "graphics/smallest-bound.hpp"
#include "target.hpp"

#include <algorithm>

namespace renderer
{
	static glm::vec3 homo_transform(const glm::mat4& mat, const glm::vec3& vec) noexcept
	{
		const glm::vec4 homo = mat * glm::vec4(vec, 1.0f);
		return glm::vec3(homo) / homo.w;
	}

	Shadow::Drawdata::Drawdata(
		const glm::mat4& camera_matrix,
		const glm::vec3& light_direction,
		float min_z,
		float linear_blend_ratio
	) noexcept
	{
		const auto camera_mat_inv = glm::inverse(camera_matrix);

		// Compute CSM split points

		const glm::vec3 near_world_pos = homo_transform(camera_mat_inv, {0, 0, 1}),
						far_world_pos = homo_transform(camera_mat_inv, {0, 0, min_z});
		const glm::vec3 linear_split1_world_pos = glm::mix(near_world_pos, far_world_pos, 0.3333),
						linear_split2_world_pos = glm::mix(near_world_pos, far_world_pos, 0.6667);
		const float linear_split1_z = homo_transform(camera_matrix, linear_split1_world_pos).z,
					linear_split2_z = homo_transform(camera_matrix, linear_split2_world_pos).z;
		const float log_split1_z = glm::mix(1.0f, min_z, 0.3333),
					log_split2_z = glm::mix(1.0f, min_z, 0.6667);
		const float split1_z = glm::mix(log_split1_z, linear_split1_z, linear_blend_ratio),
					split2_z = glm::mix(log_split2_z, linear_split2_z, linear_blend_ratio);

		const std::array<float, 4> z_series = {1.0f, split1_z, split2_z, min_z};

		for (auto [level, z_pair] : std::views::zip(csm_levels, z_series | std::views::adjacent<2>))
		{
			const auto [z_near, z_far] = z_pair;

			const auto corners = graphics::transform_corner_points(
				graphics::get_corner_points({-1, -1, z_far}, {1, 1, z_near}),
				camera_mat_inv
			);

			level.smallest_bound = graphics::find_smallest_bound(corners, light_direction);

			const auto temp_vp_matrix =
				glm::ortho(
					level.smallest_bound.left,
					level.smallest_bound.right,
					level.smallest_bound.bottom,
					level.smallest_bound.top,
					0.0f,
					1.0f
				)
				* level.smallest_bound.view_matrix;

			level.frustum_planes = graphics::compute_frustum_planes(temp_vp_matrix);
		}
	}

	void Shadow::Drawdata::CSM_level_data::append(const gltf::Drawdata& drawdata) noexcept
	{
		material_sets.push_back(drawdata.material_cache);
		const auto current_material_set = material_sets.size() - 1;

		for (const auto& drawcall : drawdata.objects)
			if (graphics::box_in_frustum(
					drawcall.world_position_min,
					drawcall.world_position_max,
					std::span(frustum_planes.data(), 4)
				))
			{
				const auto& pipeline = drawdata.material_cache[drawcall.material_index].params.pipeline;
				auto& target = drawcalls[pipeline];

				const auto corners_world =
					graphics::get_corner_points(drawcall.world_position_min, drawcall.world_position_max);
				const auto corners_light_view =
					graphics::transform_corner_points(corners_world, smallest_bound.view_matrix);

				const auto [min_z, max_z] = std::ranges::minmax(corners_light_view, {}, &glm::vec3::z);
				near = std::min(near, -max_z.z);
				far = std::max(far, -min_z.z);

				if (target.empty()) target.reserve(1024);
				target.emplace_back(
					Drawcall{.drawcall = drawcall, .material_set_index = current_material_set}
				);
			}
	}

	glm::mat4 Shadow::Drawdata::CSM_level_data::get_vp_matrix() const noexcept
	{
		const auto projection_matrix = glm::ortho(
			smallest_bound.left,
			smallest_bound.right,
			smallest_bound.bottom,
			smallest_bound.top,
			near,
			far
		);

		return projection_matrix * smallest_bound.view_matrix;
	}

	void Shadow::Drawdata::append(const gltf::Drawdata& drawdata) noexcept
	{
		for (auto& level : csm_levels) level.append(drawdata);
	}

	glm::mat4 Shadow::Drawdata::get_vp_matrix(size_t level) const noexcept
	{
		return csm_levels[level].get_vp_matrix();
	}

	std::expected<Shadow, util::Error> Shadow::create(SDL_GPUDevice* device) noexcept
	{
		return pipeline::Shadow::create(device)
			.transform([](pipeline::Shadow&& shadow_pipeline) noexcept {
				return Shadow{std::move(shadow_pipeline)};
			})
			.transform_error(util::Error::forward_fn("Create shadow pipeline failed"));
	}

	std::expected<void, util::Error> Shadow::render(
		const gpu::Command_buffer& command_buffer,
		const target::Shadow& shadow_target,
		const Drawdata& drawdata
	) const noexcept
	{
		for (const auto [level, level_data] : drawdata.csm_levels | std::views::enumerate)
		{
			auto shadow_pass_result = target::acquire_shadow_pass(command_buffer, shadow_target, level);
			if (!shadow_pass_result)
				return shadow_pass_result.error().forward("Acquire shadow render pass failed");
			auto shadow_pass = std::move(*shadow_pass_result);

			for (const auto& [pipeline_cfg, drawcalls] : level_data.drawcalls)
			{
				const auto draw_pipeline = pipeline.get_pipeline_instance(pipeline_cfg);
				draw_pipeline.bind(command_buffer, shadow_pass, level_data.get_vp_matrix());

				for (const auto& [drawcall, set_idx] : drawcalls)
				{
					draw_pipeline.set_material(
						command_buffer,
						shadow_pass,
						level_data.material_sets[set_idx][drawcall.material_index]
					);
					draw_pipeline.draw(command_buffer, shadow_pass, drawcall);
				}
			}

			shadow_pass.end();
		}

		return {};
	}
}
