#include "gltf/detail/animation/interpolation.hpp"

namespace gltf::detail::animation
{
	template <>
	glm::quat interpolate_linear(const glm::quat& a, const glm::quat& b, float at, float bt, float t) noexcept
	{
		assert(at <= t && t <= bt);
		return glm::slerp(a, b, (t - at) / (bt - at));
	}
}