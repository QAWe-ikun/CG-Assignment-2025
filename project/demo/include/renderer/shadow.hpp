#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "graphics/smallest-bound.hpp"
#include "pipeline/shadow.hpp"
#include "target/shadow.hpp"

#include <glm/glm.hpp>
#include <limits>

namespace renderer
{
	///
	/// @brief Shadow renderer
	///
	class Shadow
	{
	  public:

		class Drawdata
		{
			struct Drawcall
			{
				gltf::Primitive_drawcall drawcall;
				size_t material_set_index;
			};

			struct CSM_level_data
			{
				std::map<gltf::Material_params::Pipeline, std::vector<Drawcall>> drawcalls;
				std::vector<gltf::Material_cache::Ref> material_sets;

				graphics::Smallest_bound smallest_bound;
				std::array<glm::vec4, 6> frustum_planes;

				float near = std::numeric_limits<float>::max();
				float far = std::numeric_limits<float>::lowest();

				void append(const gltf::Drawdata& drawdata) noexcept;

				glm::mat4 get_vp_matrix() const noexcept;
			};

			std::array<CSM_level_data, 3> csm_levels;

			friend Shadow;

		  public:

			///
			/// @brief Create a drawdata for shadow rendering
			///
			/// @param camera_matrix Camera matrix
			/// @param light_direction Light direction
			/// @param min_z Minimum Z in view space
			/// @param linear_blend_ratio Linear blend ratio for CSM levels
			///
			Drawdata(
				const glm::mat4& camera_matrix,
				const glm::vec3& light_direction,
				float min_z,
				float linear_blend_ratio
			) noexcept;

			void append(const gltf::Drawdata& drawdata) noexcept;

			///
			/// @brief Compute view-projection matrix
			///
			/// @param level CSM level index
			/// @return View-projection matrix
			///
			glm::mat4 get_vp_matrix(size_t level) const noexcept;
		};

		///
		/// @brief Create a shadow renderer
		///
		static std::expected<Shadow, util::Error> create(SDL_GPUDevice* device) noexcept;

		///
		/// @brief Render the frame into the shadow target
		/// @note This function automatically begins and ends the render pass. However, the target still needs
		/// to be resized before use.
		///
		/// @param shadow_target Shadow target
		/// @param drawdata Drawdata to render
		///
		std::expected<void, util::Error> render(
			const gpu::Command_buffer& command_buffer,
			const target::Shadow& shadow_target,
			const Drawdata& drawdata
		) const noexcept;

	  private:

		pipeline::Shadow pipeline;

		Shadow(pipeline::Shadow pipeline) noexcept :
			pipeline(std::move(pipeline))
		{}

	  public:

		Shadow(const Shadow&) = delete;
		Shadow(Shadow&&) = default;
		Shadow& operator=(const Shadow&) = delete;
		Shadow& operator=(Shadow&&) = default;
	};
}