#pragma once

#include <concepts>
#include <cstdint>
#include <glm/glm.hpp>
#include <meshoptimizer.h>
#include <utility>
#include <vector>

namespace gltf::detail::mesh
{
	template <typename T>
	concept Vertex_type = requires(T a, T b) {
		{ a.position } -> std::convertible_to<glm::vec3>;
		{ a == b } -> std::convertible_to<bool>;
	};

	///
	/// @brief Remap vertices to eliminate duplicates
	///
	/// @tparam T Type of vertices
	/// @param vertices List of vertices
	/// @return Pair of remapped vertex list and index list
	///
	template <Vertex_type T>
	std::pair<std::vector<T>, std::vector<uint32_t>> remap_vertices(const std::vector<T>& vertices) noexcept
	{
		std::vector<uint32_t> remap_table(vertices.size());
		const auto vertex_count = meshopt_generateVertexRemapCustom(
			remap_table.data(),
			nullptr,
			vertices.size(),
			&vertices[0].position.x,
			vertices.size(),
			sizeof(T),
			[&vertices](const uint32_t& a_idx, const uint32_t& b_idx) {
				const auto& a = vertices[a_idx];
				const auto& b = vertices[b_idx];
				return a == b;
			}
		);

		std::vector<T> remapped_vertices(vertex_count);
		std::vector<uint32_t> remapped_indices(vertices.size());

		meshopt_remapVertexBuffer(
			remapped_vertices.data(),
			vertices.data(),
			vertices.size(),
			sizeof(T),
			remap_table.data()
		);

		meshopt_remapIndexBuffer(remapped_indices.data(), nullptr, vertices.size(), remap_table.data());

		return {std::move(remapped_vertices), std::move(remapped_indices)};
	}

	///
	/// @brief Optimize a full primitive (with all vertex attributes)
	///
	/// @param vertices Input vertex list
	/// @return Pair of optimized vertex list and index list
	///
	template <Vertex_type T>
	std::pair<std::vector<T>, std::vector<uint32_t>> optimize_primitive(
		const std::vector<T>& vertices
	) noexcept
	{
		auto [remapped_vertices, remapped_indices] = remap_vertices(vertices);

		meshopt_optimizeVertexCache(
			remapped_indices.data(),
			remapped_indices.data(),
			remapped_indices.size(),
			remapped_vertices.size()
		);

		meshopt_optimizeOverdraw(
			remapped_indices.data(),
			remapped_indices.data(),
			remapped_indices.size(),
			&remapped_vertices[0].position.x,
			remapped_vertices.size(),
			sizeof(T),
			1.05f
		);

		return {std::move(remapped_vertices), std::move(remapped_indices)};
	}
}