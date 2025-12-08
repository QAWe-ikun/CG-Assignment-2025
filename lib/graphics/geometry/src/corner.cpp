#include "graphics/corner.hpp"

#include <ranges>

namespace graphics
{
	std::array<glm::vec3, 8> get_corner_points(const glm::vec3& box_min, const glm::vec3& box_max) noexcept
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

	std::array<glm::vec3, 8> transform_corner_points(
		const std::array<glm::vec3, 8>& corners,
		const glm::mat4& transform
	) noexcept
	{
		std::array<glm::vec3, 8> transformed_corners;
		for (auto [src, dst] : std::views::zip(corners, transformed_corners))
		{
			const auto v = transform * glm::vec4(src, 1.0f);
			dst = glm::vec3(v / v.w);
		}
		return transformed_corners;
	}
}