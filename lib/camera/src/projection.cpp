#include "camera/projection.hpp"

namespace camera
{
	glm::dmat4 Projection::matrix_reverse_z(float aspect_ratio) noexcept
	{
		constexpr glm::dmat4 reversez_mat = glm::dmat4(
			glm::dvec4(1, 0, 0, 0),
			glm::dvec4(0, 1, 0, 0),
			glm::dvec4(0, 0, -1, 0),
			glm::dvec4(0, 0, 1, 1)
		);

		return reversez_mat * matrix(aspect_ratio);
	}
}