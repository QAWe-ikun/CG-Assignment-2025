#include "camera/projection/ortho.hpp"

#include <glm/gtc/matrix_transform.hpp>

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#error "GLM_FORCE_DEPTH_ZERO_TO_ONE must be defined"
#endif

namespace camera::projection
{
	Ortho::Ortho(float viewport_height, float near_plane, float far_plane) noexcept :
		viewport_height(viewport_height),
		near_plane(near_plane),
		far_plane(far_plane)
	{}

	glm::dmat4 Ortho::matrix(float aspect_ratio) noexcept
	{
		const auto viewport_width = viewport_height * aspect_ratio;
		return glm::ortho<double>(
			-viewport_width / 2,
			viewport_width / 2,
			-viewport_height / 2,
			viewport_height / 2,
			near_plane,
			far_plane
		);
	}

	Ortho_fixed::Ortho_fixed(glm::vec2 viewport_size, float near_plane, float far_plane) noexcept :
		viewport_size(viewport_size),
		near_plane(near_plane),
		far_plane(far_plane)
	{}

	glm::dmat4 Ortho_fixed::matrix(float aspect_ratio [[maybe_unused]]) noexcept
	{
		return glm::ortho<double>(
			-viewport_size.x / 2,
			viewport_size.x / 2,
			-viewport_size.y / 2,
			viewport_size.y / 2,
			near_plane,
			far_plane
		);
	}
}