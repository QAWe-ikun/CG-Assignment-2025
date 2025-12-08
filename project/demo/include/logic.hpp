#pragma once

#include "logic/ambient-light.hpp"
#include "logic/camera-control.hpp"
#include "renderer/aa.hpp"

#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>

struct Draw_feedback
{
	size_t total_drawcall_count;
	size_t visible_drawcall_count;
	size_t triangle_count;
};

class Logic
{
  public:

	enum class Debug_mode
	{
		No_debug,
		Show_AO
	};

	struct Result
	{
		renderer::Antialias::Mode aa_mode;
		logic::Ambient_light ambient_lighting;
		logic::Camera::Matrices camera;

		glm::vec3 light_direction;
		glm::vec3 light_color;
		float turbidity;
		float sky_brightness_mult;

		Debug_mode debug_mode;

		float csm_linear_blend;
	};

  private:

	logic::Camera camera_control;

	/* Lighting Control */

	float light_azimuth = 0.0;
	float light_pitch = glm::radians(78.0);

	glm::vec3 light_color = {1.0, 1.0, 1.0};
	float light_intensity = 2.0;
	float turbidity = 2.0f;
	float sky_brightness_mult = 0.25f;

	float csm_linear_blend = 0.85f;

	logic::Ambient_light ambient_lighting;

	void light_control_ui() noexcept;

	/* Antialiasing */

	renderer::Antialias::Mode aa_mode = renderer::Antialias::Mode::MLAA;

	void antialias_control_ui() noexcept;

	/* Statistics */

	void statistic_display_ui() const noexcept;

	/* Debug */

	Debug_mode debug_mode = Debug_mode::No_debug;

	void debug_control_ui() noexcept;

  public:

	Result logic() noexcept;
};