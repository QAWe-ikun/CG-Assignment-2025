#include "gltf/mesh.hpp"
#include "gltf/detail/mesh/optimize.hpp"
#include "gltf/detail/mesh/raw-primitive-list.hpp"

#include "graphics/util/quick-create.hpp"
#include "util/as-byte.hpp"
#include <algorithm>

namespace gltf
{
	using namespace detail::mesh;

	std::expected<Primitive, util::Error> Primitive::from_tinygltf(
		const tinygltf::Model& model,
		const tinygltf::Primitive& primitive
	) noexcept
	{
		auto vertex_list_result = get_primitive_list(model, primitive);
		if (!vertex_list_result)
			return vertex_list_result.error().forward("Get primitive vertex list failed");

		auto [optimized_vertices, optimized_indices] = optimize_primitive(*vertex_list_result);
		auto [optimized_shadow_vertices, optimized_shadow_indices] =
			optimize_position_only_primitive(*vertex_list_result);

		auto position_min = std::ranges::fold_left(
			optimized_shadow_vertices | std::views::transform(&Shadow_vertex::position),
			glm::vec3(std::numeric_limits<float>::max()),
			[](const glm::vec3& a, const glm::vec3& b) { return glm::min(a, b); }
		);

		auto position_max = std::ranges::fold_left(
			optimized_shadow_vertices | std::views::transform(&Shadow_vertex::position),
			glm::vec3(std::numeric_limits<float>::lowest()),
			[](const glm::vec3& a, const glm::vec3& b) { return glm::max(a, b); }
		);

		/* Clamp Minimum Dimension */

		auto size = position_max - position_min;
		const auto min_dim = std::min({size.x, size.y, size.z});
		const auto max_dim = std::max({size.x, size.y, size.z});
		if (min_dim < 0.0001f * max_dim)
		{
			const auto center = (position_min + position_max) * 0.5f;
			const auto min_extent = 0.0005f * max_dim;
			position_min = glm::min(position_min, center - glm::vec3(min_extent));
			position_max = glm::max(position_max, center + glm::vec3(min_extent));
		}

		return Primitive{
			.vertices = std::move(optimized_vertices),
			.indices = std::move(optimized_indices),
			.shadow_vertices = std::move(optimized_shadow_vertices),
			.shadow_indices = std::move(optimized_shadow_indices),
			.material = primitive.material == -1 ? std::nullopt : std::optional<uint32_t>(primitive.material),
			.position_min = position_min,
			.position_max = position_max,
		};
	}

	std::expected<Primitive_gpu, util::Error> Primitive_gpu::from_primitive(
		SDL_GPUDevice* device,
		const Primitive& primitive
	) noexcept
	{
		auto vertex_buffer =
			graphics::create_buffer_from_data(device, {.vertex = true}, util::as_bytes(primitive.vertices));

		auto index_buffer =
			graphics::create_buffer_from_data(device, {.index = true}, util::as_bytes(primitive.indices));

		auto shadow_vertex_buffer = graphics::create_buffer_from_data(
			device,
			{.vertex = true},
			util::as_bytes(primitive.shadow_vertices)
		);

		auto shadow_index_buffer = graphics::create_buffer_from_data(
			device,
			{.index = true},
			util::as_bytes(primitive.shadow_indices)
		);

		if (!vertex_buffer) return vertex_buffer.error().forward("Create vertex buffer failed");
		if (!index_buffer) return index_buffer.error().forward("Create index buffer failed");
		if (!shadow_vertex_buffer)
			return shadow_vertex_buffer.error().forward("Create position vertex buffer failed");
		if (!shadow_index_buffer)
			return shadow_index_buffer.error().forward("Create position index buffer failed");

		return Primitive_gpu{
			.index_count = static_cast<uint32_t>(primitive.indices.size()),

			.vertex_buffer = std::move(*vertex_buffer),
			.index_buffer = std::move(*index_buffer),
			.shadow_vertex_buffer = std::move(*shadow_vertex_buffer),
			.shadow_index_buffer = std::move(*shadow_index_buffer),

			.material = primitive.material,
			.position_min = primitive.position_min,
			.position_max = primitive.position_max
		};
	}

	std::expected<Mesh, util::Error> Mesh::from_tinygltf(
		const tinygltf::Model& model,
		const tinygltf::Mesh& mesh
	) noexcept
	{
		std::vector<Primitive> primitives;
		primitives.reserve(mesh.primitives.size());

		for (const auto& primitive : mesh.primitives)
		{
			auto primitive_result = Primitive::from_tinygltf(model, primitive);
			if (!primitive_result) return primitive_result.error().forward("Create Primitive failed");

			primitives.emplace_back(std::move(*primitive_result));
		}

		return Mesh{.primitives = std::move(primitives)};
	}

	std::expected<Mesh_gpu, util::Error> Mesh_gpu::from_mesh(SDL_GPUDevice* device, const Mesh& mesh) noexcept
	{
		std::vector<Primitive_gpu> primitives;
		primitives.reserve(mesh.primitives.size());

		for (const auto& primitive : mesh.primitives)
		{
			auto primitive_result = Primitive_gpu::from_primitive(device, primitive);
			if (!primitive_result) return primitive_result.error().forward("Create Primitive_gpu failed");

			primitives.emplace_back(std::move(*primitive_result));
		}

		return Mesh_gpu{.primitives = std::move(primitives)};
	}
}