#pragma once

#include "material.hpp"
#include "mesh.hpp"
#include "node.hpp"

#include <atomic>

namespace gltf
{
	struct Drawcall
	{
		glm::mat4 transform;  // Transform matrix, the space of the transform varies
		Primitive_draw primitive;
		std::optional<uint32_t> material_index;
	};

	struct Drawdata
	{
		std::vector<Drawcall> drawcalls;
		Material_cache::Ref material_cache;
	};

	class Model
	{
	  private:

		/* Resources */

		Material_list material_list;       // List of materials
		std::vector<Mesh_gpu> meshes;      // List of meshes
		std::vector<Node> nodes;           // List of nodes
		std::vector<uint32_t> root_nodes;  // List of root node indices

		/* Accelerating Structures */

		std::vector<uint32_t> node_topo_order;              // Topological order of node indices
		std::vector<std::optional<uint32_t>> node_parents;  // Parent index for each node
		std::vector<bool> renderable_nodes;  // Whether each node is renderable (children of root)

		size_t primitive_count;  // Total primitive count

		std::unique_ptr<Material_cache> material_bind_cache;

	  public:

		enum class Load_stage
		{
			Node,
			Mesh,
			Material,
			Animation,  // Not implemented, TODO
			Postprocess
		};

		struct Load_progress
		{
			Load_stage stage;
			std::optional<float> progress;
		};

		///
		/// @brief Load model from tinygltf model
		///
		/// @param tinygltf_model Tinygltf model
		/// @param sampler_config Sampler creation config
		/// @param image_config Image compression config
		/// @param progress Progress reference for loading progress (optional)
		/// @return Loaded Model or Error
		///
		static std::expected<Model, util::Error> load_model(
			SDL_GPUDevice* device,
			const tinygltf::Model& tinygltf_model,
			const Sampler_config& sampler_config,
			const Material_list::Image_config& image_config,
			const std::optional<std::reference_wrapper<std::atomic<Load_progress>>>& progress = std::nullopt
		) noexcept;

		///
		/// @brief Generate drawdata for the model
		/// @warning The life span of the returned drawdata is shorter than the life span of the model
		///
		/// @param model_transform Root model transform matrix
		/// @return Drawdata, where drawcall's matrix denotes `Model->World` transform
		///
		Drawdata generate_drawdata(const glm::mat4& model_transform) const noexcept;

	  private:

		// Compute parent indices for all nodes.
		void compute_node_parents() noexcept;

		// Compute topological order of nodes, must be called after `compute_node_parents()`.
		std::expected<void, util::Error> compute_topo_order() noexcept;

		// Compute which nodes are renderable (children of root nodes). No loop checks are performed and must
		// be called after `compute_topo_order()`.
		void compute_renderable_nodes() noexcept;

	  public:

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(Model&&) = default;
		Model& operator=(Model&&) = default;
		~Model() = default;

		Model(
			Material_list material_list,
			std::vector<Mesh_gpu> meshes,
			std::vector<Node> nodes,
			std::vector<uint32_t> root_nodes
		) noexcept;
	};

	///
	/// @brief Load tinygltf model from binary glTF data
	///
	/// @param model_data Binary glTF data
	/// @return tinygltf Model on success, or Error on failure
	///
	std::expected<tinygltf::Model, util::Error> load_tinygltf_model(
		const std::vector<std::byte>& model_data
	) noexcept;
}