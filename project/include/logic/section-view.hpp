///
/// @file section-view.hpp
/// @brief Section view control for hiding roof and adjusting camera
///

#pragma once

#include "camera-control.hpp"

#include <glm/glm.hpp>
#include <optional>
#include <vector>

namespace logic
{
	///
	/// @brief Section view controller for displaying building cross-sections
	///
	class Section_view
	{
	  public:

		///
		/// @brief Display UI controls for section view
		///
		void control_ui() noexcept;

		///
		/// @brief Check if section view mode is enabled
		///
		/// @return True if section view is enabled
		///
		bool is_enabled() const noexcept { return enabled; }

		///
		/// @brief Check if hide nodes mode is enabled
		///
		/// @return True if hide nodes is enabled
		///
		bool is_hide_nodes_enabled() const noexcept { return hide_nodes_enabled; }

		///
		/// @brief Toggle hide nodes mode
		///
		/// @param model The GLTF model to find nodes in
		///

		///
		/// @brief Toggle section view mode
		///
		/// @param model The GLTF model to find nodes in
		/// @param camera Camera controller to adjust view
		/// @param light_azimuth Reference to light azimuth angle
		/// @param light_pitch Reference to light pitch angle
		///
		void toggle(
			Camera& camera,
			float& light_azimuth,
			float& light_pitch
		) noexcept;

		///
		/// @brief Get list of hidden node indices
		///
		/// @return Vector of node indices to hide
		///
		const std::vector<uint32_t>& get_hidden_nodes() const noexcept { return hidden_node_indices; }

	  private:

		struct Camera_state
		{
			glm::dvec3 position;
			double azimuth;
			double pitch;
		};

		struct Light_state
		{
			float azimuth;
			float pitch;
		};

		bool enabled = false;                        // 启用剖面模式
		bool hide_nodes_enabled = false;             // 启用隐藏节点模式
		std::optional<bool> saved_hide_nodes_state;  // 进入剖面图前的隐藏状态
		std::vector<uint32_t> hidden_node_indices;
		std::optional<Camera_state> saved_camera_state;
		std::optional<Light_state> saved_light_state;

		// Section view camera settings
		static constexpr double section_view_height = 16.0;
		static constexpr double section_view_azimuth_deg = 0.0;
		static constexpr double section_view_pitch_deg = -89.0;

		// Section view light settings
		static constexpr float section_light_azimuth_deg = 180.0f;
		static constexpr float section_light_pitch_deg = 89.0f;
	};
}
