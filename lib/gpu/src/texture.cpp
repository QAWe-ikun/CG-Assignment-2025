#include "gpu/texture.hpp"
#include "gpu/util.hpp"

namespace gpu
{
	std::expected<Texture, util::Error> Texture::create(
		SDL_GPUDevice* device,
		const SDL_GPUTextureCreateInfo& create_info
	) noexcept
	{
		assert(device != nullptr);

		auto* const texture = SDL_CreateGPUTexture(device, &create_info);
		if (texture == nullptr) RETURN_SDL_ERROR;
		return Texture(device, texture);
	}

	Texture::Usage::operator SDL_GPUTextureUsageFlags(this Usage self) noexcept
	{
		return std::bit_cast<uint8_t>(self) & 0x7f;
	}

	SDL_GPUTextureCreateInfo Texture::Format::create(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		uint32_t mip_levels,
		SDL_GPUSampleCount sample_count
	) const noexcept
	{
		return SDL_GPUTextureCreateInfo{
			.type = type,
			.format = format,
			.usage = usage,
			.width = width,
			.height = height,
			.layer_count_or_depth = depth,
			.num_levels = mip_levels,
			.sample_count = sample_count,
			.props = 0
		};
	}

	bool Texture::Format::supported_on(SDL_GPUDevice* device) const noexcept
	{
		assert(device != nullptr);

		return SDL_GPUTextureSupportsFormat(device, format, type, usage);
	}

	void Texture::set_name(const char* name) const noexcept
	{
		assert(resource != nullptr);
		assert(device != nullptr);

		SDL_SetGPUTextureName(device, resource, name);
	}
}