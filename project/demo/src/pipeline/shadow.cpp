#include "pipeline/shadow.hpp"

#include "asset/shader/shadow-mask.frag.hpp"
#include "asset/shader/shadow-mask.vert.hpp"
#include "asset/shader/shadow.frag.hpp"
#include "asset/shader/shadow.vert.hpp"

#include "gltf/mesh.hpp"
#include "gltf/model.hpp"
#include "gpu/graphics-pipeline.hpp"
#include "target/shadow.hpp"
#include "util/as-byte.hpp"

#include <SDL3/SDL_gpu.h>
#include <ranges>
#include <span>

namespace pipeline
{
	namespace
	{
		const auto vertex_attributes = std::to_array<SDL_GPUVertexAttribute>({
			{.location = 0,
			 .buffer_slot = 0,
			 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			 .offset = offsetof(gltf::Shadow_vertex, position)},
		});

		const auto masked_vertex_attributes = std::to_array<SDL_GPUVertexAttribute>({
			{.location = 0,
			 .buffer_slot = 0,
			 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			 .offset = offsetof(gltf::Shadow_vertex, position)},
			{.location = 1,
			 .buffer_slot = 0,
			 .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			 .offset = offsetof(gltf::Shadow_vertex, texcoord)},
		});

		const auto vertex_buffer_descs = std::to_array<SDL_GPUVertexBufferDescription>({
			{.slot = 0,
			 .pitch = sizeof(gltf::Shadow_vertex),
			 .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			 .instance_step_rate = 0},
		});

		const auto depth_stencil_state = gpu::Graphics_pipeline::Depth_stencil_state{
			.format = target::Shadow::depth_format.format,
			.compare_op = SDL_GPU_COMPAREOP_GREATER,
			.back_stencil_state = {},
			.front_stencil_state = {},
			.compare_mask = 0xFF,
			.write_mask = 0xFF,
			.enable_depth_test = true,
			.enable_depth_write = true,
			.enable_stencil_test = false
		};

		struct Shaders
		{
			gpu::Graphic_shader vertex;
			gpu::Graphic_shader vertex_mask;
			gpu::Graphic_shader fragment;
			gpu::Graphic_shader fragment_mask;

			static std::expected<Shaders, util::Error> create(SDL_GPUDevice* device) noexcept;
		};

		std::expected<Shaders, util::Error> Shaders::create(SDL_GPUDevice* device) noexcept
		{
			auto vertex_shader = gpu::Graphic_shader::create(
				device,
				shader_asset::shadow_vert,
				gpu::Graphic_shader::Stage::Vertex,
				0,
				0,
				0,
				2
			);

			auto vertex_mask_shader = gpu::Graphic_shader::create(
				device,
				shader_asset::shadow_mask_vert,
				gpu::Graphic_shader::Stage::Vertex,
				0,
				0,
				0,
				2
			);

			auto fragment_shader = gpu::Graphic_shader::create(
				device,
				shader_asset::shadow_frag,
				gpu::Graphic_shader::Stage::Fragment,
				0,
				0,
				0,
				0
			);

			auto fragment_mask_shader = gpu::Graphic_shader::create(
				device,
				shader_asset::shadow_mask_frag,
				gpu::Graphic_shader::Stage::Fragment,
				1,
				0,
				0,
				1
			);

			if (!vertex_shader) return vertex_shader.error().forward("Create Shadow vertex shader failed");
			if (!vertex_mask_shader)
				return vertex_mask_shader.error().forward("Create Shadow Mask vertex shader failed");
			if (!fragment_shader)
				return fragment_shader.error().forward("Create Shadow fragment shader failed");
			if (!fragment_mask_shader)
				return fragment_mask_shader.error().forward("Create Shadow Mask fragment shader failed");

			return Shaders{
				.vertex = std::move(*vertex_shader),
				.vertex_mask = std::move(*vertex_mask_shader),
				.fragment = std::move(*fragment_shader),
				.fragment_mask = std::move(*fragment_mask_shader)
			};
		}

