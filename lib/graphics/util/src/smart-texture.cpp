#include "graphics/util/smart-texture.hpp"
#include <glm/fwd.hpp>

namespace graphics
{
	Auto_texture::Auto_texture(gpu::Texture::Format format) noexcept :
		format(format)
	{}

	glm::u32vec2 Auto_texture::get_size() const noexcept
	{
		return size;
	}

	std::expected<void, util::Error> Auto_texture::resize(
		SDL_GPUDevice* device,
		glm::u32vec2 new_size
	) noexcept
	{
		if (texture != nullptr && size == new_size) return {};

		if (new_size.x == 0 || new_size.y == 0) return util::Error("Invalid texture size");

		size = new_size;
		auto create_texture_result = gpu::Texture::create(device, format.create(size.x, size.y, 1, 1));
		if (!create_texture_result) return create_texture_result.error().forward("Resize failed");

		texture = std::make_unique<gpu::Texture>(std::move(create_texture_result.value()));
		return {};
	}

	SDL_GPUTexture* Auto_texture::operator*() const noexcept
	{
		assert(texture != nullptr && "Texture not initialized. Call resize() first.");
		return *texture;
	}

	Cycle_texture::Cycle_texture(gpu::Texture::Format format, size_t extra_pool_size) noexcept :
		format(format),
		extra_pool_size(std::max(1zu, extra_pool_size))
	{}

	std::expected<void, util::Error> Cycle_texture::resize_and_cycle(
		SDL_GPUDevice* device,
		glm::u32vec2 new_size
	) noexcept
	{
		if (new_size == size && !texture_pool.empty())
		{
			auto new_current = std::move(texture_pool.back());
			texture_pool.pop_back();
			texture_pool.insert(texture_pool.begin(), std::move(new_current));

			return {};
		}

		if (new_size.x == 0 || new_size.y == 0) return util::Error("Invalid texture size");

		texture_pool.clear();
		size = new_size;

		for (const auto _ : std::views::iota(0zu, extra_pool_size + 2))
		{
			auto create_texture_result = gpu::Texture::create(device, format.create(size.x, size.y, 1, 1));
			if (!create_texture_result)
				return create_texture_result.error().forward("Create new texture failed");

			texture_pool.emplace_back(std::move(create_texture_result.value()));
		}

		return {};
	}

	glm::u32vec2 Cycle_texture::get_size() const noexcept
	{
		return size;
	}

	SDL_GPUTexture* Cycle_texture::current() const noexcept
	{
		assert(!texture_pool.empty() && "Texture not initialized. Call resize_and_cycle() first.");
		return texture_pool[0];
	}

	SDL_GPUTexture* Cycle_texture::prev() const noexcept
	{
		assert(texture_pool.size() >= 2 && "Texture not initialized. Call resize_and_cycle() first.");
		return texture_pool[1];
	}
}