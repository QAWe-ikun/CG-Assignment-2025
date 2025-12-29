#include "logic/climate-viewer.hpp"
#include <algorithm>
#include <cmath>
#include <format>
#include <glm/glm.hpp>
#include <imgui.h>
#include <numbers>
#include <random>

namespace logic
{
	Climate_viewer::Climate_viewer()
	{
		// 初始化房间数据（按区域枚举顺序，匹配门窗映射）
		// Room_zone::Bedroom1 = 0 -> door1
		rooms.emplace_back(
			"卧室1",
			Room_zone::Bedroom1,
			22.0f,
			60.0f,
			25.0f,
			5.0f,
			glm::vec4(0.5f, 0.7f, 0.9f, 1.0f)
		);
		// Room_zone::Kitchen = 1 -> door2
		rooms.emplace_back(
			"厨房",
			Room_zone::Kitchen,
			26.0f,
			65.0f,
			45.0f,
			15.0f,
			glm::vec4(0.9f, 0.5f, 0.5f, 1.0f)
		);
		// Room_zone::Bathroom = 2 -> door3
		rooms.emplace_back(
			"卫生间",
			Room_zone::Bathroom,
			25.0f,
			75.0f,
			20.0f,
			12.0f,
			glm::vec4(0.5f, 0.9f, 0.9f, 1.0f)
		);
		// Room_zone::Bedroom2 = 3 -> door4
		rooms.emplace_back(
			"卧室2",
			Room_zone::Bedroom2,
			23.0f,
			58.0f,
			30.0f,
			6.0f,
			glm::vec4(0.7f, 0.5f, 0.9f, 1.0f)
		);
		// Room_zone::LivingRoom = 4 -> door5 + curtains
		rooms.emplace_back(
			"客厅",
			Room_zone::LivingRoom,
			24.5f,
			55.0f,
			35.0f,
			8.0f,
			glm::vec4(0.9f, 0.7f, 0.3f, 1.0f)
		);

		// 设置随机变化率
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(-0.5f, 0.5f);

		for (auto& room : rooms)
		{
			room.temp_variation = dist(gen);
			room.humidity_variation = dist(gen);
			room.pm25_variation = dist(gen) * 2.0f;
		}
	}

	void Climate_viewer::update_realtime_data(float delta_time) noexcept
	{
		if (!enable_realtime_update) return;

		update_timer += delta_time;
		alarm_timer += delta_time;

		if (update_timer >= update_interval)
		{
			update_timer = 0.0f;

			// 更新每个房间的PM2.5数据（模拟实时变化）
			for (auto& room : rooms)
			{
				room.pm25 += room.pm25_variation * delta_time * 0.5f;

				// 限制范围
				room.pm25 = std::clamp(room.pm25, 0.0f, 200.0f);
				room.co = std::clamp(room.co, 0.0f, 50.0f);

				// 随机改变PM2.5变化方向
				if (room.pm25 < 5.0f || room.pm25 > 150.0f)
				{
					room.pm25_variation = -room.pm25_variation;
				}
			}
		}

		// 检测火灾危险
		if (enable_fire_detection)
		{
			check_fire_hazard();
		}
	}

	glm::vec4 Climate_viewer::get_temperature_color(float temperature) const noexcept
	{
		// 温度颜色映射: 冷(深蓝) -> 适中(绿) -> 热(深红)
		float normalized = std::clamp((temperature - 15.0f) / 20.0f, 0.0f, 1.0f);

		if (normalized < 0.33f)
		{
			// 深蓝到浅蓝
			float t = normalized / 0.33f;
			return {0.0f, 0.3f + t * 0.5f, 0.8f + t * 0.2f, 1.0f};
		}
		else if (normalized < 0.67f)
		{
			// 浅蓝到黄绿
			float t = (normalized - 0.33f) / 0.34f;
			return {t * 0.5f, 0.8f, 1.0f - t * 0.5f, 1.0f};
		}
		else
		{
			// 黄绿到深红
			float t = (normalized - 0.67f) / 0.33f;
			return {0.5f + t * 0.5f, 0.8f - t * 0.7f, 0.5f - t * 0.5f, 1.0f};
		}
	}

	glm::vec4 Climate_viewer::get_humidity_color(float humidity) const noexcept
	{
		// 湿度颜色映射: 干燥(橙黄) -> 潮湿(深蓝)
		float normalized = std::clamp(humidity / 100.0f, 0.0f, 1.0f);
		return {1.0f - normalized * 0.7f, 0.7f - normalized * 0.2f, 0.3f + normalized * 0.7f, 1.0f};
	}

	glm::vec4 Climate_viewer::get_pm25_color(float pm25) const noexcept
	{
		// PM2.5颜色映射: 优(绿) -> 良(黄) -> 污染(红) -> 重度污染(紫)
		if (pm25 <= 35.0f)
		{
			float t = pm25 / 35.0f;
			return {0.2f + t * 0.6f, 0.9f - t * 0.2f, 0.2f, 1.0f};
		}
		else if (pm25 <= 75.0f)
		{
			float t = (pm25 - 35.0f) / 40.0f;
			return {0.8f + t * 0.2f, 0.7f - t * 0.2f, 0.2f - t * 0.2f, 1.0f};
		}
		else if (pm25 <= 150.0f)
		{
			float t = (pm25 - 75.0f) / 75.0f;
			return {1.0f - t * 0.2f, 0.5f - t * 0.5f, 0.0f, 1.0f};
		}
		else
		{
			float t = std::min((pm25 - 150.0f) / 100.0f, 1.0f);
			return {0.8f - t * 0.2f, 0.0f, 0.0f + t * 0.6f, 1.0f};
		}
	}

	glm::vec4 Climate_viewer::get_co_color(float co) const noexcept
	{
		// CO颜色映射: 低(绿) -> 中(黄) -> 高(红)
		float normalized = std::clamp(co / 50.0f, 0.0f, 1.0f);

		if (normalized < 0.5f)
		{
			float t = normalized * 2.0f;
			return {0.3f + t * 0.7f, 0.9f, 0.3f - t * 0.3f, 1.0f};
		}
		else
		{
			float t = (normalized - 0.5f) * 2.0f;
			return {1.0f, 0.9f - t * 0.9f, 0.0f, 1.0f};
		}
	}

