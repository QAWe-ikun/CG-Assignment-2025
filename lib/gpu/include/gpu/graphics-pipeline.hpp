#pragma once

#include <SDL3/SDL_gpu.h>
#include <expected>
#include <optional>
#include <span>

#include "resource-box.hpp"

namespace gpu
{
	///
	/// @brief graphics Shader Object
	///
	///
	class Graphic_shader : public Resource_box<SDL_GPUShader>
	{
	  public:

		Graphic_shader(const Graphic_shader&) = delete;
		Graphic_shader& operator=(const Graphic_shader&) = delete;
		Graphic_shader(Graphic_shader&&) noexcept = default;
		Graphic_shader& operator=(Graphic_shader&&) noexcept = default;
		~Graphic_shader() noexcept = default;

		enum class Stage
		{
			Vertex = SDL_GPU_SHADERSTAGE_VERTEX,
			Fragment = SDL_GPU_SHADERSTAGE_FRAGMENT
		};

		///
		/// @brief Creates a graphics shader
		///
		static std::expected<Graphic_shader, util::Error> create(
			SDL_GPUDevice* device,
			std::span<const std::byte> shader_data,
			Stage stage,
			uint32_t num_samplers,
			uint32_t num_storage_textures,
			uint32_t num_storage_buffers,
			uint32_t num_uniform_buffers
		) noexcept;

	  private:

		using Resource_box<SDL_GPUShader>::Resource_box;
	};

	///
	/// @brief GPU graphics Pipeline
	///
	///
	class Graphics_pipeline : public Resource_box<SDL_GPUGraphicsPipeline>
	{
	  public:

		Graphics_pipeline(const Graphics_pipeline&) = delete;
		Graphics_pipeline& operator=(const Graphics_pipeline&) = delete;
		Graphics_pipeline(Graphics_pipeline&&) noexcept = default;
		Graphics_pipeline& operator=(Graphics_pipeline&&) noexcept = default;
		~Graphics_pipeline() noexcept = default;

		///
		/// @brief Depth stencil state
		///
		///
		struct Depth_stencil_state
		{
			SDL_GPUTextureFormat format;
			SDL_GPUCompareOp compare_op;
			SDL_GPUStencilOpState back_stencil_state;
			SDL_GPUStencilOpState front_stencil_state;
			Uint8 compare_mask;
			Uint8 write_mask;
			bool enable_depth_test;
			bool enable_depth_write;
			bool enable_stencil_test;
		};

		///
		/// @brief Creates a graphics pipeline
		///
		/// @param device GPU device
		/// @param vertex_shader Vertex shader
		/// @param fragment_shader Fragment shader
		/// @param primitive_type Primitive type used for drawing
		/// @param multisample_count Multisample count used
		/// @param rasterizer_state Rasterizer state
		/// @param vertex_attributes Vertex attribute descriptions in the vertex shader
		/// @param vertex_buffer_descs Vertex buffer descriptions
		/// @param color_target_descs Render target descriptions
		/// @param depth_stencil_state (Optional) Depth stencil state
		/// @return Pipeline object on success, or error on failure
		///
		static std::expected<Graphics_pipeline, util::Error> create(
			SDL_GPUDevice* device,
			const Graphic_shader& vertex_shader,
			const Graphic_shader& fragment_shader,
			SDL_GPUPrimitiveType primitive_type,
			SDL_GPUSampleCount multisample_count,
			const SDL_GPURasterizerState& rasterizer_state,
			std::span<const SDL_GPUVertexAttribute> vertex_attributes,
			std::span<const SDL_GPUVertexBufferDescription> vertex_buffer_descs,
			std::span<const SDL_GPUColorTargetDescription> color_target_descs,
			const std::optional<Depth_stencil_state>& depth_stencil_state
		) noexcept;

	  private:

		using Resource_box<SDL_GPUGraphicsPipeline>::Resource_box;
	};
}