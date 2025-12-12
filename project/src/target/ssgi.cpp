#include "target/ssgi.hpp"

namespace target
{
	std::expected<void, util::Error> SSGI::resize(SDL_GPUDevice* device, glm::u32vec2 size) noexcept
	{
		const auto half_size = (size + 1u) / 2u;
		if (const auto result = primary_trace_texture.resize(device, half_size); !result)
			return result.error().forward("Resize SSGI trace texture failed");

		return {};
	}
}