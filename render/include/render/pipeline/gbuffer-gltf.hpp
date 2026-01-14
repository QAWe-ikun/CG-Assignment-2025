#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "gltf/skin.hpp"
#include "gpu/command-buffer.hpp"
#include "gpu/graphics-pipeline.hpp"
#include "gpu/render-pass.hpp"
#include "render/drawdata/gbuffer.hpp"
#include "render/pipeline/gltf-pipeline.hpp"

#include <expected>

namespace render::pipeline
{
	class GbufferGLTF
	{
		// (Pipeline Mode, Rigged) -> Pipeline Instance
		std::map<std::pair<gltf::PipelineMode, bool>, std::unique_ptr<PipelineGLTF>> pipelines;

		struct alignas(64) Frag_param
		{
			alignas(16) glm::vec4 base_color_factor;
			alignas(16) glm::vec3 emissive_factor;
			alignas(4) float metallic_factor;
			alignas(4) float roughness_factor;
			alignas(4) float normal_scale;
			alignas(4) float alpha_cutoff;
			alignas(4) float occlusion_strength;

			static Frag_param from(const gltf::MaterialParams::Factor& factor) noexcept;
		};

		struct PerObjectParam
		{
			alignas(4) float emissive_multiplier;

			static PerObjectParam from(const gltf::PrimitiveDrawcall& drawcall) noexcept;
		};

		GbufferGLTF(
			std::map<std::pair<gltf::PipelineMode, bool>, std::unique_ptr<PipelineGLTF>> pipelines
		) noexcept :
			pipelines(std::move(pipelines))
		{}

		class PipelineNormal : public PipelineGLTF
		{
			gltf::PipelineMode mode;
			gpu::GraphicsPipeline pipeline;

		  public:

			PipelineNormal(gltf::PipelineMode mode, gpu::GraphicsPipeline pipeline) :
				mode(mode),
				pipeline(std::move(pipeline))
			{}

			PipelineNormal(const PipelineNormal&) = delete;
			PipelineNormal(PipelineNormal&&) = default;
			PipelineNormal& operator=(const PipelineNormal&) = delete;
			PipelineNormal& operator=(PipelineNormal&&) = default;

			void bind(
				const gpu::CommandBuffer& command_buffer,
				const gpu::RenderPass& render_pass,
				const glm::mat4& camera_matrix
			) const noexcept override;

			void set_material(
				const gpu::CommandBuffer& command_buffer,
				const gpu::RenderPass& render_pass,
				const gltf::MaterialGPU& material
			) const noexcept override;

			void set_skin(
				const gpu::RenderPass& render_pass,
				const gltf::DeferredSkinningResource& skinning_resource
			) const noexcept override;

			void draw(
				const gpu::CommandBuffer& command_buffer,
				const gpu::RenderPass& render_pass,
				const gltf::PrimitiveDrawcall& drawcall
			) const noexcept override;
		};

		class PipelineRigged : public PipelineGLTF
		{
			gltf::PipelineMode mode;
			gpu::GraphicsPipeline pipeline;

		  public:

			PipelineRigged(gltf::PipelineMode mode, gpu::GraphicsPipeline pipeline) :
				mode(mode),
				pipeline(std::move(pipeline))
			{}

			PipelineRigged(const PipelineRigged&) = delete;
			PipelineRigged(PipelineRigged&&) = default;
			PipelineRigged& operator=(const PipelineRigged&) = delete;
			PipelineRigged& operator=(PipelineRigged&&) = default;

			void bind(
				const gpu::CommandBuffer& command_buffer,
				const gpu::RenderPass& render_pass,
				const glm::mat4& camera_matrix
			) const noexcept override;

			void set_material(
				const gpu::CommandBuffer& command_buffer,
				const gpu::RenderPass& render_pass,
				const gltf::MaterialGPU& material
			) const noexcept override;

			void set_skin(
				const gpu::RenderPass& render_pass,
				const gltf::DeferredSkinningResource& skinning_resource
			) const noexcept override;

			void draw(
				const gpu::CommandBuffer& command_buffer,
				const gpu::RenderPass& render_pass,
				const gltf::PrimitiveDrawcall& drawcall
			) const noexcept override;
		};

	  public:

		GbufferGLTF(const GbufferGLTF&) = delete;
		GbufferGLTF(GbufferGLTF&&) = default;
		GbufferGLTF& operator=(const GbufferGLTF&) = delete;
		GbufferGLTF& operator=(GbufferGLTF&&) = default;

		static std::expected<GbufferGLTF, util::Error> create(SDL_GPUDevice* device) noexcept;

		void render(
			const gpu::CommandBuffer& command_buffer,
			const gpu::RenderPass& gbuffer_pass,
			const drawdata::Gbuffer& drawdata
		) const noexcept;
	};
}