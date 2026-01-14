#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "graphics/smallest-bound.hpp"

namespace render::drawdata
{
	struct Shadow
	{
		struct Drawcall
		{
			gltf::PrimitiveDrawcall drawcall;
			size_t resource_set_index;
			float min_z;
		};

		struct Resource
		{
			gltf::MaterialCache::Ref material_cache;
			std::shared_ptr<gltf::DeferredSkinningResource> deferred_skinning_resource;
		};

		struct ShadowLevelData
		{
			std::map<std::pair<gltf::PipelineMode, bool>, std::vector<Drawcall>> drawcalls;
			std::vector<Resource> resource_sets;

			graphics::SmallestBound smallest_bound;
			std::array<glm::vec4, 4> frustum_planes;

			float near = std::numeric_limits<float>::max();
			float far = std::numeric_limits<float>::lowest();

			void append(const gltf::Drawdata& drawdata) noexcept;

			glm::mat4 get_vp_matrix() const noexcept;

			void sort() noexcept;
		};

		std::array<ShadowLevelData, 3> csm_levels;

		///
		/// @brief Create a drawdata for shadow rendering
		///
		/// @param camera_matrix Camera matrix
		/// @param light_direction Light direction
		/// @param min_z Minimum Z in view space
		/// @param linear_blend_ratio Linear blend ratio for CSM levels
		///
		Shadow(
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
}