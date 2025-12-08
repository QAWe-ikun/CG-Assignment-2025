#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "gpu/command-buffer.hpp"
#include "gpu/graphics-pipeline.hpp"
#include "gpu/render-pass.hpp"

namespace pipeline
{
	class Shadow
	{
		std::map<gltf::Material_params::Pipeline, gpu::Graphics_pipeline> pipelines;

		Shadow(std::map<gltf::Material_params::Pipeline, gpu::Graphics_pipeline> pipelines) noexcept :
			pipelines(std::move(pipelines))
		{}

		struct alignas(64) Frag_param
		{
			float alpha_cutoff;
		};

	  public:

		class Pipeline_instance
		{
			std::reference_wrapper<const gpu::Graphics_pipeline> pipeline;

			friend Shadow;

			Pipeline_instance(std::reference_wrapper<const gpu::Graphics_pipeline> pipeline) noexcept :
				pipeline(pipeline)
			{}

		  public:

			Pipeline_instance(const Pipeline_instance&) = default;
			Pipeline_instance(Pipeline_instance&&) = default;
			Pipeline_instance& operator=(const Pipeline_instance&) = default;
			Pipeline_instance& operator=(Pipeline_instance&&) = default;

			void bind(
				const gpu::Command_buffer& command_buffer,
				const gpu::Render_pass& render_pass,
				const glm::mat4& shadow_matrix
			) const noexcept;

			void set_material(
				const gpu::Command_buffer& command_buffer,
				const gpu::Render_pass& render_pass,
				const gltf::Material_gpu& material
			) const noexcept;

			void draw(
				const gpu::Command_buffer& command_buffer,
				const gpu::Render_pass& render_pass,
				const gltf::Primitive_drawcall& drawcall
			) const noexcept;
		};

		Shadow(const Shadow&) = delete;
		Shadow(Shadow&&) = default;
		Shadow& operator=(const Shadow&) = delete;
		Shadow& operator=(Shadow&&) = default;

		static std::expected<Shadow, util::Error> create(SDL_GPUDevice* device) noexcept;

		Pipeline_instance get_pipeline_instance(gltf::Material_params::Pipeline mode) const noexcept;
	};
}