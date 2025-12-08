#include "pipeline/gbuffer.hpp"
#include "asset/shader/gbuffer-mask.frag.hpp"
#include "asset/shader/gbuffer.frag.hpp"
#include "asset/shader/gbuffer.vert.hpp"
#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "gpu/graphics-pipeline.hpp"
#include "target/gbuffer.hpp"
#include "target/light.hpp"
#include "util/as-byte.hpp"

#include <SDL3/SDL_gpu.h>
#include <expected>
#include <ranges>

namespace pipeline
{
	namespace
	{
		const auto vertex_attributes = std::to_array<SDL_GPUVertexAttribute>({
			{.location = 0,
			 .buffer_slot = 0,
			 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			 .offset = offsetof(gltf::Vertex, position)},
			{.location = 1,
			 .buffer_slot = 0,
			 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			 .offset = offsetof(gltf::Vertex, normal)  },
			{.location = 2,
			 .buffer_slot = 0,
			 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			 .offset = offsetof(gltf::Vertex, tangent) },
			{.location = 3,
			 .buffer_slot = 0,
			 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			 .offset = offsetof(gltf::Vertex, texcoord)},
		});

		const auto vertex_buffer_descs = std::to_array<SDL_GPUVertexBufferDescription>({
			{.slot = 0,
			 .pitch = sizeof(gltf::Vertex),
			 .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			 .instance_step_rate = 0},
		});

		const SDL_GPUColorTargetBlendState albedo_color_blend_state = {
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.color_write_mask = SDL_GPU_COLORCOMPONENT_R
				| SDL_GPU_COLORCOMPONENT_G
				| SDL_GPU_COLORCOMPONENT_B
				| SDL_GPU_COLORCOMPONENT_A,
			.enable_blend = false,
			.enable_color_write_mask = true,
			.padding1 = 0,
			.padding2 = 0
		};

		const SDL_GPUColorTargetDescription albedo_color_target =
			{.format = target::Gbuffer::albedo_format.format, .blend_state = albedo_color_blend_state};

		const SDL_GPUColorTargetBlendState lighting_info_blend_state = {
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G,
			.enable_blend = false,
			.enable_color_write_mask = true,
			.padding1 = 0,
			.padding2 = 0
		};

		const SDL_GPUColorTargetDescription lighting_info_target = {
			.format = target::Gbuffer::lighting_info_format.format,
			.blend_state = lighting_info_blend_state
		};

		const SDL_GPUColorTargetBlendState light_buffer_blend_state = {
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.color_write_mask = SDL_GPU_COLORCOMPONENT_R
				| SDL_GPU_COLORCOMPONENT_G
				| SDL_GPU_COLORCOMPONENT_B
				| SDL_GPU_COLORCOMPONENT_A,
			.enable_blend = false,
			.enable_color_write_mask = true,
			.padding1 = 0,
			.padding2 = 0
		};

		const SDL_GPUColorTargetDescription light_buffer_target = {
			.format = target::Light_buffer::light_buffer_format.format,
			.blend_state = light_buffer_blend_state
		};

		const auto color_target_descs = std::to_array<SDL_GPUColorTargetDescription>(
			{albedo_color_target, lighting_info_target, light_buffer_target}
		);

		gpu::Graphics_pipeline::Depth_stencil_state get_depth_stencil_state(bool double_sided) noexcept
		{
			const auto write_stencil_state = SDL_GPUStencilOpState{
				.fail_op = SDL_GPU_STENCILOP_KEEP,
				.pass_op = SDL_GPU_STENCILOP_REPLACE,
				.depth_fail_op = SDL_GPU_STENCILOP_KEEP,
				.compare_op = SDL_GPU_COMPAREOP_ALWAYS
			};

			const auto no_write_stencil_state = SDL_GPUStencilOpState{
				.fail_op = SDL_GPU_STENCILOP_KEEP,
				.pass_op = SDL_GPU_STENCILOP_KEEP,
				.depth_fail_op = SDL_GPU_STENCILOP_KEEP,
				.compare_op = SDL_GPU_COMPAREOP_ALWAYS
			};

			const auto depth_stencil_state = gpu::Graphics_pipeline::Depth_stencil_state{
				.format = target::Gbuffer::depth_format.format,
				.compare_op = SDL_GPU_COMPAREOP_GREATER,
				.back_stencil_state = double_sided ? write_stencil_state : no_write_stencil_state,
				.front_stencil_state = write_stencil_state,
				.compare_mask = 0xFF,
				.write_mask = 0xFF,
				.enable_depth_test = true,
				.enable_depth_write = true,
				.enable_stencil_test = true
			};

			return depth_stencil_state;
		}

	}

	Gbuffer::Frag_param::Frag_param(const gltf::Material_params::Factor& factor) noexcept :
		base_color_factor(factor.base_color_mult),
		emissive_factor(factor.emissive_mult),
		metallic_factor(factor.metallic_mult),
		roughness_factor(factor.roughness_mult),
		normal_scale(factor.normal_scale),
		alpha_cutoff(factor.alpha_cutoff),
		occlusion_strength(factor.occlusion_strength)
	{}

	static std::expected<gpu::Graphic_shader, util::Error> create_vertex_shader(
		SDL_GPUDevice* device
	) noexcept
	{
		return gpu::Graphic_shader::create(
			device,
			shader_asset::gbuffer_vert,
			gpu::Graphic_shader::Stage::Vertex,
			0,
			0,
			0,
			2
		);
	}

