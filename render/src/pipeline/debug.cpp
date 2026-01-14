#include "render/pipeline/debug.hpp"
#include "asset/shader/debug-single.frag.hpp"
#include "util/as-byte.hpp"

#include <SDL3/SDL_gpu.h>

namespace render::pipeline
{
	std::expected<DebugPipeline, util::Error> DebugPipeline::create(
		SDL_GPUDevice* device,
		gpu::Texture::Format format
	) noexcept
	{
		auto shader = gpu::GraphicsShader::create(
			device,
			shader_asset::debug_single_frag,
			gpu::GraphicsShader::Stage::Fragment,
			1,
			0,
			0,
			1
		);
		if (!shader) return shader.error().forward("Create debug shader failed");

		auto FullscreenPass =
			graphics::FullscreenPass<false>::create(device, *shader, format, "Debug Pipeline");
		if (!FullscreenPass) return FullscreenPass.error().forward("Create fullscreen pass failed");

		const gpu::Sampler::CreateInfo sampler_create_info{
			.min_filter = gpu::Sampler::Filter::Nearest,
			.mag_filter = gpu::Sampler::Filter::Nearest,
			.mipmap_mode = gpu::Sampler::MipmapMode::Nearest,
			.max_lod = 0.0f
		};
		auto sampler = gpu::Sampler::create(device, sampler_create_info);
		if (!sampler) return sampler.error().forward("Create sampler failed");

		return DebugPipeline(std::move(*FullscreenPass), std::move(*sampler));
	}

	void DebugPipeline::render_channels(
		const gpu::CommandBuffer& command_buffer [[maybe_unused]],
		const gpu::RenderPass& render_pass,
		SDL_GPUTexture* input_texture,
		glm::u32vec2 size [[maybe_unused]],
		uint8_t channel_count
	) const noexcept
	{
		if (channel_count == 0 || channel_count > 4) return;

		const auto channel_count_u32 = static_cast<uint32_t>(channel_count);
		command_buffer.push_uniform_to_fragment(0, util::as_bytes(channel_count_u32));

		const auto bind = SDL_GPUTextureSamplerBinding{.texture = input_texture, .sampler = sampler};
		const std::array bindings = {bind};
		FullscreenPass.render_to_renderpass(render_pass, bindings, {}, {});
	}
}