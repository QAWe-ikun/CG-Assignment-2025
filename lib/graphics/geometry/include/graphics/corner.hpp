#pragma once

#include <array>
#include <glm/glm.hpp>

namespace graphics
{
	///
	/// @brief Get the 8 corner points of an AABB
	///
	/// @param box_min Minimum corner of the box
	/// @param box_max Maximum corner of the box
	/// @return Array of 8 corner points
	///
	std::array<glm::vec3, 8> get_corner_points(const glm::vec3& box_min, const glm::vec3& box_max) noexcept;

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