#include "camera/projection/perspective.hpp"

#include <glm/gtc/matrix_transform.hpp>

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#error "GLM_FORCE_DEPTH_ZERO_TO_ONE must be defined"
#endif

namespace camera::projection
{
	Perspective::Perspective(float fov_y, float near_plane, std::optional<float> far_plane) noexcept :
		fov_y(fov_y),
		near_plane(near_plane),
		far_plane(far_plane)
	{}

	glm::dmat4 Perspective::matrix(float aspect_ratio) noexcept
	{
		if (far_plane.has_value())
			return glm::perspective<double>(fov_y, aspect_ratio, near_plane, far_plane.value());
		else
			return glm::infinitePerspective<double>(fov_y, aspect_ratio, near_plane);
	}
}