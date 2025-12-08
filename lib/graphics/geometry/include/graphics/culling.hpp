#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <span>
#include <utility>

namespace graphics
{
	///
	/// @brief Compute frustum planes from VP matrix
	/// @note For best performance, calculate the planes once per frame and reuse them for culling multiple
	/// objects
	///
	/// @param mat Input VP matrix
	/// @return Frustum planes in the order: left, right, bottom, top, near, far
	///
	std::array<glm::vec4, 6> compute_frustum_planes(const glm::mat4& mat) noexcept;

	///
	/// @brief Conservatively transform local AABB to world AABB
	///
	/// @param local_min Local space AABB minimum
	/// @param local_max Local space AABB maximum
	/// @param world_matrix Model transform matrix (Local to World)
	/// @return World space AABB (min, max)
	///
	std::pair<glm::vec3, glm::vec3> local_bound_to_world(
		const glm::vec3& local_min,
		const glm::vec3& local_max,
		const glm::mat4& world_matrix
	) noexcept;

	///
	/// @brief Tell if the world-space AABB is inside the frustum defined by the planes
	///
	/// @param box_min World space AABB minimum
	/// @param box_max World space AABB maximum
	/// @param planes Frustum planes, computed by `compute_frustum_planes()`. Can be a subset of planes.
	/// @return True if the box is inside the frustum, false otherwise
	///
	bool box_in_frustum(
		const glm::vec3& box_min,
		const glm::vec3& box_max,
		std::span<const glm::vec4> planes
	) noexcept;
}