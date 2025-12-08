#include "renderer/gbuffer.hpp"
#include "gltf/model.hpp"
#include "graphics/corner.hpp"
#include "graphics/culling.hpp"

#include <algorithm>
#include <ranges>

namespace renderer
{
	Gbuffer::Drawdata::Drawdata(const glm::mat4& camera_matrix, const glm::vec3& eye_position) noexcept :
		camera_matrix(camera_matrix),
		eye_position(eye_position)
	{
		frustum_planes = graphics::compute_frustum_planes(camera_matrix);

		glm::vec4 near_plane_pos_homo(0.0, 0.0, 1.0, 1.0);
		near_plane_pos_homo = glm::inverse(camera_matrix) * near_plane_pos_homo;
		eye_to_nearplane = glm::vec3(near_plane_pos_homo) / near_plane_pos_homo.w - eye_position;
		near_distance = glm::length(eye_to_nearplane);
		eye_to_nearplane = glm::normalize(eye_to_nearplane);
	}

	void Gbuffer::Drawdata::append(const gltf::Drawdata& drawdata) noexcept
	{
		material_sets.push_back(drawdata.material_cache);
		const auto current_material_set = material_sets.size() - 1;

		auto visible_drawcalls =
			drawdata.objects | std::views::filter([this](const gltf::Primitive_drawcall& drawcall) {
				return graphics::box_in_frustum(
					drawcall.world_position_min,
					drawcall.world_position_max,
					frustum_planes
				);
			});

		const auto point_in_range = [this](const glm::vec3& p) {
			const auto eye_to_p = p - eye_position;
			const auto eye_to_p_norm = eye_to_p / near_distance;
			return glm::dot(eye_to_p_norm, eye_to_nearplane) > 1;
		};

		const auto clip_to_world = [this](const glm::vec3& world) {
			const auto homo = camera_matrix * glm::vec4(world, 1.0f);
			return glm::vec3(homo) / homo.w;
		};

		for (const auto& drawcall : visible_drawcalls)
		{
			const auto& pipeline = drawdata.material_cache[drawcall.material_index].params.pipeline;
			auto& target = drawcalls[pipeline];

			min_z = std::ranges::fold_left(
				graphics::get_corner_points(drawcall.world_position_min, drawcall.world_position_max)
					| std::views::filter(point_in_range)
					| std::views::transform(clip_to_world),
				min_z,
				[](float min, glm::vec3 p) { return std::min(min, p.z); }
			);

			if (target.empty()) target.reserve(1024);
			target.emplace_back(Drawcall{.drawcall = drawcall, .material_set_index = current_material_set});
		}
	}

	float Gbuffer::Drawdata::get_min_z() const noexcept
	{
		return glm::clamp(min_z, 0.0f, 1.0f);
	}

	std::expected<Gbuffer, util::Error> Gbuffer::create(SDL_GPUDevice* device) noexcept
	{
		return pipeline::Gbuffer::create(device)
			.transform([](pipeline::Gbuffer&& gbuffer_pipeline) noexcept {
				return Gbuffer{std::move(gbuffer_pipeline)};
			})
			.transform_error(util::Error::forward_fn("Create gbuffer pipeline failed"));
	}

	void Gbuffer::render(
		const gpu::Command_buffer& command_buffer,
		const gpu::Render_pass& gbuffer_pass,
		const Drawdata& drawdata
	) const noexcept
	{
		for (const auto& [pipeline_cfg, drawcalls] : drawdata.drawcalls)
		{
			const auto draw_pipeline = pipeline.get_pipeline_instance(pipeline_cfg);
			draw_pipeline.bind(command_buffer, gbuffer_pass, drawdata.camera_matrix);

			for (const auto& [drawcall, set_idx] : drawcalls)
			{
				draw_pipeline.set_material(
					command_buffer,
					gbuffer_pass,
					drawdata.material_sets[set_idx][drawcall.material_index]
				);
				draw_pipeline.draw(command_buffer, gbuffer_pass, drawcall);
			}
		}
	}
}