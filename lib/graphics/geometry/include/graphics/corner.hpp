#pragma once

#include <array>
#include <glm/glm.hpp>

#include "util/inline.hpp"

namespace graphics
{
	///
	/// @brief Get the 8 corner points of an AABB
	///
	/// @param box_min Minimum corner of the box
	/// @param box_max Maximum corner of the box
	/// @return Array of 8 corner points
	///
	FORCE_INLINE inline std::array<glm::vec3, 8> get_corner_points(
		const glm::vec3& box_min,
		const glm::vec3& box_max
	) noexcept
	{
		return {
			glm::vec3{box_min.x, box_min.y, box_min.z},
			glm::vec3{box_max.x, box_min.y, box_min.z},
			glm::vec3{box_min.x, box_max.y, box_min.z},
			glm::vec3{box_max.x, box_max.y, box_min.z},
			glm::vec3{box_min.x, box_min.y, box_max.z},
			glm::vec3{box_max.x, box_min.y, box_max.z},
			glm::vec3{box_min.x, box_max.y, box_max.z},
			glm::vec3{box_max.x, box_max.y, box_max.z},
		};
	}

	///
	/// @brief Apply a transformation matrix to corner points
	///
	/// @param corners Array of 8 corner points
	/// @param transform Transformation matrix
	/// @return Array of 8 transformed corner points
	///
	std::array<glm::vec3, 8> transform_corner_points(
		const std::array<glm::vec3, 8>& corners,
		const glm::mat4& transform
	) noexcept;
}