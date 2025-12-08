#include "graphics/camera/view/lookat.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace graphics::camera::view
{
	static bool is_parallel(const glm::vec3& a, const glm::vec3& b) noexcept
	{
		const auto a_norm = glm::normalize(a);
		const auto b_norm = glm::normalize(b);
		return glm::abs(glm::dot(a_norm, b_norm)) > 0.999f;
	}

	glm::dmat4 Lookat::matrix() const noexcept
	{
		if (is_parallel(center - eye, up)) [[unlikely]]
		{
			if (is_parallel(up, glm::vec3(0.0f, 0.0f, 1.0f))) [[unlikely]]
				return glm::lookAt<double, glm::packed_highp>(eye, center, glm::vec3(1.0f, 0.0f, 0.0f));
			else
				return glm::lookAt<double, glm::packed_highp>(eye, center, glm::vec3(0.0f, 0.0f, 1.0f));
		}

		return glm::lookAt<double, glm::packed_highp>(eye, center, up);
	}

	glm::vec3 Lookat::eye_position() const noexcept
	{
		return eye;
	}
}