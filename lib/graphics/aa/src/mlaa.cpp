#include "graphics/aa/mlaa.hpp"

#include "asset/shader/mlaa-pass1.frag.hpp"
#include "asset/shader/mlaa-pass2.frag.hpp"
#include "asset/shader/mlaa-pass3.frag.hpp"

#include "graphics/aa/detail/mlaa-ortho-lut.hpp"

#include <array>

namespace graphics::aa
{
	static constexpr gpu::Sampler::Create_info sampler_info = {
		.mipmap_mode = gpu::Sampler::Mipmap_mode::Nearest,
		.address_mode_u = gpu::Sampler::Address_mode::Clamp_to_edge,
		.address_mode_v = gpu::Sampler::Address_mode::Clamp_to_edge,
		.address_mode_w = gpu::Sampler::Address_mode::Clamp_to_edge
	};

	static constexpr auto edge_texture_format = gpu::Texture::Format{
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8_UNORM,
		.usage = {.sampler = true, .color_target = true}
	};

	static constexpr auto blend_texture_format = gpu::Texture::Format{
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = {.sampler = true, .color_target = true}
	};

	static constexpr size_t lut_size = 17;

	std::expected<MLAA, util::Error> MLAA::create(SDL_GPUDevice* device, SDL_GPUTextureFormat format) noexcept
	{
		auto blend_lut = generate_ortho_area_lut(device, lut_size);
		if (!blend_lut) return blend_lut.error().forward("Create Area LUT failed");

		auto sampler = gpu::Sampler::create(device, sampler_info);
		if (!sampler) return sampler.error().forward("Create sampler failed");

		auto shader1 = gpu::Graphics_shader::create(
			device,
			std::as_bytes(shader_asset::mlaa_pass1_frag),
			gpu::Graphics_shader::Stage::Fragment,
			1,
			0,
			0,
			0
		);
		auto pass1 = std::move(shader1).and_then([device](gpu::Graphics_shader shader) {
			return Fullscreen_pass<true>::create(device, shader, edge_texture_format, {});
		});

		auto shader2 = gpu::Graphics_shader::create(
			device,
			std::as_bytes(shader_asset::mlaa_pass2_frag),
			gpu::Graphics_shader::Stage::Fragment,
			2,
			0,
			0,
			0
		);
		auto pass2 = std::move(shader2).and_then([device](gpu::Graphics_shader shader) {
			return Fullscreen_pass<true>::create(device, shader, blend_texture_format, {});
		});

		auto shader3 = gpu::Graphics_shader::create(
			device,
			std::as_bytes(shader_asset::mlaa_pass3_frag),
			gpu::Graphics_shader::Stage::Fragment,
			2,
			0,
			0,
			0
		);
		auto pass3 = std::move(shader3).and_then([&](gpu::Graphics_shader shader) {
			return Fullscreen_pass<true>::create(
				device,
				shader,
				{.type = SDL_GPU_TEXTURETYPE_2D, .format = format, .usage = {.color_target = true}},
				{.clear_before_render = false}
			);
		});

		if (!pass1) return pass1.error().forward("Create MLAA Pass 1 failed");
		if (!pass2) return pass2.error().forward("Create MLAA Pass 2 failed");
		if (!pass3) return pass3.error().forward("Create MLAA Pass 3 failed");

		return MLAA(
			std::move(*sampler),
			std::move(*blend_lut),
			std::move(*pass1),
			std::move(*pass2),
			std::move(*pass3)
		);
	}

	MLAA::MLAA(
		gpu::Sampler sampler,
		gpu::Texture blend_lut,
		Fullscreen_pass<true> pass1,
		Fullscreen_pass<true> pass2,
		Fullscreen_pass<true> pass3
	) noexcept :
		sampler(std::move(sampler)),
		blend_lut(std::move(blend_lut)),
		pass1(std::move(pass1)),
		pass2(std::move(pass2)),
		pass3(std::move(pass3)),
		edge_texture(edge_texture_format, "MLAA Edge Texture"),
		blend_texture(blend_texture_format, "MLAA Blend Texture")
	{}

	std::expected<void, util::Error> MLAA::run_antialiasing(
		SDL_GPUDevice* device,
		const gpu::Command_buffer& command_buffer,
		SDL_GPUTexture* source,
		SDL_GPUTexture* target,
		glm::u32vec2 size
	) noexcept
	{
		if (const auto result = edge_texture.resize(device, size); !result)
			return result.error().forward("Resize texture buffer failed");

		if (const auto result = blend_texture.resize(device, size); !result)
			return result.error().forward("Resize texture buffer failed");

		const auto pass1_texture = std::to_array<SDL_GPUTextureSamplerBinding>({
			{.texture = source, .sampler = sampler}
		});

		const auto pass2_texture = std::to_array<SDL_GPUTextureSamplerBinding>({
			{.texture = *edge_texture, .sampler = sampler},
			{.texture = blend_lut,     .sampler = sampler}
		});

		const auto pass3_texture = std::to_array<SDL_GPUTextureSamplerBinding>({
			{.texture = source,         .sampler = sampler},
			{.texture = *blend_texture, .sampler = sampler}
		});

		if (const auto result =
				pass1.render(command_buffer, *edge_texture, pass1_texture, std::nullopt, std::nullopt);
			!result)
			return result.error().forward("Run MLAA Pass 1 failed");

		if (const auto result =
				pass2.render(command_buffer, *blend_texture, pass2_texture, std::nullopt, std::nullopt);
			!result)
			return result.error().forward("Run MLAA Pass 2 failed");

		if (const auto result =
				pass3.render(command_buffer, target, pass3_texture, std::nullopt, std::nullopt);
			!result)
			return result.error().forward("Run MLAA Pass 3 failed");

		return {};
	}
}  // namespace graphics::aa