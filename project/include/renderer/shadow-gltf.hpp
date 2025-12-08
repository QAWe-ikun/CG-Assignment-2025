#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "graphics/smallest-bound.hpp"
#include "pipeline/shadow-gltf.hpp"
#include "target/shadow.hpp"

#include <cstddef>
#include <glm/glm.hpp>
#include <limits>

namespace renderer
{
	///
	/// @brief Shadow renderer for GLTF drawcalls
	///
	class Shadow_gltf
	{
	  public:

		class Drawdata
		{
			struct Drawcall
			{
				gltf::Primitive_drawcall drawcall;
				size_t resource_set_index;
				float min_z;
			};

			struct Resource
			{
				gltf::Material_cache::Ref material_cache;
				std::shared_ptr<gltf::Deferred_skinning_resource> deferred_skinning_resource;
			};

			struct CSM_level_data
			{
				std::map<std::pair<gltf::Pipeline_mode, bool>, std::vector<Drawcall>> drawcalls;
				std::vector<Resource> resource_sets;

				graphics::Smallest_bound smallest_bound;
				std::array<glm::vec4, 4> frustum_planes;

				float near = std::numeric_limits<float>::max();
				float far = std::numeric_limits<float>::lowest();

				void append(const gltf::Drawdata& drawdata) noexcept;

				glm::mat4 get_vp_matrix() const noexcept;

				void sort() noexcept;
			};

			std::array<CSM_level_data, 3> csm_levels;

			friend Shadow_gltf;

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

			///
			/// @brief Append glTF drawdata
			///
			/// @param drawdata glTF drawdata
			///
			void append(const gltf::Drawdata& drawdata) noexcept;

			///
			/// @brief Compute view-projection matrix
			///
			/// @param level CSM level index
			/// @return View-projection matrix
			///
			glm::mat4 get_vp_matrix(size_t level) const noexcept;

			///
			/// @brief Sort drawcalls for optimal rendering
			///
			///
			void sort() noexcept;
		};

		///
		/// @brief Create a shadow renderer
		///
		static std::expected<Shadow_gltf, util::Error> create(SDL_GPUDevice* device) noexcept;

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

		pipeline::Shadow_gltf pipeline;

		Shadow_gltf(pipeline::Shadow_gltf pipeline) noexcept :
			pipeline(std::move(pipeline))
		{}

	  public:

		Shadow_gltf(const Shadow_gltf&) = delete;
		Shadow_gltf(Shadow_gltf&&) = default;
		Shadow_gltf& operator=(const Shadow_gltf&) = delete;
		Shadow_gltf& operator=(Shadow_gltf&&) = default;
	};
}