#include "gltf/detail/image/check.hpp"

namespace gltf::detail::image
{
	bool dim_power_of_2(uint32_t value) noexcept
	{
		return (value != 0) && ((value & (value - 1)) == 0);
	}

	bool image_power_of_2(glm::u32vec2 size) noexcept
	{
		return dim_power_of_2(size.x) && dim_power_of_2(size.y);
	}

	bool image_size_multiple_of_block(glm::u32vec2 size) noexcept
	{
		return (size.x % 4 == 0) && (size.y % 4 == 0);
	}
}