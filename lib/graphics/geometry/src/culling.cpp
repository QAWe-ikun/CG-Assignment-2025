#include "graphics/culling.hpp"
#include "graphics/corner.hpp"

#include <algorithm>
#include <glm/common.hpp>

namespace graphics
{
	std::array<glm::vec4, 6> compute_frustum_planes(const glm::mat4& mat) noexcept
	{
		std::array<glm::vec4, 6> planes;

		// Left plane
		planes[0] = glm::vec4(
			mat[0][3] + mat[0][0],
			mat[1][3] + mat[1][0],
			mat[2][3] + mat[2][0],
			mat[3][3] + mat[3][0]
		);

		// Right plane
		planes[1] = glm::vec4(
			mat[0][3] - mat[0][0],
			mat[1][3] - mat[1][0],
			mat[2][3] - mat[2][0],
			mat[3][3] - mat[3][0]
		);

		// Bottom plane
		planes[2] = glm::vec4(
			mat[0][3] + mat[0][1],
			mat[1][3] + mat[1][1],
			mat[2][3] + mat[2][1],
			mat[3][3] + mat[3][1]
		);

		// Top plane
		planes[3] = glm::vec4(
			mat[0][3] - mat[0][1],
			mat[1][3] - mat[1][1],
			mat[2][3] - mat[2][1],
			mat[3][3] - mat[3][1]
		);

		// Near plane
		planes[4] = glm::vec4(
			mat[0][3] + mat[0][2],
			mat[1][3] + mat[1][2],
			mat[2][3] + mat[2][2],
			mat[3][3] + mat[3][2]
		);

		// Far plane
		planes[5] = glm::vec4(
			mat[0][3] - mat[0][2],
			mat[1][3] - mat[1][2],
			mat[2][3] - mat[2][2],
			mat[3][3] - mat[3][2]
		);

		// Normalize planes
		for (auto& plane : planes)
		{
			const float length = glm::length(glm::vec3(plane));
			plane /= length;
		}

		return planes;
	}

	std::pair<glm::vec3, glm::vec3> local_bound_to_world(
		const glm::vec3& local_min,
		const glm::vec3& local_max,
		const glm::mat4& world_matrix
	) noexcept
	{
		const std::array<glm::vec3, 8> corners =
			transform_corner_points(get_corner_points(local_min, local_max), world_matrix);

		auto world_min = glm::vec3(std::numeric_limits<float>::max());
		auto world_max = glm::vec3(std::numeric_limits<float>::lowest());
		for (const auto& corner : corners)
		{
			world_min = glm::min(world_min, corner);
			world_max = glm::max(world_max, corner);
		}

		return {world_min, world_max};
	}

	bool box_in_frustum(
		const glm::vec3& box_min,
		const glm::vec3& box_max,
		const std::array<glm::vec4, 6>& planes
	) noexcept
	{
		return std::ranges::all_of(planes, [&box_min, &box_max](const auto& plane) {
			const glm::vec3 normal = glm::vec3(plane);
			const glm::vec3 positive_vertex = glm::vec3(
				normal.x >= 0 ? box_max.x : box_min.x,
				normal.y >= 0 ? box_max.y : box_min.y,
				normal.z >= 0 ? box_max.z : box_min.z
			);
			return glm::dot(normal, positive_vertex) + plane.w >= 0;
		});
	}
}
