#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "gpu/command-buffer.hpp"
#include "gpu/graphics-pipeline.hpp"
#include "gpu/render-pass.hpp"
#include "render/drawdata/shadow.hpp"
#include "render/pipeline/gltf-pipeline.hpp"
#include "render/target/shadow.hpp"

namespace render::pipeline
{
	class ShadowGLTF
	{
		// (Pipeline Mode, Rigged) -> Pipeline Instance
		std::map<std::pair<gltf::PipelineMode, bool>, std::unique_ptr<PipelineGLTF>> pipelines;

		ShadowGLTF(
			std::map<std::pair<gltf::PipelineMode, bool>, std::unique_ptr<PipelineGLTF>> pipelines
		) noexcept :
			pipelines(std::move(pipelines))
		{}

		struct FragParam
		{
			float base_color_factor_a;
			float alpha_cutoff;
		};

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

		ShadowGLTF(const ShadowGLTF&) = delete;
		ShadowGLTF(ShadowGLTF&&) = default;
		ShadowGLTF& operator=(const ShadowGLTF&) = delete;
		ShadowGLTF& operator=(ShadowGLTF&&) = default;

		static std::expected<ShadowGLTF, util::Error> create(SDL_GPUDevice* device) noexcept;

		std::expected<void, util::Error> render(
			const gpu::CommandBuffer& command_buffer,
			const target::Shadow& shadow_target,
			const drawdata::Shadow& drawdata
		) const noexcept;
	};
}