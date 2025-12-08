#include "gltf/detail/mesh/raw-primitive-list.hpp"
#include "gltf/detail/mesh/data.hpp"

#include <ranges>

namespace gltf::detail::mesh
{
	std::expected<std::vector<Vertex>, util::Error> get_primitive_list(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive
	) noexcept
	{
		/* Check Primitive Type */

		if (primitive.mode != TINYGLTF_MODE_TRIANGLES
			&& primitive.mode != TINYGLTF_MODE_TRIANGLE_FAN
			&& primitive.mode != TINYGLTF_MODE_TRIANGLE_STRIP)
			return util::Error("Only triangle primitives are supported");

		/* Get Index Data */

		auto index_raw_result = get_indices(model, primitive);
		if (!index_raw_result) return index_raw_result.error().forward("Get index failed");
		const auto index = std::move(*index_raw_result);

		/* Get Position Data */

		auto position_result = unpack_positions(model, primitive, index);
		if (!position_result) return position_result.error().forward("Get POSITION failed");
		const auto position_vertices = std::move(*position_result);

		/* Get Normal Data */

		auto normal_result = unpack_normals(model, primitive, index, position_vertices);
		if (!normal_result) return normal_result.error().forward("Get NORMAL failed");
		const auto normal_vertices = std::move(*normal_result);

		/* Get Texcoord0 Data */

		auto texcoord0_vertices =
			unpack_texcoords(model, primitive, index, "TEXCOORD_0")
				.value_or(std::vector<glm::vec2>(position_vertices.size(), glm::vec2(0.0f, 0.0f)));

		/* Get Tangent data */

		auto tangent_result = compute_tangents(position_vertices, texcoord0_vertices);
		if (!tangent_result) return tangent_result.error().forward("Compute primitive TANGENT failed");
		const auto tangent_vertices = std::move(*tangent_result);

		/* Check Size */

		if (position_vertices.size() != normal_vertices.size()
			|| position_vertices.size() != texcoord0_vertices.size()
			|| position_vertices.size() != tangent_vertices.size())
			return util::Error("Primitive attribute vertex counts do not match");

		/* Assemble Primitive */

		return std::views::zip_transform(
				   [](const auto& position, const auto& normal, const auto& texcoord, const auto& tangent) {
					   return Vertex{
						   .position = position,
						   .normal = normal,
						   .tangent = tangent,
						   .texcoord = texcoord,
					   };
				   },
				   position_vertices,
				   normal_vertices,
				   texcoord0_vertices,
				   tangent_vertices
			   )
			| std::ranges::to<std::vector>();
	}

	std::expected<std::vector<Rigged_vertex>, util::Error> get_rigged_primitive_list(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive
	) noexcept
	{
		/* Check Primitive Type */

		if (primitive.mode != TINYGLTF_MODE_TRIANGLES
			&& primitive.mode != TINYGLTF_MODE_TRIANGLE_FAN
			&& primitive.mode != TINYGLTF_MODE_TRIANGLE_STRIP)
			return util::Error("Only triangle primitives are supported");

		/* Get Index Data */

		auto index_raw_result = get_indices(model, primitive);
		if (!index_raw_result) return index_raw_result.error().forward("Get index failed");
		const auto index = std::move(*index_raw_result);

		/* Get Position Data */

		auto position_result = unpack_positions(model, primitive, index);
		if (!position_result) return position_result.error().forward("Get POSITION failed");
		const auto position_vertices = std::move(*position_result);

		/* Get Normal Data */

		auto normal_result = unpack_normals(model, primitive, index, position_vertices);
		if (!normal_result) return normal_result.error().forward("Get NORMAL failed");
		const auto normal_vertices = std::move(*normal_result);

		/* Get Texcoord0 Data */

		auto texcoord0_vertices =
			unpack_texcoords(model, primitive, index, "TEXCOORD_0")
				.value_or(std::vector<glm::vec2>(position_vertices.size(), glm::vec2(0.0f, 0.0f)));

		/* Get Tangent data */

		auto tangent_result = compute_tangents(position_vertices, texcoord0_vertices);
		if (!tangent_result) return tangent_result.error().forward("Compute primitive TANGENT failed");
		const auto tangent_vertices = std::move(*tangent_result);

		/* Get Joint Indices Data */

		auto joint_indices_result = unpack_joint_indices(model, primitive, index);
		if (!joint_indices_result) return joint_indices_result.error().forward("Get JOINTS_0 failed");
		const auto joint_indices_vertices = std::move(*joint_indices_result);

		/* Get Joint Weights Data */

		auto joint_weights_result = unpack_joint_weights(model, primitive, index);
		if (!joint_weights_result) return joint_weights_result.error().forward("Get WEIGHTS_0 failed");
		const auto joint_weights_vertices = std::move(*joint_weights_result);

		/* Check Size */

		if (position_vertices.size() != normal_vertices.size()
			|| position_vertices.size() != texcoord0_vertices.size()
			|| position_vertices.size() != tangent_vertices.size()
			|| position_vertices.size() != joint_indices_vertices.size()
			|| position_vertices.size() != joint_weights_vertices.size())
			return util::Error("Primitive attribute vertex counts do not match");

		/* Assemble Primitive */

		return std::views::zip_transform(
				   [](const auto& position,
					  const auto& normal,
					  const auto& texcoord,
					  const auto& tangent,
					  const auto& joint_indices,
					  const auto& joint_weights) {
					   return Rigged_vertex{
						   .position = position,
						   .normal = normal,
						   .tangent = tangent,
						   .texcoord = texcoord,
						   .joint_indices = joint_indices,
						   .joint_weights = joint_weights,
					   };
				   },
				   position_vertices,
				   normal_vertices,
				   texcoord0_vertices,
				   tangent_vertices,
				   joint_indices_vertices,
				   joint_weights_vertices
			   )

			| std::ranges::to<std::vector>();
	}
}