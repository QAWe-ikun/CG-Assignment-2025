#include "render/pipeline/ao.hpp"

#include "asset/shader/ao.frag.hpp"
#include "gpu/graphics-pipeline.hpp"
#include "util/as-byte.hpp"

#include <random>

namespace render::pipeline
{
	AO::UniformParams AO::UniformParams::from(const Params& params) noexcept
	{
		static thread_local std::mt19937 generator{std::random_device{}()};
		static thread_local std::uniform_real_distribution<float> distribution{0.0f, 1.0f};

		const float random_seed = distribution(generator);

		return {
			.camera_mat_inv = params.camera_mat_inv,
			.view_mat = params.view_mat,
			.proj_mat_inv = params.proj_mat_inv,
			.prev_camera_mat = params.prev_camera_mat,
			.random_seed = random_seed,
			.radius = params.radius,
			.blend_alpha = params.blend_alpha
		};
	}

	static std::expected<gpu::GraphicsShader, util::Error> create_ao_shader(SDL_GPUDevice* device) noexcept
	{
		return gpu::GraphicsShader::
			create(device, shader_asset::ao_frag, gpu::GraphicsShader::Stage::Fragment, 4, 0, 0, 1)
				.transform_error(util::Error::forward_fn("Create AO fragment shader failed"));
	}

	std::expected<AO, util::Error> AO::create(SDL_GPUDevice* device) noexcept
	{
		const auto shader = create_ao_shader(device);
		if (!shader) return shader.error();

		const gpu::Sampler::CreateInfo sampler_linear_create_info{
			.min_filter = gpu::Sampler::Filter::Linear,
			.mag_filter = gpu::Sampler::Filter::Linear,
			.mipmap_mode = gpu::Sampler::MipmapMode::Nearest,
			.max_lod = 0.0f
		};

		const gpu::Sampler::CreateInfo sampler_nearest_create_info{
			.min_filter = gpu::Sampler::Filter::Nearest,
			.mag_filter = gpu::Sampler::Filter::Nearest,
			.mipmap_mode = gpu::Sampler::MipmapMode::Nearest,
			.max_lod = 0.0f
		};

		auto sampler_linear = gpu::Sampler::create(device, sampler_linear_create_info);
		if (!sampler_linear) return sampler_linear.error().forward("Create tonemapping sampler failed");

		auto sampler_nearest = gpu::Sampler::create(device, sampler_nearest_create_info);
		if (!sampler_nearest) return sampler_nearest.error().forward("Create tonemapping sampler failed");

		auto FullscreenPass =
			graphics::FullscreenPass<false>::create(device, *shader, target::AO::ao_format, {});
		if (!FullscreenPass) return FullscreenPass.error().forward("Create AO fullscreen pass failed");

		return AO(std::move(*sampler_linear), std::move(*sampler_nearest), std::move(*FullscreenPass));
	}

	void AO::render(
		const gpu::CommandBuffer& command_buffer,
		const gpu::RenderPass& render_pass,
		const target::AO& ao_target,
		const target::Gbuffer& gbuffer,
		const Params& params
	) const noexcept
	{
		const std::array texture_bindings = {
			gbuffer.depth_value_texture.current().bind_with_sampler(sampler_linear),
			gbuffer.lighting_info_texture->bind_with_sampler(sampler_nearest),
			gbuffer.depth_value_texture.prev().bind_with_sampler(sampler_linear),
			ao_target.halfres_ao_texture.prev().bind_with_sampler(sampler_linear)
		};

		const auto uniform_params = UniformParams::from(params);
		command_buffer.push_uniform_to_fragment(0, util::as_bytes(uniform_params));

		command_buffer.push_debug_group("AO Pass");
		FullscreenPass.render_to_renderpass(render_pass, texture_bindings, {}, {});
		command_buffer.pop_debug_group();
	}
}