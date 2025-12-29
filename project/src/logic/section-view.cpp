#include "logic/section-view.hpp"

#include <imgui.h>

namespace logic
{
	void Section_view::control_ui() noexcept
	{
		ImGui::SeparatorText("剖面图");

		if (ImGui::Checkbox("启用剖面模式", &enabled))
		{
			// 状态已改变，触发切换（将在logic函数中处理）
		}

		if (enabled)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "剖面模式已启用");
			ImGui::Text("屋顶已隐藏，相机俯视");
		}
		else
		{
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "正常视图模式");
		}

		ImGui::Separator();

		if (ImGui::Checkbox("隐藏屋顶", &hide_nodes_enabled))
		{
			// 状态已改变，触发切换（将在logic函数中处理）
		}

		if (hide_nodes_enabled)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "屋顶已隐藏");
		}
	}

	void Section_view::toggle(
		Camera& camera,
		float& light_azimuth,
		float& light_pitch
	) noexcept
	{
		if (enabled)
		{
			// 保存当前隐藏节点状态
			if (!saved_hide_nodes_state.has_value())
			{
				saved_hide_nodes_state = hide_nodes_enabled;
				// 进入剖面图模式时自动开启隐藏
				hide_nodes_enabled = true;
			}

			// 保存当前相机状态
			if (!saved_camera_state.has_value())
			{
				const auto [pos, azimuth, pitch] = camera.get_camera_state();
				saved_camera_state = Camera_state{.position = pos, .azimuth = azimuth, .pitch = pitch};

				// 设置剖面图相机位置和角度
				const glm::dvec3 section_position = glm::dvec3(0.0, section_view_height, 0.0);
				const double section_azimuth = glm::radians(section_view_azimuth_deg);
				const double section_pitch = glm::radians(section_view_pitch_deg);
				camera.set_camera(section_position, section_azimuth, section_pitch);
			}

			// 保存当前光照状态
			if (!saved_light_state.has_value())
			{
				saved_light_state = Light_state{.azimuth = light_azimuth, .pitch = light_pitch};

				// 设置剖面图光照角度
				light_azimuth = glm::radians(section_light_azimuth_deg);
				light_pitch = glm::radians(section_light_pitch_deg);
			}
		}
		else
		{
			// 恢复之前的隐藏节点状态
			if (saved_hide_nodes_state.has_value())
			{
				hide_nodes_enabled = *saved_hide_nodes_state;
				saved_hide_nodes_state.reset();

			}

			// 恢复之前保存的相机状态
			if (saved_camera_state.has_value())
			{
				camera.set_camera(
					saved_camera_state->position,
					saved_camera_state->azimuth,
					saved_camera_state->pitch
				);
				saved_camera_state.reset();
			}

			// 恢复之前保存的光照状态
			if (saved_light_state.has_value())
			{
				light_azimuth = saved_light_state->azimuth;
				light_pitch = saved_light_state->pitch;
				saved_light_state.reset();
			}
		}
	}
}
