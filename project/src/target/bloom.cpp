#include "target/bloom.hpp"
#include "gpu/texture.hpp"

#include <ranges>

namespace target
{
	std::expected<void, util::Error> Bloom::resize(SDL_GPUDevice* device, glm::u32vec2 size) noexcept
	{
		if (this->size == size && !downsample_chain.empty() && !upsample_chain.empty()) return {};

		downsample_chain.clear();
		upsample_chain.clear();

		auto downsample_size = size;
		auto upsample_size = size / 2u;

		for (const auto mip : std::views::iota(0u, downsample_mip_count))
		{
			const auto create_info = format.create(downsample_size.x, downsample_size.y, 1, 1);
			auto downsample_texture =
				gpu::Texture::create(device, create_info, std::format("Bloom downsample mip {}", mip));
			if (!downsample_texture)
				return downsample_texture.error().forward("Create bloom downsample texture failed");
			downsample_chain.emplace_back(std::move(*downsample_texture));

			downsample_size /= 2u;
		}

		for (const auto mip : std::views::iota(0u, upsample_mip_count))
		{
			const auto create_info = format.create(upsample_size.x, upsample_size.y, 1, 1);
			auto upsample_texture =
				gpu::Texture::create(device, create_info, std::format("Bloom upsample mip {}", mip));
			if (!upsample_texture)
				return upsample_texture.error().forward("Create bloom upsample texture failed");
			upsample_chain.emplace_back(std::move(*upsample_texture));

			upsample_size /= 2u;
		}

		this->size = size;

		return {};
	}

	const gpu::Texture& Bloom::get_downsample_chain(size_t mip) const noexcept
	{
		return downsample_chain[mip];
	}

	const gpu::Texture& Bloom::get_upsample_chain(size_t mip) const noexcept
	{
		return upsample_chain[mip];
	}
}