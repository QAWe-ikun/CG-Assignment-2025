#pragma once

#include "gltf/material.hpp"
#include "gltf/model.hpp"
#include "pipeline/gbuffer.hpp"
#include <glm/glm.hpp>

namespace renderer
{
	///
	/// @brief Gbuffer renderer
	/// @details #### Typical Usage
	/// ```cpp
	/// // Initialization
	/// auto gbuffer_renderer = renderer::Gbuffer::create(device);
	/// if(!gbuffer_renderer) handle_error();
	///
	/// // Frame Preparation
	/// renderer::Gbuffer::Drawdata drawdata(camera_matrix);
	/// drawdata.append(object1_drawdata);
	/// drawdata.append(object2_drawdata);
	///
	/// // Inside Render Pass
	/// gbuffer_renderer->render(command_buffer, gbuffer_pass, drawdata);
	/// ```
	///
	class Gbuffer
	{
	  public:

		///
		/// @brief Drawdata for Gbuffer rendering
		/// @details Contains culled drawcalls and camera info, use `add` to append glTF drawdata
		///
		class Drawdata
		{
			struct Drawcall
			{
				gltf::Primitive_drawcall drawcall;
				size_t material_set_index;
			};

			std::map<gltf::Material_params::Pipeline, std::vector<Drawcall>> drawcalls;
			std::vector<gltf::Material_cache::Ref> material_sets;

			glm::mat4 camera_matrix;
			glm::vec3 eye_position;
			glm::vec3 eye_to_nearplane;

			std::array<glm::vec4, 6> frustum_planes;
			float min_z = 1;
			float near_distance;

			friend Gbuffer;

		  public:

			///
			/// @brief Create drawdata with camera matrix
			///
			/// @param camera_matrix Camera matrix
			///
			Drawdata(const glm::mat4& camera_matrix, const glm::vec3& eye_position) noexcept;

			///
			/// @brief Add glTF drawdata
			///
			/// @param drawdata glTF drawdata
			///
			void append(const gltf::Drawdata& drawdata) noexcept;

			///
			/// @brief Get maximum z depth
			///
			/// @return Maximum z depth
			///
			float get_min_z() const noexcept;
		};

		///
		/// @brief Create a gbuffer render renderer
		///
		/// @return Created Gbuffer renderer, or error on failure
		///
		static std::expected<Gbuffer, util::Error> create(SDL_GPUDevice* device) noexcept;

		///
		/// @brief Render the drawdata into the gbuffer pass
		///
		/// @param command_buffer Target command buffer
		/// @param gbuffer_pass Target gbuffer render pass
		/// @param drawdata Drawdata to render
		///
		void render(
			const gpu::Command_buffer& command_buffer,
			const gpu::Render_pass& gbuffer_pass,
			const Drawdata& drawdata
		) const noexcept;

	  private:

		pipeline::Gbuffer pipeline;

		Gbuffer(pipeline::Gbuffer pipeline) noexcept :
			pipeline(std::move(pipeline))
		{}

	  public:

		Gbuffer(const Gbuffer&) = delete;
		Gbuffer(Gbuffer&&) = default;
		Gbuffer& operator=(const Gbuffer&) = delete;
		Gbuffer& operator=(Gbuffer&&) = default;
	};
}