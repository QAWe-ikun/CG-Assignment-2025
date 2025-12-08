///
/// @file mesh.hpp
/// @brief Provides structs repesenting vertices, primitives and meshes, both on CPU and GPU sides.
///

#pragma once

#include "gpu/buffer.hpp"
#include "util/inline.hpp"
#include <glm/glm.hpp>
#include <optional>
#include <tiny_gltf.h>
#include <vector>

namespace gltf
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec2 texcoord;
	};

	struct Shadow_vertex
	{
		glm::vec3 position;
		glm::vec2 texcoord;
	};

	// Primitive Mesh Data
	struct Primitive
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		std::vector<Shadow_vertex> shadow_vertices;
		std::vector<uint32_t> shadow_indices;

		std::optional<uint32_t> material;
		glm::vec3 position_min, position_max;

		///
		/// @brief Create a `Primitive` from a `tinygltf::Primitive`, performing validation
		///
		/// @param model Tinygltf model
		/// @param primitive Tinygltf primitive
		/// @return Primitive on success, or error on failure
		///
		static std::expected<Primitive, util::Error> from_tinygltf(
			const tinygltf::Model& model,
			const tinygltf::Primitive& primitive
		) noexcept;
	};

	// Plain raw data for drawing a primitive
	struct Primitive_mesh_binding
	{
		SDL_GPUBufferBinding vertex_buffer_binding;
		SDL_GPUBufferBinding index_buffer_binding;
		SDL_GPUBufferBinding shadow_vertex_buffer_binding;
		SDL_GPUBufferBinding shadow_index_buffer_binding;
		uint32_t index_count;
	};

	// Primitive Mesh Data for GPU
	struct Primitive_gpu
	{
		uint32_t index_count;

		gpu::Buffer vertex_buffer;
		gpu::Buffer index_buffer;
		gpu::Buffer shadow_vertex_buffer;
		gpu::Buffer shadow_index_buffer;

		std::optional<uint32_t> material;
		glm::vec3 position_min, position_max;

		///
		/// @brief Create a `Primitive_gpu` from a `Primitive`, uploading data to the GPU
		///
		/// @param primitive CPU-side primitive
		/// @return GPU-side primitive, or error on failure
		///
		static std::expected<Primitive_gpu, util::Error> from_primitive(
			SDL_GPUDevice* device,
			const Primitive& primitive
		) noexcept;

		///
		/// @brief Generate drawdata for this primitive
		/// @return (Primitive_draw, local_position_min, local_position_max)
		///
		FORCE_INLINE std::tuple<Primitive_mesh_binding, glm::vec3, glm::vec3> gen_drawdata() const noexcept
		{
			return {
				Primitive_mesh_binding{
									   .vertex_buffer_binding = {.buffer = vertex_buffer, .offset = 0},
									   .index_buffer_binding = {.buffer = index_buffer, .offset = 0},
									   .shadow_vertex_buffer_binding = {.buffer = shadow_vertex_buffer, .offset = 0},
									   .shadow_index_buffer_binding = {.buffer = shadow_index_buffer, .offset = 0},
									   .index_count = index_count,
									   },
				position_min,
				position_max
			};
		}
	};

	// Mesh data on CPU side
	struct Mesh
	{
		std::vector<Primitive> primitives;

		///
		/// @brief Parse a `tinygltf::Mesh` into a `Mesh`
		///
		/// @param model Tinygltf model
		/// @param mesh Tinygltf mesh
		/// @return Mesh on success, or error on failure
		///
		static std::expected<Mesh, util::Error> from_tinygltf(
			const tinygltf::Model& model,
			const tinygltf::Mesh& mesh
		) noexcept;
	};

	// Mesh data on GPU side
	struct Mesh_gpu
	{
		std::vector<Primitive_gpu> primitives;

		///
		/// @brief Upload a `Mesh` to GPU, creating `Mesh_gpu`
		///
		/// @param mesh CPU-side mesh
		/// @return GPU-side mesh, or error on failure
		///
		static std::expected<Mesh_gpu, util::Error> from_mesh(
			SDL_GPUDevice* device,
			const Mesh& mesh
		) noexcept;
	};
}