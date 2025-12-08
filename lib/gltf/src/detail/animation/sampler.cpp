#include "gltf/detail/animation/sampler.hpp"

namespace gltf::detail::animation
{
	std::optional<Interpolation> parse_interpolation(const std::string& str) noexcept
	{
		if (str == "LINEAR") return Interpolation::Linear;
		if (str == "STEP") return Interpolation::Step;
		if (str == "CUBICSPLINE") return Interpolation::Cubic;
		return std::nullopt;
	}
}