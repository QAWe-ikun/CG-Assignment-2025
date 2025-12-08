#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "gpu/command-buffer.hpp"
#include "gpu/graphics-pipeline.hpp"
#include "gpu/render-pass.hpp"

#include <expected>

namespace pipeline
{
	class Gbuffer
	{
		std::map<gltf::Material_params::Pipeline, gpu::Graphics_pipeline> pipelines;

		struct alignas(64) Frag_param
		{
			alignas(16) glm::vec3 base_color_factor;
			alignas(16) glm::vec3 emissive_factor;
			alignas(4) float metallic_factor;
			alignas(4) float roughness_factor;
			alignas(4) float normal_scale;
			alignas(4) float alpha_cutoff;
			alignas(4) float occlusion_strength;

			Frag_param(const Frag_param&) = default;
			Frag_param(Frag_param&&) = default;
			Frag_param& operator=(const Frag_param&) = default;
			Frag_param& operator=(Frag_param&&) = default;

			Frag_param(const gltf::Material_params::Factor& factor) noexcept;
		};

		Gbuffer(std::map<gltf::Material_params::Pipeline, gpu::Graphics_pipeline> pipelines) noexcept :
			pipelines(std::move(pipelines))
		{}

	  public:

		class Pipeline_instance
		{
			std::reference_wrapper<const gpu::Graphics_pipeline> pipeline;
			gltf::Material_params::Pipeline mode;

			friend Gbuffer;

			Pipeline_instance(
				std::reference_wrapper<const gpu::Graphics_pipeline> pipeline,
				gltf::Material_params::Pipeline mode
			) noexcept :
				pipeline(pipeline),
				mode(mode)
			{}

		  public:

			Pipeline_instance(const Pipeline_instance&) = default;
			Pipeline_instance(Pipeline_instance&&) = default;
			Pipeline_instance& operator=(const Pipeline_instance&) = default;
			Pipeline_instance& operator=(Pipeline_instance&&) = default;

			void bind(
				const gpu::Command_buffer& command_buffer,
				const gpu::Render_pass& render_pass,
				const glm::mat4& camera_matrix
			) const noexcept;

			void set_material(
				const gpu::Command_buffer& command_buffer,
				const gpu::Render_pass& render_pass,
				const gltf::Material_gpu& material
			) const noexcept;

			void set_skin(/* TBD */) noexcept;

			void draw(
				const gpu::Command_buffer& command_buffer,
				const gpu::Render_pass& render_pass,
				const gltf::Primitive_drawcall& drawcall
			) const noexcept;
		};

		Gbuffer(const Gbuffer&) = delete;
		Gbuffer(Gbuffer&&) = default;
		Gbuffer& operator=(const Gbuffer&) = delete;
		Gbuffer& operator=(Gbuffer&&) = default;

		static std::expected<Gbuffer, util::Error> create(SDL_GPUDevice* device) noexcept;

		Pipeline_instance get_pipeline_instance(gltf::Material_params::Pipeline mode) const noexcept;
	};
}