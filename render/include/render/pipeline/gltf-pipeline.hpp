#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "gltf/skin.hpp"
#include "gpu/command-buffer.hpp"
#include "gpu/render-pass.hpp"

namespace render::pipeline
{
	///
	/// @brief Interface for Gbuffer glTF pipelines
	///
	///
	class PipelineGLTF
	{
	  public:

		virtual ~PipelineGLTF() = default;

		///
		/// @brief Bind the pipeline to the command buffer and render pass
		///
		/// @param command_buffer Command buffer
		/// @param render_pass Render pass
		/// @param camera_matrix Camera matrix
		///
		virtual void bind(
			const gpu::CommandBuffer& command_buffer,
			const gpu::RenderPass& render_pass,
			const glm::mat4& camera_matrix
		) const noexcept = 0;

		///
		/// @brief Set a material for the pipeline
		///
		/// @param command_buffer Command buffer
		/// @param render_pass Render pass
		/// @param material glTF Material
		///
		virtual void set_material(
			const gpu::CommandBuffer& command_buffer,
			const gpu::RenderPass& render_pass,
			const gltf::MaterialGPU& material
		) const noexcept = 0;

		///
		/// @brief Set a skinning resource for the pipeline
		///
		/// @param render_pass Render pass
		/// @param skinning_resource Skinning resource
		///
		virtual void set_skin(
			const gpu::RenderPass& render_pass,
			const gltf::DeferredSkinningResource& skinning_resource
		) const noexcept = 0;

		///
		/// @brief Draw the primitive drawcall
		///
		/// @param command_buffer Command buffer
		/// @param render_pass Render pass
		/// @param drawcall Primitive drawcall
		///
		virtual void draw(
			const gpu::CommandBuffer& command_buffer,
			const gpu::RenderPass& render_pass,
			const gltf::PrimitiveDrawcall& drawcall
		) const noexcept = 0;
	};
}