	glm::vec4 Climate_viewer::get_gradient_color(float value, float min_val, float max_val) const noexcept
	{
		float normalized = (max_val - min_val) > 0.001f
			? std::clamp((value - min_val) / (max_val - min_val), 0.0f, 1.0f)
			: 0.5f;

		// 从冷色(蓝)到暖色(红)的渐变
		if (normalized < 0.5f)
		{
			float t = normalized * 2.0f;
			return {t, 0.5f + t * 0.5f, 1.0f - t * 0.5f, 1.0f};
		}
		else
		{
			float t = (normalized - 0.5f) * 2.0f;
			return {1.0f, 1.0f - t * 0.5f, 0.5f - t * 0.5f, 1.0f};
		}
	}

	void Climate_viewer::check_fire_hazard() noexcept
	{
		bool danger_detected = false;

		// 检查每个房间是否超过危险阈值
		for (auto& room : rooms)
		{
			room.in_danger = (room.pm25 >= pm25_danger_threshold) || (room.co >= co_danger_threshold);
			if (room.in_danger)
			{
				danger_detected = true;
			}
		}

		// 触发或取消报警
		if (danger_detected && !fire_alarm_active)
		{
			trigger_fire_alarm();
		}
		else if (!danger_detected && fire_alarm_active)
		{
			reset_fire_alarm();
		}
	}

	void Climate_viewer::trigger_fire_alarm() noexcept
	{
		fire_alarm_active = true;
		alarm_timer = 0.0f;
		open_all_doors_and_windows();
	}

	void Climate_viewer::reset_fire_alarm() noexcept
	{
		fire_alarm_active = false;
		alarm_timer = 0.0f;
	}

	void Climate_viewer::open_all_doors_and_windows() noexcept
	{
		// 打开所有房间的门和窗
		for (auto& room : rooms)
		{
			room.door_open = true;
			room.window_open = true;
			room.curtain_open = true;
		}
	}

