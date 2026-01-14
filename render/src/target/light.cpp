#include "render/target/light.hpp"

namespace render::target
{
	std::expected<void, util::Error> LightBuffer::cycle(SDL_GPUDevice* device, glm::u32vec2 size) noexcept
	{
		if (auto result = light_texture.resize_and_cycle(device, size); !result)
			return result.error().forward("Resize light buffer texture failed");

		return {};
	}
}