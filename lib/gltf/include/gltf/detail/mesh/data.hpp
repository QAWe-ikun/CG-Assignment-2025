#pragma once

#include "util/error.hpp"
#include <expected>
#include <glm/glm.hpp>
#include <optional>
#include <tiny_gltf.h>
#include <vector>

namespace gltf::detail::mesh
{
	// Get raw positions, indexed by original indices
	std::expected<std::vector<glm::vec3>, util::Error> get_raw_positions(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive
	) noexcept;

	// Get raw normals, indexed by original indices
	std::expected<std::optional<std::vector<glm::vec3>>, util::Error> get_raw_normals(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive
	) noexcept;

	// Get raw texcoords, indexed by original indices
	std::expected<std::vector<glm::vec2>, util::Error> get_raw_texcoords(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive,
		const std::string& texcoord_name
	) noexcept;

	// Get index data, if exists
	std::expected<std::optional<std::vector<uint32_t>>, util::Error> get_indices(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive
	) noexcept;

	///
	/// @brief Unpack position data from the primitive
	/// @details This function does the following:
	/// 1. Acqurie the raw POSITION data from the primitive
	/// 2. If index is present, unpack the POSITION data according to the index
	/// 3. Expand to triangle list if the primitive is in triangle fan or triangle strip mode
	///
	/// @param model Tinygltf model
	/// @param primitive Tinygltf primitive
	/// @param index Optional index data
	/// @return POSITION attribute data on success, or error on failure
	///
	std::expected<std::vector<glm::vec3>, util::Error> unpack_positions(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive,
		const std::optional<std::vector<uint32_t>>& index
	) noexcept;

	///
	/// @brief Unpack normal data from the primitive
	/// @details This function does the following:
	/// 1. Acquire the raw NORMAL data from the primitive if available, otherwise generate normals from
	/// POSITION data and return
	/// 2. If index is present, unpack the NORMAL data according to the index
	/// 3. Expand to triangle list if the primitive is in triangle fan or triangle strip mode
	///
	/// @param model Tinygltf model
	/// @param primitive Tinygltf primitive
	/// @param index Optional index data
	/// @param position_vertices POSITION attribute data, used for normal generation if necessary
	/// @return NORMAL attribute data on success, or error on failure
	///
	std::expected<std::vector<glm::vec3>, util::Error> unpack_normals(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive,
		const std::optional<std::vector<uint32_t>>& index,
		const std::vector<glm::vec3>& position_vertices
	) noexcept;

	///
	/// @brief Unpack texcoord data from the primitive
	/// @details This function does the following:
	/// 1. Acqurie the raw TEXCOORD data from the primitive
	/// 2. If index is present, unpack the TEXCOORD data according to the index
	/// 3. Expand to triangle list if the primitive is in triangle fan or triangle strip mode
	///
	/// @param model Tinygltf model
	/// @param primitive Tinygltf primitive
	/// @param index Optional index data
	/// @param texcoord_name Name of the TEXCOORD attribute (e.g. `"TEXCOORD_0"`)
	/// @return TEXCOORD attribute data on success, or error on failure
	///
	std::expected<std::vector<glm::vec2>, util::Error> unpack_texcoords(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive,
		const std::optional<std::vector<uint32_t>>& index,
		const std::string& texcoord_name
	) noexcept;

	///
	/// @brief Compute tangent data from **UNPACKED** position and texcoord0 data
	/// @details This function calculates the TANGENT data from **UNPACKED** POSITION and TEXCOORD_0 data
	///
	/// @param position_vertices POSITION attribute data, must be in triangle list form
	/// @param texcoord0_vertices TEXCOORD_0 attribute data, must be in triangle list form
	/// @return TANGENT attribute data, or error on failure
	///
	std::expected<std::vector<glm::vec3>, util::Error> compute_tangents(
		const std::vector<glm::vec3>& position_vertices,
		const std::vector<glm::vec2>& texcoord0_vertices
	) noexcept;
}