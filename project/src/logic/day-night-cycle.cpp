#include "logic/day-night-cycle.hpp"
#include "glm/common.hpp"
#include "glm/gtc/constants.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <numbers>

namespace logic
{
	static glm::vec3 temperature_to_linear_color(float temperature_kelvin)
	{
		const float temp = glm::clamp(temperature_kelvin, 1000.0f, 40000.0f) / 100.0f;

		float red = 0.0f;
		float green = 0.0f;
		float blue = 0.0f;

		if (temp <= 66.0f)
		{
			red = 255.0f;
		}
		else
		{
			red = 329.698727446f * std::pow(temp - 60.0f, -0.1332047592f);
		}

		if (temp <= 66.0f)
		{
			green = 99.4708025861f * std::log(temp) - 161.1195681661f;
		}
		else
		{
			green = 288.1221695283f * std::pow(temp - 60.0f, -0.0755148492f);
		}

		if (temp >= 66.0f)
		{
			blue = 255.0f;
		}
		else if (temp <= 19.0f)
		{
			blue = 0.0f;
		}
		else
		{
			blue = 138.5177312231f * std::log(temp - 10.0f) - 305.0447927307f;
		}

		const auto to_srgb01 = [](float v) {
			return glm::clamp(v, 0.0f, 255.0f) / 255.0f;
		};

		const auto srgb = glm::vec3(to_srgb01(red), to_srgb01(green), to_srgb01(blue));

		// Convert sRGB (gamma encoded) to linear-light RGB using the inverse sRGB transfer function
		const auto srgb_to_linear = [](float c) {
			if (c <= 0.04045f) return c / 12.92f;
			return std::pow((c + 0.055f) / 1.055f, 2.4f);
		};

		return {srgb_to_linear(srgb.r), srgb_to_linear(srgb.g), srgb_to_linear(srgb.b)};
	}

	void Day_night_cycle::update(float delta_time) noexcept
	{
		if (!enable_cycle) return;

		// 更新时间
		time_of_day += delta_time * time_speed / 3600.0f;  // 转换为小时
		if (time_of_day >= 24.0f) time_of_day -= 24.0f;
		if (time_of_day < 0.0f) time_of_day += 24.0f;
	}

	Day_night_cycle::Light_params Day_night_cycle::get_light_params() const noexcept
	{
		if (auto_light_control)
		{
			return calculate_light_params();
		}

		// 如果不是自动控制，返回默认值（由Logic手动控制）
		return Light_params{};
	}

	void Day_night_cycle::set_time_of_day(float time) noexcept
	{
		time_of_day = time;
		if (time_of_day >= 24.0f) time_of_day -= 24.0f;
		if (time_of_day < 0.0f) time_of_day += 24.0f;
	}

	Day_night_cycle::Light_params Day_night_cycle::calculate_light_params() const noexcept
	{
		// 基于时间计算太阳位置
		// 参考值: 中午12:00 -> 方位角160°, 高度角45°
		// 假设: 6:00 日出, 12:00 正午, 18:00 日落, 0:00 午夜

		// 白天太阳的最高高度角（参考值）
		const float max_pitch = glm::radians(45.0f);
		const float sunrise_azimuth = glm::radians(-90.0f);  // 东方

		Light_params params{};

		const auto spherical_coord = [](float pitch, float azimuth) {
			return glm::vec3(
				glm::cos(pitch) * glm::sin(azimuth),
				glm::sin(pitch),
				glm::cos(pitch) * glm::cos(azimuth)
			);
		};

		const auto to_sunrise_dir = spherical_coord(0, sunrise_azimuth);
		const auto to_noon_dir = spherical_coord(max_pitch, sunrise_azimuth - glm::radians(90.0f));
		const auto angle = (time_of_day - 6.0f) / 12.0f * std::numbers::pi_v<float>;  // 6:00-18:00 映射到 0-π
		const auto sun_dir = glm::normalize(glm::cos(angle) * to_sunrise_dir + glm::sin(angle) * to_noon_dir);

		const auto sun_pitch = glm::asin(sun_dir.y);
		const auto sun_horizonal_brightness_mult =
			glm::smoothstep(glm::radians(0.0f), glm::radians(15.0f), sun_pitch);
		const auto temperature = glm::mix(1000.0f, 5000.0f, sun_horizonal_brightness_mult);
		const auto sun_color = temperature_to_linear_color(temperature);

		params.intensity = sun_horizonal_brightness_mult;
		params.color = sun_color;

		const float daylight =
			glm::clamp(std::sin((time_of_day - 6.0f) / 24.0f * glm::two_pi<float>()), 0.0f, 1.0f);
		params.ambient_intensity = glm::mix(0.01f, 1.0f, daylight);

		params.pitch = sun_pitch;
		params.azimuth = glm::atan(sun_dir.x, sun_dir.z);

		return params;
	}

	void Day_night_cycle::control_ui() noexcept
	{
		ImGui::SeparatorText("昼夜循环");

		ImGui::Checkbox("启用昼夜循环", &enable_cycle);

		if (enable_cycle)
		{
			ImGui::Checkbox("自动控制光照", &auto_light_control);
			ImGui::SliderFloat("时间流速", &time_speed, 1.0f, 3600.0f, "%.1fx", ImGuiSliderFlags_Logarithmic);

			// 显示当前时间
			int hours = static_cast<int>(time_of_day);
			int minutes = static_cast<int>((time_of_day - hours) * 60.0f);
			ImGui::Text("当前时间: %02d:%02d", hours, minutes);

			// 手动调整时间
			ImGui::SliderFloat("##时间", &time_of_day, 0.0f, 24.0f, "");

			// 快速切换到特定时间
			if (ImGui::Button("日出(6:30)"))
			{
				time_of_day = 6.5f;
			}
			ImGui::SameLine();
			if (ImGui::Button("正午(12:00)"))
			{
				time_of_day = 12.0f;
			}
			ImGui::SameLine();
			if (ImGui::Button("日落(17:30)"))
			{
				time_of_day = 17.5f;
			}
			ImGui::SameLine();
			if (ImGui::Button("午夜(0:00)"))
			{
				time_of_day = 0.0f;
			}
		}
	}
}
