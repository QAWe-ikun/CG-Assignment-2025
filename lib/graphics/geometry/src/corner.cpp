#include "graphics/corner.hpp"

#include <ranges>

namespace graphics
{

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