	static std::expected<gpu::Graphic_shader, util::Error> create_fragment_shader(
		SDL_GPUDevice* device
	) noexcept
	{
		return gpu::Graphic_shader::create(
			device,
			shader_asset::gbuffer_frag,
			gpu::Graphic_shader::Stage::Fragment,
			5,
			0,
			0,
			1
		);
	}

	static std::expected<gpu::Graphic_shader, util::Error> create_fragment_mask_shader(
		SDL_GPUDevice* device
	) noexcept
	{
		return gpu::Graphic_shader::create(
			device,
			shader_asset::gbuffer_mask_frag,
			gpu::Graphic_shader::Stage::Fragment,
			5,
			0,
			0,
			1
		);
	}

	static std::expected<gpu::Graphics_pipeline, util::Error> create_pipeline(
		SDL_GPUDevice* device,
		const gpu::Graphic_shader& vertex,
		const gpu::Graphic_shader& fragment,
		const gpu::Graphic_shader& fragment_mask,
		gltf::Material_params::Pipeline mode
	) noexcept
	{
		SDL_GPURasterizerState rasterizer_state;
		rasterizer_state.cull_mode = mode.double_sided ? SDL_GPU_CULLMODE_NONE : SDL_GPU_CULLMODE_BACK;
		rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
		rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
		rasterizer_state.depth_bias_clamp = 0;
		rasterizer_state.depth_bias_constant_factor = 0;
		rasterizer_state.depth_bias_slope_factor = 0;
		rasterizer_state.enable_depth_bias = false;
		rasterizer_state.enable_depth_clip = true;

		const gpu::Graphic_shader& fragment_shader =
			(mode.alpha_mode == gltf::Alpha_mode::Mask) ? fragment_mask : fragment;

		return gpu::Graphics_pipeline::create(
			device,
			vertex,
			fragment_shader,
			SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			SDL_GPU_SAMPLECOUNT_1,
			rasterizer_state,
			vertex_attributes,
			vertex_buffer_descs,
			color_target_descs,
			get_depth_stencil_state(mode.double_sided)
		);
	}

	std::expected<Gbuffer, util::Error> Gbuffer::create(SDL_GPUDevice* device) noexcept
	{
		auto vertex_shader = create_vertex_shader(device);
		if (!vertex_shader) return vertex_shader.error().forward("Create vertex shader failed");

		auto fragment_shader = create_fragment_shader(device);
		if (!fragment_shader) return fragment_shader.error().forward("Create fragment shader failed");

		auto fragment_mask_shader = create_fragment_mask_shader(device);
		if (!fragment_mask_shader)
			return fragment_mask_shader.error().forward("Create fragment mask shader failed");

		std::map<gltf::Material_params::Pipeline, gpu::Graphics_pipeline> pipeline_result;

		for (const auto [alpha_mode, double_sided, rigged] : std::views::cartesian_product(
				 std::array{gltf::Alpha_mode::Opaque, gltf::Alpha_mode::Mask, gltf::Alpha_mode::Blend},
				 std::array{false, true},
				 std::array{false, true}
			 ))
		{
			const auto pipeline_cfg = gltf::Material_params::Pipeline{
				.alpha_mode = alpha_mode,
				.double_sided = double_sided,
				.rigged = rigged
			};

			auto pipeline = create_pipeline(
				device,
				*vertex_shader,
				*fragment_shader,
				*fragment_mask_shader,
				pipeline_cfg
			);

			if (!pipeline)
				return pipeline.error().forward(
					std::format(
						"Create graphics pipeline failed (alpha_mode: {}, double_sided: {}, rigged: {})",
						static_cast<int>(alpha_mode),
						double_sided,
						rigged
					)
				);

			pipeline_result.emplace(pipeline_cfg, std::move(*pipeline));
		}

		return Gbuffer(std::move(pipeline_result));
	}

	void Gbuffer::Pipeline_instance::bind(
		const gpu::Command_buffer& command_buffer [[maybe_unused]],
		const gpu::Render_pass& render_pass,
		const glm::mat4& camera_matrix
	) const noexcept
	{
		render_pass.bind_pipeline(pipeline);
		command_buffer.push_uniform_to_vertex(0, util::as_bytes(camera_matrix));
		render_pass.set_stencil_reference(0x01);
	}

	void Gbuffer::Pipeline_instance::set_material(
		const gpu::Command_buffer& command_buffer,
		const gpu::Render_pass& render_pass,
		const gltf::Material_gpu& material
	) const noexcept
	{
		const auto frag_param = Frag_param(material.params.factor);

		command_buffer.push_uniform_to_fragment(0, util::as_bytes(frag_param));
		render_pass.bind_fragment_samplers(
			0,
			material.base_color,
			material.normal,
			material.metallic_roughness,
			material.occlusion,
			material.emissive
		);
	}

	void Gbuffer::Pipeline_instance::draw(
		const gpu::Command_buffer& command_buffer,
		const gpu::Render_pass& render_pass,
		const gltf::Primitive_drawcall& drawcall
	) const noexcept
	{
		const auto transform = drawcall.world_transform;
		command_buffer.push_uniform_to_vertex(1, util::as_bytes(transform));

		render_pass.bind_vertex_buffers(0, drawcall.primitive.vertex_buffer_binding);
		render_pass
			.bind_index_buffer(drawcall.primitive.index_buffer_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
		render_pass.draw_indexed(drawcall.primitive.index_count, 0, 1, 0, 0);
	}

	Gbuffer::Pipeline_instance Gbuffer::get_pipeline_instance(
		gltf::Material_params::Pipeline mode
	) const noexcept
	{
		return {std::ref(pipelines.at(mode)), mode};
	}
}