		std::expected<gpu::Graphics_pipeline, util::Error> create_pipeline(
			SDL_GPUDevice* device,
			const Shaders& shaders,
			gltf::Material_params::Pipeline mode
		) noexcept
		{
			SDL_GPURasterizerState rasterizer_state;
			rasterizer_state.cull_mode = mode.double_sided ? SDL_GPU_CULLMODE_NONE : SDL_GPU_CULLMODE_BACK;
			rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
			rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
			rasterizer_state.depth_bias_clamp = 0.00;
			rasterizer_state.depth_bias_constant_factor = 0;
			rasterizer_state.depth_bias_slope_factor = -2.0;
			rasterizer_state.enable_depth_bias = true;
			rasterizer_state.enable_depth_clip = true;

			const gpu::Graphic_shader& used_vertex_shader =
				(mode.alpha_mode == gltf::Alpha_mode::Mask) ? shaders.vertex_mask : shaders.vertex;
			const gpu::Graphic_shader& used_fragment_shader =
				(mode.alpha_mode == gltf::Alpha_mode::Mask) ? shaders.fragment_mask : shaders.fragment;
			std::span used_vertex_attributes = (mode.alpha_mode == gltf::Alpha_mode::Mask)
				? std::span<const SDL_GPUVertexAttribute, std::dynamic_extent>(masked_vertex_attributes)
				: std::span<const SDL_GPUVertexAttribute, std::dynamic_extent>(vertex_attributes);

			return gpu::Graphics_pipeline::create(
				device,
				used_vertex_shader,
				used_fragment_shader,
				SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
				SDL_GPU_SAMPLECOUNT_1,
				rasterizer_state,
				used_vertex_attributes,
				vertex_buffer_descs,
				{},
				depth_stencil_state
			);
		}
	}

	std::expected<Shadow, util::Error> Shadow::create(SDL_GPUDevice* device) noexcept
	{
		auto shaders = Shaders::create(device);
		if (!shaders) return shaders.error().forward("Create Shadow shaders failed");

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

			auto pipeline = create_pipeline(device, *shaders, pipeline_cfg);

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

		return Shadow(std::move(pipeline_result));
	}

	void Shadow::Pipeline_instance::bind(
		const gpu::Command_buffer& command_buffer,
		const gpu::Render_pass& render_pass,
		const glm::mat4& shadow_matrix
	) const noexcept
	{
		render_pass.bind_pipeline(pipeline);
		command_buffer.push_uniform_to_vertex(0, util::as_bytes(shadow_matrix));
	}

	void Shadow::Pipeline_instance::set_material(
		const gpu::Command_buffer& command_buffer,
		const gpu::Render_pass& render_pass,
		const gltf::Material_gpu& material
	) const noexcept
	{
		if (material.params.pipeline.alpha_mode == gltf::Alpha_mode::Mask)
		{
			const Frag_param frag_param{.alpha_cutoff = material.params.factor.alpha_cutoff};
			const std::array textures = {material.base_color};

			command_buffer.push_uniform_to_fragment(0, util::as_bytes(frag_param));
			render_pass.bind_fragment_samplers(0, textures);
		}
	}

	void Shadow::Pipeline_instance::draw(
		const gpu::Command_buffer& command_buffer,
		const gpu::Render_pass& render_pass,
		const gltf::Primitive_drawcall& drawcall
	) const noexcept
	{
		command_buffer.push_uniform_to_vertex(1, util::as_bytes(drawcall.world_transform));
		render_pass.bind_vertex_buffers(0, drawcall.primitive.shadow_vertex_buffer_binding);
		render_pass.bind_index_buffer(
			drawcall.primitive.shadow_index_buffer_binding,
			SDL_GPU_INDEXELEMENTSIZE_32BIT
		);
		render_pass.draw_indexed(drawcall.primitive.index_count, 0, 1, 0, 0);
	}

	Shadow::Pipeline_instance Shadow::get_pipeline_instance(
		gltf::Material_params::Pipeline mode
	) const noexcept
	{
		return Pipeline_instance{std::cref(pipelines.at(mode))};
	}

}