	void Climate_viewer::draw_fire_alarm_panel() noexcept
	{
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.1f, 0.1f, 0.9f));
		if (ImGui::CollapsingHeader("安全防护系统", ImGuiTreeNodeFlags_OpenOnArrow))
		{

			// 危险阈值设置
			ImGui::Text("危险阈值设置:");
			ImGui::SliderFloat("PM2.5阈值 (μg/m³)", &pm25_danger_threshold, 50.0f, 300.0f, "%.0f");
			ImGui::SliderFloat("CO阈值 (ppm)", &co_danger_threshold, 10.0f, 50.0f, "%.0f");
			ImGui::Checkbox("启用火灾检测", &enable_fire_detection);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// 报警状态显示
			if (fire_alarm_active)
			{
				// 闪烁效果
				float blink = std::sin(alarm_timer * 8.0f) * 0.5f + 0.5f;
				ImVec4 alarm_color = ImVec4(1.0f, blink * 0.3f, blink * 0.3f, 1.0f);

				ImGui::PushStyleColor(ImGuiCol_Text, alarm_color);
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

				// 大号警告文字
				ImVec2 window_size = ImGui::GetWindowSize();
				std::string alarm_text = "[!!!] 火灾警报! 检测到危险区域! [!!!]";
				ImVec2 text_size = ImGui::CalcTextSize(alarm_text.c_str());
				ImGui::SetCursorPosX((window_size.x - text_size.x) * 0.5f);
				ImGui::Text("%s", alarm_text.c_str());

				ImGui::PopFont();
				ImGui::PopStyleColor();

				ImGui::Spacing();

				// 显示危险房间
				ImGui::Text("危险区域:");
				for (const auto& room : rooms)
				{
					if (room.in_danger)
					{
						ImGui::BulletText(
							"%s - PM2.5: %.1f, CO: %.1f",
							room.name.c_str(),
							room.pm25,
							room.co
						);
					}
				}

				ImGui::Spacing();
				ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "[OK] 已自动打开所有门窗");
			}
			else
			{
				ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "[OK] 系统正常，未检测到危险");
			}
		}
			ImGui::PopStyleColor();

	}

	void Climate_viewer::draw_room_card_header(Climate_data& room, size_t index) noexcept
	{
		ImGui::PushID(static_cast<int>(index));

		// 根据温度设置卡片背景色
		auto temp_color = get_temperature_color(room.temperature);
		ImVec4 card_color = ImVec4(temp_color.r * 0.25f, temp_color.g * 0.25f, temp_color.b * 0.25f, 0.9f);

		ImGui::PushStyleColor(ImGuiCol_Header, card_color);
		ImGui::PushStyleColor(
			ImGuiCol_HeaderHovered,
			ImVec4(card_color.x * 1.3f, card_color.y * 1.3f, card_color.z * 1.3f, card_color.w)
		);
		ImGui::PushStyleColor(
			ImGuiCol_HeaderActive,
			ImVec4(card_color.x * 1.6f, card_color.y * 1.6f, card_color.z * 1.6f, card_color.w)
		);

		// 使用TreeNode并保持展开状态，避免在拖动滑块时折叠
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (room.expanded)
		{
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		// 如果房间处于危险状态，显示警告标志
		std::string header =
			room.in_danger ? std::format("[!] {} [危险!]", room.name) : std::format("{}", room.name);

		// 使用稳定的ID，标题文本包含实时温度
		bool node_open = ImGui::TreeNodeEx(room.name.c_str(), flags, "%s", header.c_str());
		room.expanded = node_open;
	}

	void Climate_viewer::draw_room_card_controls(Climate_data& room) noexcept
	{
		// 显示/隐藏
		ImGui::Checkbox("显示", &room.visible);
		ImGui::SameLine();
		ImGui::ColorEdit4(
			"##color",
			&room.color.r,
			ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel
		);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}

	void Climate_viewer::draw_room_thermometers(Climate_data& room) noexcept
	{
		if (!show_temperature && !show_humidity) return;

		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
		float canvas_width = ImGui::GetContentRegionAvail().x;
		float thermo_width = 80.0f;
		float thermo_height = 200.0f;
		float canvas_height = thermo_height + 30.0f;

		// 背景
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(
			canvas_pos,
			ImVec2(canvas_pos.x + canvas_width, canvas_pos.y + canvas_height),
			IM_COL32(25, 25, 35, 255),
			5.0f
		);

		// 绘制温度计（左侧）
		if (show_temperature)
		{
			float temp_x = canvas_pos.x + canvas_width * 0.25f - thermo_width * 0.5f;
			float temp_y = canvas_pos.y + 15.0f;
			auto temp_col = get_temperature_color(room.temperature);
			draw_thermometer(
				room.temperature,
				15.0f,
				35.0f,
				20.0f,
				26.0f,
				"温度",
				"°C",
				temp_col,
				temp_x,
				temp_y,
				thermo_width,
				thermo_height
			);
		}

		// 绘制湿度计（右侧）
		if (show_humidity)
		{
			float hum_x = canvas_pos.x + canvas_width * 0.75f - thermo_width * 0.5f;
			float hum_y = canvas_pos.y + 15.0f;
			auto hum_col = get_humidity_color(room.humidity);
			draw_thermometer(
				room.humidity,
				30.0f,
				90.0f,
				40.0f,
				60.0f,
				"湿度",
				"%",
				hum_col,
				hum_x,
				hum_y,
				thermo_width,
				thermo_height
			);
		}

		ImGui::Dummy(ImVec2(canvas_width, canvas_height));
		ImGui::Spacing();
	}

	void Climate_viewer::draw_room_gauges(Climate_data& room) noexcept
	{
		if (!show_pm25 && !show_co) return;

		ImGui::Checkbox("PM2.5和CO", &room.show_room_gauges);
		ImGui::Spacing();

		if (!room.show_room_gauges) return;

		// 获取绘制区域
		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
		float canvas_width = ImGui::GetContentRegionAvail().x;
		float gauge_radius = std::min(canvas_width * 0.25f, 70.0f);
		float canvas_height = gauge_radius * 2.5f;

		// 背景
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(
			canvas_pos,
			ImVec2(canvas_pos.x + canvas_width, canvas_pos.y + canvas_height),
			IM_COL32(25, 25, 35, 255)
		);

		// 绘制PM2.5仪表盘（左侧）
		if (show_pm25)
		{
			ImVec2 pm25_pos = ImVec2(canvas_pos.x + canvas_width * 0.25f, canvas_pos.y + gauge_radius * 1.2f);
			auto pm25_color = get_pm25_color(room.pm25);
			draw_gauge(room.pm25, 200.0f, "PM2.5 (µg/m³)", pm25_color, pm25_pos.x, pm25_pos.y, gauge_radius);
		}

		// 绘制CO仪表盘（右侧）
		if (show_co)
		{
			ImVec2 co_pos = ImVec2(canvas_pos.x + canvas_width * 0.75f, canvas_pos.y + gauge_radius * 1.2f);
			auto co_color = get_co_color(room.co);
			draw_gauge(room.co, 50.0f, "CO (ppm)", co_color, co_pos.x, co_pos.y, gauge_radius);
		}

		ImGui::Dummy(ImVec2(canvas_width, canvas_height));
		ImGui::Spacing();
	}

	void Climate_viewer::draw_room_card(size_t index) noexcept
	{
		auto& room = rooms[index];

		draw_room_card_header(room, index);

		if (room.expanded)
		{
			ImGui::Indent(15.0f);
			draw_room_card_controls(room);
			draw_room_thermometers(room);
			draw_room_gauges(room);
			ImGui::Unindent(15.0f);
			ImGui::Spacing();
			ImGui::TreePop();
		}

		ImGui::PopStyleColor(3);
		ImGui::PopID();
	}

	void Climate_viewer::draw_heatmap() noexcept
	{
		if (rooms.empty()) return;

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
		ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, 320);

		// 背景（深色背景板）
		draw_list->AddRectFilled(
			canvas_pos,
			ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
			IM_COL32(15, 15, 20, 255),
			5.0f
		);

		// 计算可见房间数量
		int visible_count = 0;
		for (const auto& room : rooms)
		{
			if (room.visible) visible_count++;
		}

		if (visible_count == 0)
		{
			ImGui::Dummy(canvas_size);
			return;
		}

		// 布局参数
		float bar_height = 120.0f;
		float bar_spacing = 35.0f;
		float top_margin = 40.0f;
		float room_width = (canvas_size.x - 80.0f) / static_cast<float>(visible_count);

		// 温度和湿度的绝对范围
		const float temp_min = 15.0f, temp_max = 35.0f;
		const float hum_min = 30.0f, hum_max = 90.0f;

		// 绘制标题和图例
		draw_list->AddText(
			ImVec2(canvas_pos.x + 10, canvas_pos.y + 8),
			IM_COL32(220, 220, 230, 255),
			"房间环境热力分布图"
		);

		// 温度图例
		ImVec2 legend_pos = ImVec2(canvas_pos.x + canvas_size.x - 120, canvas_pos.y + 8);
		draw_list->AddText(legend_pos, IM_COL32(200, 200, 220, 255), "温度 15-35°C");
		for (int i = 0; i < 5; ++i)
		{
			float t = static_cast<float>(i) / 4.0f;
			float temp_val = temp_min + t * (temp_max - temp_min);
			auto color = get_temperature_color(temp_val);
			draw_list->AddRectFilled(
				ImVec2(legend_pos.x + i * 15, legend_pos.y + 18),
				ImVec2(legend_pos.x + (i + 1) * 15 - 1, legend_pos.y + 30),
				IM_COL32(
					static_cast<int>(color.r * 255),
					static_cast<int>(color.g * 255),
					static_cast<int>(color.b * 255),
					255
				)
			);
		}

		// 绘制温度热力图
		float temp_y = canvas_pos.y + top_margin;

		// 温度标签和舒适区标识
		draw_list->AddText(ImVec2(canvas_pos.x, temp_y), IM_COL32(255, 255, 255, 255), "温度");

		// 绘制温度刻度线
		for (int i = 0; i <= 4; ++i)
		{
			float t = static_cast<float>(i) / 4.0f;
			float temp_val = temp_min + t * (temp_max - temp_min);
			float y = temp_y + bar_height * (1.0f - t);

			draw_list->AddLine(
				ImVec2(canvas_pos.x + 55, y),
				ImVec2(canvas_pos.x + canvas_size.x - 10, y),
				IM_COL32(60, 60, 70, 150),
				1.0f
			);

			std::string label = std::format("{:.0f}°", temp_val);
			draw_list->AddText(ImVec2(canvas_pos.x + 35, y - 7), IM_COL32(180, 180, 200, 255), label.c_str());
		}

		// 绘制湿度热力图
		float hum_y = temp_y + bar_height + bar_spacing;

		// 湿度标签
		draw_list->AddText(ImVec2(canvas_pos.x, hum_y), IM_COL32(255, 255, 255, 255), "湿度");

		// 绘制湿度刻度线
		for (int i = 0; i <= 3; ++i)
		{
			float t = static_cast<float>(i) / 3.0f;
			float hum_val = hum_min + t * (hum_max - hum_min);
			float y = hum_y + bar_height * (1.0f - t);

			draw_list->AddLine(
				ImVec2(canvas_pos.x + 55, y),
				ImVec2(canvas_pos.x + canvas_size.x - 10, y),
				IM_COL32(60, 60, 70, 150),
				1.0f
			);

			std::string label = std::format("{:.0f}%", hum_val);
			draw_list->AddText(ImVec2(canvas_pos.x + 35, y - 7), IM_COL32(180, 180, 200, 255), label.c_str());
		}

		// 绘制各房间的3D柱状图
		int visible_index = 0;
		ImVec2 mouse_pos = ImGui::GetMousePos();

		// 3D效果参数
		const float depth_offset_x = 6.0f;  // 深度偏移X
		const float depth_offset_y = 6.0f;  // 深度偏移Y

		for (size_t i = 0; i < rooms.size(); ++i)
		{
			const auto& room = rooms[i];
			if (!room.visible) continue;

			float x = canvas_pos.x + 60 + visible_index * room_width;
			float bar_w = room_width - 10.0f;

			// 温度3D柱（带立体效果）
			if (show_temperature)
			{
				float temp_normalized =
					std::clamp((room.temperature - temp_min) / (temp_max - temp_min), 0.0f, 1.0f);
				float temp_bar_height = bar_height * temp_normalized;

				auto temp_color = get_temperature_color(room.temperature);

				float bar_top = temp_y + bar_height - temp_bar_height;
				float bar_bottom = temp_y + bar_height;

				// 1. 绘制阴影（底部）
				draw_list->AddRectFilled(
					ImVec2(x + depth_offset_x + 2, bar_bottom + 2),
					ImVec2(x + bar_w + depth_offset_x, bar_bottom + 8),
					IM_COL32(0, 0, 0, 80),
					2.0f
				);

				// 2. 绘制右侧面（深色，模拟光照）
				ImU32 side_color_dark = IM_COL32(
					static_cast<int>(temp_color.r * 255 * 0.4f),
					static_cast<int>(temp_color.g * 255 * 0.4f),
					static_cast<int>(temp_color.b * 255 * 0.4f),
					230
				);
				draw_list->AddQuadFilled(
					ImVec2(x + bar_w, bar_top),
					ImVec2(x + bar_w + depth_offset_x, bar_top - depth_offset_y),
					ImVec2(x + bar_w + depth_offset_x, bar_bottom - depth_offset_y),
					ImVec2(x + bar_w, bar_bottom),
					side_color_dark
				);

				// 3. 绘制顶面（亮色，接受光照）
				ImU32 top_color_bright = IM_COL32(
					static_cast<int>(temp_color.r * 255 * 1.2f),
					static_cast<int>(temp_color.g * 255 * 1.2f),
					static_cast<int>(temp_color.b * 255 * 1.2f),
					230
				);
				draw_list->AddQuadFilled(
					ImVec2(x + 4, bar_top),
					ImVec2(x + bar_w, bar_top),
					ImVec2(x + bar_w + depth_offset_x, bar_top - depth_offset_y),
					ImVec2(x + 4 + depth_offset_x, bar_top - depth_offset_y),
					top_color_bright
				);

				// 4. 绘制正面（主体，渐变效果）
				ImU32 color_top = IM_COL32(
					static_cast<int>(temp_color.r * 255 * 0.8f),
					static_cast<int>(temp_color.g * 255 * 0.8f),
					static_cast<int>(temp_color.b * 255 * 0.8f),
					230
				);
				ImU32 color_bottom = IM_COL32(
					static_cast<int>(temp_color.r * 255),
					static_cast<int>(temp_color.g * 255),
					static_cast<int>(temp_color.b * 255),
					230
				);
				draw_list->AddRectFilledMultiColor(
					ImVec2(x + 4, bar_top),
					ImVec2(x + bar_w, bar_bottom),
					color_top,
					color_top,
					color_bottom,
					color_bottom
				);

				// 5. 绘制正面边框（增强立体感）
				draw_list->AddRect(
					ImVec2(x + 4, bar_top),
					ImVec2(x + bar_w, bar_bottom),
					IM_COL32(255, 255, 255, 120),
					0.0f,
					0,
					1.5f
				);

				// 6. 高光效果（顶部边缘）
				draw_list->AddLine(
					ImVec2(x + 4, bar_top),
					ImVec2(x + bar_w, bar_top),
					IM_COL32(255, 255, 255, 180),
					2.0f
				);

				// 数值标签（带3D效果）
				std::string label = std::format("{:.1f}°", room.temperature);
				ImVec2 text_size = ImGui::CalcTextSize(label.c_str());

				float label_y = bar_top - 20;
				float label_x = x + (bar_w - text_size.x) * 0.5f;

				// 标签阴影
				draw_list->AddText(ImVec2(label_x + 1, label_y + 1), IM_COL32(0, 0, 0, 150), label.c_str());
				// 标签背景
				draw_list->AddRectFilled(
					ImVec2(label_x - 3, label_y - 1),
					ImVec2(label_x + text_size.x + 3, label_y + 15),
					IM_COL32(40, 40, 50, 220),
					3.0f
				);
				// 标签文字
				draw_list->AddText(ImVec2(label_x, label_y), IM_COL32(255, 255, 255, 255), label.c_str());
			}

			// 湿度3D柱
			if (show_humidity)
			{
				float hum_normalized =
					std::clamp((room.humidity - hum_min) / (hum_max - hum_min), 0.0f, 1.0f);
				float hum_bar_height = bar_height * hum_normalized;

				auto hum_color = get_humidity_color(room.humidity);

				float bar_top = hum_y + bar_height - hum_bar_height;
				float bar_bottom = hum_y + bar_height;

				// 1. 绘制阴影
				draw_list->AddRectFilled(
					ImVec2(x + depth_offset_x + 2, bar_bottom + 2),
					ImVec2(x + bar_w + depth_offset_x, bar_bottom + 8),
					IM_COL32(0, 0, 0, 80),
					2.0f
				);

				// 2. 绘制右侧面（深色）
				ImU32 side_color_dark = IM_COL32(
					static_cast<int>(hum_color.r * 255 * 0.4f),
					static_cast<int>(hum_color.g * 255 * 0.4f),
					static_cast<int>(hum_color.b * 255 * 0.4f),
					230
				);
				draw_list->AddQuadFilled(
					ImVec2(x + bar_w, bar_top),
					ImVec2(x + bar_w + depth_offset_x, bar_top - depth_offset_y),
					ImVec2(x + bar_w + depth_offset_x, bar_bottom - depth_offset_y),
					ImVec2(x + bar_w, bar_bottom),
					side_color_dark
				);

				// 3. 绘制顶面（亮色）
				ImU32 top_color_bright = IM_COL32(
					static_cast<int>(hum_color.r * 255 * 1.2f),
					static_cast<int>(hum_color.g * 255 * 1.2f),
					static_cast<int>(hum_color.b * 255 * 1.2f),
					230
				);
				draw_list->AddQuadFilled(
					ImVec2(x + 4, bar_top),
					ImVec2(x + bar_w, bar_top),
					ImVec2(x + bar_w + depth_offset_x, bar_top - depth_offset_y),
					ImVec2(x + 4 + depth_offset_x, bar_top - depth_offset_y),
					top_color_bright
				);

				// 4. 绘制正面（渐变）
				ImU32 color_top = IM_COL32(
					static_cast<int>(hum_color.r * 255 * 0.8f),
					static_cast<int>(hum_color.g * 255 * 0.8f),
					static_cast<int>(hum_color.b * 255 * 0.8f),
					230
				);
				ImU32 color_bottom = IM_COL32(
					static_cast<int>(hum_color.r * 255),
					static_cast<int>(hum_color.g * 255),
					static_cast<int>(hum_color.b * 255),
					230
				);
				draw_list->AddRectFilledMultiColor(
					ImVec2(x + 4, bar_top),
					ImVec2(x + bar_w, bar_bottom),
					color_top,
					color_top,
					color_bottom,
					color_bottom
				);

				// 5. 正面边框
				draw_list->AddRect(
					ImVec2(x + 4, bar_top),
					ImVec2(x + bar_w, bar_bottom),
					IM_COL32(255, 255, 255, 120),
					0.0f,
					0,
					1.5f
				);

				// 6. 高光效果
				draw_list->AddLine(
					ImVec2(x + 4, bar_top),
					ImVec2(x + bar_w, bar_top),
					IM_COL32(255, 255, 255, 180),
					2.0f
				);

				// 数值标签
				std::string label = std::format("{:.0f}%", room.humidity);
				ImVec2 text_size = ImGui::CalcTextSize(label.c_str());

				float label_y = bar_top - 20;
				float label_x = x + (bar_w - text_size.x) * 0.5f;

				// 标签阴影
				draw_list->AddText(ImVec2(label_x + 1, label_y + 1), IM_COL32(0, 0, 0, 150), label.c_str());
				// 标签背景
				draw_list->AddRectFilled(
					ImVec2(label_x - 3, label_y - 1),
					ImVec2(label_x + text_size.x + 3, label_y + 15),
					IM_COL32(40, 40, 50, 220),
					3.0f
				);
				// 标签文字
				draw_list->AddText(ImVec2(label_x, label_y), IM_COL32(255, 255, 255, 255), label.c_str());
			}

			// 房间名称（底部）
			ImVec2 name_size = ImGui::CalcTextSize(room.name.c_str());
			draw_list->AddText(
				ImVec2(x + (bar_w - name_size.x) * 0.5f, canvas_pos.y + canvas_size.y - 22),
				IM_COL32(
					static_cast<int>(room.color.r * 255),
					static_cast<int>(room.color.g * 255),
					static_cast<int>(room.color.b * 255),
					255
				),
				room.name.c_str()
			);

			// 鼠标悬停效果
			if (mouse_pos.x >= x
				&& mouse_pos.x <= x + bar_w
				&& mouse_pos.y >= canvas_pos.y
				&& mouse_pos.y <= canvas_pos.y + canvas_size.y)
			{
				// 高亮边框
				draw_list->AddRect(
					ImVec2(x + 2, temp_y),
					ImVec2(x + bar_w + 2, hum_y + bar_height),
					IM_COL32(255, 255, 100, 200),
					3.0f,
					0,
					3.0f
				);

				// 详细信息提示框
				ImVec2 tooltip_pos = ImVec2(mouse_pos.x + 15, mouse_pos.y - 40);
				std::string tooltip = std::format(
					"{}\n温度: {:.1f}°C\n湿度: {:.1f}%",
					room.name,
					room.temperature,
					room.humidity
				);

				ImVec2 tooltip_size = ImGui::CalcTextSize(tooltip.c_str());
				draw_list->AddRectFilled(
					ImVec2(tooltip_pos.x - 5, tooltip_pos.y - 3),
					ImVec2(tooltip_pos.x + tooltip_size.x + 5, tooltip_pos.y + tooltip_size.y + 3),
					IM_COL32(30, 30, 40, 240),
					5.0f
				);
				draw_list->AddRect(
					ImVec2(tooltip_pos.x - 5, tooltip_pos.y - 3),
					ImVec2(tooltip_pos.x + tooltip_size.x + 5, tooltip_pos.y + tooltip_size.y + 3),
					IM_COL32(255, 255, 100, 255),
					5.0f,
					0,
					2.0f
				);
				draw_list->AddText(tooltip_pos, IM_COL32(255, 255, 255, 255), tooltip.c_str());
			}

			visible_index++;
		}

		ImGui::Dummy(canvas_size);
	}

	void Climate_viewer::draw_comparison_chart() noexcept
	{
		if (rooms.empty()) return;

		// 表格显示
		if (ImGui::BeginTable(
				"ComparisonTable",
				5,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable
			))
		{

			// 表头
			ImGui::TableSetupColumn("房间", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("温度(°C)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("湿度(%)", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("PM2.5", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("CO(ppm)", ImGuiTableColumnFlags_WidthFixed, 90.0f);
			ImGui::TableHeadersRow();

			// 数据行
			for (const auto& room : rooms)
			{
				if (!room.visible) continue;

				ImGui::TableNextRow();

				// 房间名称
				ImGui::TableNextColumn();
				ImGui::TextColored(
					ImVec4(room.color.r, room.color.g, room.color.b, 1.0f),
					"%s",
					room.name.c_str()
				);

				// 温度
				ImGui::TableNextColumn();
				auto temp_col = get_temperature_color(room.temperature);
				ImGui::TextColored(
					ImVec4(temp_col.r, temp_col.g, temp_col.b, 1.0f),
					"%.1f",
					room.temperature
				);

				// 湿度
				ImGui::TableNextColumn();
				auto hum_col = get_humidity_color(room.humidity);
				ImGui::TextColored(ImVec4(hum_col.r, hum_col.g, hum_col.b, 1.0f), "%.1f", room.humidity);

				// PM2.5
				ImGui::TableNextColumn();
				auto pm_col = get_pm25_color(room.pm25);
				ImGui::TextColored(ImVec4(pm_col.r, pm_col.g, pm_col.b, 1.0f), "%.1f", room.pm25);

				// CO
				ImGui::TableNextColumn();
				auto co_col = get_co_color(room.co);
				ImGui::TextColored(ImVec4(co_col.r, co_col.g, co_col.b, 1.0f), "%.1f", room.co);
			}

			ImGui::EndTable();
		}
	}

	void Climate_viewer::control_ui() noexcept
	{
		// 更新实时数据
		update_realtime_data(ImGui::GetIO().DeltaTime);

		// 火灾安全防护面板
		draw_fire_alarm_panel();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// 控制面板
		if (ImGui::CollapsingHeader("[显示设置]", ImGuiTreeNodeFlags_OpenOnArrow
		))
		{
			ImGui::Checkbox("温度", &show_temperature);
			ImGui::SameLine();
			ImGui::Checkbox("湿度", &show_humidity);
			ImGui::SameLine();
			ImGui::Checkbox("PM2.5", &show_pm25);
			ImGui::SameLine();
			ImGui::Checkbox("CO", &show_co);

			ImGui::Spacing();
			ImGui::Checkbox("热力图", &show_heatmap);
			ImGui::SameLine();
			ImGui::Checkbox("对比表", &show_comparison);
			ImGui::SameLine();
			ImGui::Checkbox("实时更新", &enable_realtime_update);

			if (enable_realtime_update)
			{
				ImGui::SliderFloat("更新间隔", &update_interval, 0.1f, 2.0f, "%.1f秒");
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// 热力图可视化
		if (show_heatmap)
		{
			if (ImGui::CollapsingHeader(
					"[热力图分布]（颜色渐变表示数值高低）", ImGuiTreeNodeFlags_OpenOnArrow
				))
			{
				draw_heatmap();
				ImGui::Spacing();
			}

			ImGui::Separator();
			ImGui::Spacing();
		}
		// 参数对比表
		if (show_comparison)
		{
			if (ImGui::CollapsingHeader("[房间环境总览]", ImGuiTreeNodeFlags_OpenOnArrow))
			{
				draw_comparison_chart();
				ImGui::Spacing();
			}

			ImGui::Separator();
			ImGui::Spacing();
		}

		// 房间卡片列表
		if (ImGui::CollapsingHeader("[房间详情]", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::Spacing();
			for (size_t i = 0; i < rooms.size(); ++i)
			{
				draw_room_card(i);
			}
		}
	}

	bool Climate_viewer::draw_thermometer(
		float& value,
		float min_value,
		float max_value,
		float comfort_min,
		float comfort_max,
		const char* label,
		const char* unit,
		glm::vec4 color,
		float pos_x,
		float pos_y,
		float width,
		float height
	) noexcept
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 pos = ImVec2(pos_x, pos_y);

		const float tube_width = width * 0.35f;
		const float bulb_radius = width * 0.5f;
		const float tube_height = height - bulb_radius * 1.5f;
		const float scale_padding = 8.0f;

		// 温度计主体位置
		float tube_x = pos.x + width * 0.5f;
		float tube_top = pos.y;
		float tube_bottom = pos.y + tube_height;
		float bulb_center_y = tube_bottom + bulb_radius * 0.8f;

		// 绘制温度计外壳（玻璃管）
		draw_list->AddRectFilled(
			ImVec2(tube_x - tube_width * 0.5f, tube_top),
			ImVec2(tube_x + tube_width * 0.5f, tube_bottom),
			IM_COL32(80, 80, 90, 200),
			tube_width * 0.3f
		);

		// 绘制底部球泡
		draw_list->AddCircleFilled(ImVec2(tube_x, bulb_center_y), bulb_radius, IM_COL32(80, 80, 90, 200));

		// 绘制舒适区间背景
		float comfort_min_y = tube_bottom - (comfort_min - min_value) / (max_value - min_value) * tube_height;
		float comfort_max_y = tube_bottom - (comfort_max - min_value) / (max_value - min_value) * tube_height;
		draw_list->AddRectFilled(
			ImVec2(tube_x - tube_width * 0.4f, comfort_max_y),
			ImVec2(tube_x + tube_width * 0.4f, comfort_min_y),
			IM_COL32(100, 255, 100, 60)
		);

		// 计算当前值的液柱高度
		float normalized = std::clamp((value - min_value) / (max_value - min_value), 0.0f, 1.0f);
		float liquid_top = tube_bottom - normalized * tube_height;

		// 绘制液柱
		draw_list->AddRectFilled(
			ImVec2(tube_x - tube_width * 0.25f, liquid_top),
			ImVec2(tube_x + tube_width * 0.25f, tube_bottom),
			IM_COL32(
				static_cast<int>(color.r * 255),
				static_cast<int>(color.g * 255),
				static_cast<int>(color.b * 255),
				255
			),
			tube_width * 0.2f
		);

		// 绘制球泡中的液体
		draw_list->AddCircleFilled(
			ImVec2(tube_x, bulb_center_y),
			bulb_radius * 0.85f,
			IM_COL32(
				static_cast<int>(color.r * 255),
				static_cast<int>(color.g * 255),
				static_cast<int>(color.b * 255),
				255
			)
		);

		// 绘制刻度线和数值
		int num_ticks = 5;
		for (int i = 0; i <= num_ticks; ++i)
		{
			float t = static_cast<float>(i) / num_ticks;
			float tick_value = min_value + t * (max_value - min_value);
			float tick_y = tube_bottom - t * tube_height;

			// 刻度线
			draw_list->AddLine(
				ImVec2(tube_x + tube_width * 0.5f, tick_y),
				ImVec2(tube_x + tube_width * 0.5f + scale_padding, tick_y),
				IM_COL32(200, 200, 200, 255),
				1.5f
			);

			// 刻度值
			std::string tick_text = std::format("{:.0f}", tick_value);
			draw_list->AddText(
				ImVec2(tube_x + tube_width * 0.5f + scale_padding + 3, tick_y - 7),
				IM_COL32(200, 200, 200, 255),
				tick_text.c_str()
			);
		}

		// 绘制当前值指示器
		float indicator_y = liquid_top;
		draw_list->AddTriangleFilled(
			ImVec2(tube_x - tube_width * 0.5f - 5, indicator_y),
			ImVec2(tube_x - tube_width * 0.5f, indicator_y - 4),
			ImVec2(tube_x - tube_width * 0.5f, indicator_y + 4),
			IM_COL32(255, 255, 100, 255)
		);

		// 当前值文本
		std::string value_text = std::format("{:.1f}{}", value, unit);
		ImVec2 value_text_size = ImGui::CalcTextSize(value_text.c_str());
		draw_list->AddRectFilled(
			ImVec2(tube_x - tube_width * 0.5f - value_text_size.x - 15, indicator_y - 10),
			ImVec2(tube_x - tube_width * 0.5f - 8, indicator_y + 10),
			IM_COL32(40, 40, 50, 230),
			3.0f
		);
		draw_list->AddText(
			ImVec2(tube_x - tube_width * 0.5f - value_text_size.x - 12, indicator_y - 7),
			IM_COL32(255, 255, 100, 255),
			value_text.c_str()
		);

		// 标签文本
		ImVec2 label_size = ImGui::CalcTextSize(label);
		draw_list->AddText(
			ImVec2(pos.x + (width - label_size.x) * 0.5f, pos.y + height + 5),
			IM_COL32(220, 220, 230, 255),
			label
		);

		// 舒适区间标注
		std::string comfort_text = "舒适区";
		ImVec2 comfort_text_size = ImGui::CalcTextSize(comfort_text.c_str());
		float comfort_mid_y = (comfort_min_y + comfort_max_y) * 0.5f;
		draw_list->AddText(
			ImVec2(tube_x + tube_width * 0.5f + comfort_text_size.x, comfort_mid_y - 7),
			IM_COL32(100, 255, 100, 200),
			comfort_text.c_str()
		);

		// 鼠标交互检测
		bool value_changed = false;
		ImVec2 mouse_pos = ImGui::GetMousePos();

		// 检测是否在温度计交互区域内
		if (mouse_pos.x >= pos.x
			&& mouse_pos.x <= pos.x + width
			&& mouse_pos.y >= tube_top
			&& mouse_pos.y <= tube_bottom + bulb_radius)
		{
			if (ImGui::IsMouseDown(0))
			{
				// 根据鼠标Y位置计算新值
				float mouse_t = std::clamp((tube_bottom - mouse_pos.y) / tube_height, 0.0f, 1.0f);
				float new_value = min_value + mouse_t * (max_value - min_value);

				if (std::abs(new_value - value) > 0.1f)
				{
					value = new_value;
					value_changed = true;
				}
			}

			// 高亮显示
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			draw_list->AddRect(
				ImVec2(tube_x - tube_width * 0.6f, tube_top - 2),
				ImVec2(tube_x + tube_width * 0.6f, tube_bottom + 2),
				IM_COL32(255, 255, 255, 100),
				tube_width * 0.3f,
				0,
				2.0f
			);
		}

		return value_changed;
	}

	bool Climate_viewer::draw_gauge(
		float& value,
		float max_value,
		const char* label,
		glm::vec4 color,
		float pos_x,
		float pos_y,
		float radius
	) noexcept
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 pos = ImVec2(pos_x, pos_y);

		const float thickness = radius * 0.15f;
		const float start_angle = std::numbers::pi_v<float> * 0.75f;  // 起始角度（左下）
		const float end_angle = std::numbers::pi_v<float> * 2.25f;    // 结束角度（右下）
		const float angle_range = end_angle - start_angle;

		// 绘制背景圆弧
		draw_list->PathArcTo(pos, radius, start_angle, end_angle, 32);
		draw_list->PathStroke(IM_COL32(60, 60, 70, 255), 0, thickness);

		// 计算当前值对应的角度
		float normalized = std::clamp(value / max_value, 0.0f, 1.0f);
		float current_angle = start_angle + angle_range * normalized;

		// 绘制当前值的彩色圆弧
		draw_list->PathArcTo(pos, radius, start_angle, current_angle, 32);
		draw_list->PathStroke(
			IM_COL32(
				static_cast<int>(color.r * 255),
				static_cast<int>(color.g * 255),
				static_cast<int>(color.b * 255),
				255
			),
			0,
			thickness * 1.2f
		);

		// 绘制中心点
		draw_list->AddCircleFilled(pos, radius * 0.1f, IM_COL32(200, 200, 220, 255));

		// 绘制指针
		float pointer_angle = current_angle;
		float pointer_length = radius * 0.8f;
		ImVec2 pointer_end = ImVec2(
			pos.x + std::cos(pointer_angle) * pointer_length,
			pos.y + std::sin(pointer_angle) * pointer_length
		);
		draw_list->AddLine(pos, pointer_end, IM_COL32(255, 255, 255, 255), thickness * 0.5f);

		// 检测鼠标交互
		bool value_changed = false;
		ImVec2 mouse_pos = ImGui::GetMousePos();
		float dist_to_center = std::sqrt(
			(mouse_pos.x - pos.x) * (mouse_pos.x - pos.x) + (mouse_pos.y - pos.y) * (mouse_pos.y - pos.y)
		);

		// 检测是否在仪表盘交互区域内
		if (dist_to_center <= radius * 1.2f && ImGui::IsMouseDown(0))
		{
			// 计算鼠标相对于中心的角度
			float mouse_angle = std::atan2(mouse_pos.y - pos.y, mouse_pos.x - pos.x);

			// 将角度归一化到[0, 2π)
			if (mouse_angle < 0)
			{
				mouse_angle += 2.0f * std::numbers::pi_v<float>;
			}

			// 计算相对于起始角度的偏移
			float relative_angle = mouse_angle - start_angle;
			if (relative_angle < 0)
			{
				relative_angle += 2.0f * std::numbers::pi_v<float>;
			}

			// 限制在有效范围内
			if (relative_angle <= angle_range)
			{
				float new_normalized = relative_angle / angle_range;
				float new_value = new_normalized * max_value;
				new_value = std::clamp(new_value, 0.0f, max_value);

				if (std::abs(new_value - value) > 0.1f)
				{
					value = new_value;
					value_changed = true;
				}
			}
		}

		// 高亮显示交互区域
		if (dist_to_center <= radius * 1.2f)
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			// 绘制半透明高亮圆
			draw_list->AddCircle(pos, radius, IM_COL32(255, 255, 255, 100), 32, thickness * 0.3f);
		}

		// 绘制数值文本
		std::string value_text = std::format("{:.1f}", value);
		ImVec2 text_size = ImGui::CalcTextSize(value_text.c_str());
		draw_list->AddText(
			ImVec2(pos.x - text_size.x * 0.5f, pos.y + radius * 0.3f),
			IM_COL32(255, 255, 255, 255),
			value_text.c_str()
		);

		// 绘制标签
		ImVec2 label_size = ImGui::CalcTextSize(label);
		draw_list->AddText(
			ImVec2(pos.x - label_size.x * 0.5f, pos.y + radius),
			IM_COL32(200, 200, 220, 255),
			label
		);

		// 绘制最小值标注
		std::string min_text = "0";
		draw_list->AddText(
			ImVec2(pos.x - radius - 20, pos.y + radius * 0.5f),
			IM_COL32(150, 150, 170, 255),
			min_text.c_str()
		);

		// 绘制最大值标注
		std::string max_text = std::format("{:.0f}", max_value);
		draw_list->AddText(
			ImVec2(pos.x + radius + 10, pos.y + radius * 0.5f),
			IM_COL32(150, 150, 170, 255),
			max_text.c_str()
		);

		return value_changed;
	}

	void Climate_viewer::update_from_time_of_day(float time_of_day) noexcept
	{
		// 根据时间计算温度和湿度的目标值
		// 白天 (6:00-18:00): 温度升高，湿度降低
		// 夜晚 (18:00-6:00): 温度降低，湿度升高

		const float sunrise = 6.0f;
		const float sunset = 18.0f;

		float day_factor = 0.0f;  // 0 = 完全夜晚, 1 = 完全白天

		if (time_of_day >= sunrise && time_of_day <= sunset)
		{
			// 白天: 使用正弦曲线模拟温度变化
			const float day_progress = (time_of_day - sunrise) / (sunset - sunrise);
			day_factor = std::sin(day_progress * std::numbers::pi_v<float>);
		}
		else
		{
			// 夜晚
			day_factor = 0.0f;
		}

		// 更新每个房间的温度和湿度
		for (auto& room : rooms)
		{
			// 平滑插值到目标温度和湿度
			const float target_temp = glm::mix(room.base_night_temp, room.base_day_temp, day_factor);
			const float target_humidity =
				glm::mix(room.base_night_humidity, room.base_day_humidity, day_factor);

			// 使用较慢的插值速度，让变化更平滑
			const float lerp_speed = 0.02f;
			room.temperature = glm::mix(room.temperature, target_temp, lerp_speed);
			room.humidity = glm::mix(room.humidity, target_humidity, lerp_speed);

			// 限制范围
			room.temperature = std::clamp(room.temperature, 15.0f, 35.0f);
			room.humidity = std::clamp(room.humidity, 30.0f, 90.0f);
		}
	}

	bool Climate_viewer::is_zone_in_danger(Room_zone zone) const noexcept
	{
		size_t zone_index = static_cast<size_t>(zone);
		if (zone_index < rooms.size())
		{
			return rooms[zone_index].in_danger;
		}
		return false;
	}

}