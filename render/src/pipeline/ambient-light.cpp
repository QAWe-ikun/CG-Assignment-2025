#include "render/pipeline/ambient-light.hpp"

#include "asset/shader/ambient-light.frag.hpp"
#include "gpu/sampler.hpp"
#include "graphics/util/fullscreen-pass.hpp"
#include "render/target/gbuffer.hpp"
#include "render/target/light.hpp"
#include "util/as-byte.hpp"

#include <SDL3/SDL_gpu.h>
#include <optional>

namespace render::pipeline
{
	std::expected<AmbientLight, util::Error> AmbientLight::create(SDL_GPUDevice* device) noexcept
	{
		const gpu::Sampler::CreateInfo sampler_nearest_create_info{
			.min_filter = gpu::Sampler::Filter::Nearest,
			.mag_filter = gpu::Sampler::Filter::Nearest,
			.mipmap_mode = gpu::Sampler::MipmapMode::Nearest,
			.max_lod = 0.0f
		};
		auto sampler_nearest = gpu::Sampler::create(device, sampler_nearest_create_info);
		if (!sampler_nearest) return sampler_nearest.error().forward("Create sampler failed");

		const gpu::Sampler::CreateInfo sampler_linear_create_info{
			.min_filter = gpu::Sampler::Filter::Linear,
			.mag_filter = gpu::Sampler::Filter::Linear,
			.mipmap_mode = gpu::Sampler::MipmapMode::Linear,
			.max_lod = 0.0f
		};
		auto sampler_linear = gpu::Sampler::create(device, sampler_linear_create_info);
		if (!sampler_linear) return sampler_linear.error().forward("Create sampler failed");

		auto shader = gpu::GraphicsShader::create(
			device,
			shader_asset::ambient_light_frag,
			gpu::GraphicsShader::Stage::Fragment,
			3,
			0,
			0,
			1
		);
		if (!shader) return shader.error().forward("Create environment light shader failed");

		auto FullscreenPass = graphics::FullscreenPass<false>::create(
			device,
			*shader,
			target::LightBuffer::light_buffer_format,
			"Ambient Light Pipeline",
			graphics::FullscreenBlendMode::Add,
			graphics::FullscreenStencilState{
				.depth_format = target::Gbuffer::depth_format.format,
				.enable_stencil_test = true,
				.compare_mask = 0xFF,
				.write_mask = 0x00,
				.compare_op = SDL_GPU_COMPAREOP_EQUAL,
				.reference = 0x01
			}
		);
		if (!FullscreenPass) return FullscreenPass.error().forward("Create fullscreen pass failed");

		return AmbientLight(
			std::move(*FullscreenPass),
			std::move(*sampler_nearest),
			std::move(*sampler_linear)
		);
	}

	void AmbientLight::render(
		const gpu::CommandBuffer& command_buffer,
		const gpu::RenderPass& render_pass,
		const target::Gbuffer& gbuffer,
		const target::AO& ao,
		const Param& param
	) const noexcept
	{
		command_buffer.push_uniform_to_fragment(0, util::as_bytes(param));

		const std::array bindings = {
			gbuffer.albedo_texture->bind_with_sampler(sampler_nearest),
			gbuffer.lighting_info_texture->bind_with_sampler(sampler_nearest),
			ao.halfres_ao_texture.current().bind_with_sampler(sampler_nearest)
		};

		command_buffer.push_debug_group("Ambient Light Pass");
		FullscreenPass.render_to_renderpass(render_pass, bindings, std::nullopt, std::nullopt);
		command_buffer.pop_debug_group();
	}
}