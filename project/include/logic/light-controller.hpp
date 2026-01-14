#pragma once

#include "gltf/light.hpp"
#include "gltf/model.hpp"
#include "render/drawdata/light.hpp"
#include "render/light-volume.hpp"

#include <expected>
#include <map>
#include <memory>
#include <string>

namespace logic
{
	struct LightSource
	{
		std::shared_ptr<const render::LightVolume> volume;
		gltf::Light light;
		uint32_t node_index;
	};

	struct LightGroup
	{
		std::string display_name;
		std::vector<LightSource> lights;
		std::vector<uint32_t> emission_nodes;
		bool enabled = true;
	};

	class LightController
	{
		std::map<std::string, logic::LightGroup> light_groups;

		LightController(std::map<std::string, logic::LightGroup> light_groups) :
			light_groups(std::move(light_groups))
		{}

	  public:

		static std::expected<LightController, util::Error> create(
			SDL_GPUDevice* device,
			const gltf::Model& model
		) noexcept;

		///
		/// @brief Light control UI
		///
		///
		void control_ui() noexcept;

		///
		/// @brief Get emission overrides for light groups
		/// @return List of node index and emission multiplier pairs
		///
		std::vector<std::pair<uint32_t, float>> get_emission_overrides() const noexcept;

		///
		/// @brief Get light drawdata from enabled light groups
		/// @param drawdata Main drawdata for node matrices
		/// @return List of light drawdata
		///
		std::vector<render::drawdata::Light> get_light_drawdata(
			const gltf::Drawdata& drawdata
		) const noexcept;

		///
		/// @brief Handle fire event by enabling all lights
		///
		///
		void handle_fire_event() noexcept;
	};
}