#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace logic
{
	// 区域标识枚举（对应门窗索引）
	enum class Room_zone
	{
		Bedroom1 = 0,   // 卧室1 - door1
		Kitchen = 1,    // 厨房 - door2
		Bathroom = 2,   // 卫生间 - door3
		Bedroom2 = 3,   // 卧室2 - door4
		LivingRoom = 4  // 客厅 - door5 + curtains
	};

	struct Climate_data
	{
		std::string name;       // 房间名称
		Room_zone zone;         // 区域标识
		float temperature;      // 温度 (°C)
		float humidity;         // 湿度 (%)
		float pm25;             // PM2.5 (μg/m³)
		float co;               // CO 浓度 (ppm)
		glm::vec4 color;        // 房间颜色标识
		bool visible;           // 是否显示
		bool expanded;          // 是否展开详细信息
		bool show_room_gauges;  // 是否显示房间仪表盘

		// 模拟实时变化
		float temp_variation;      // 温度变化率
		float humidity_variation;  // 湿度变化率
		float pm25_variation;      // PM2.5变化率

		// 白天/夜晚基准值
		float base_day_temp;        // 白天基准温度
		float base_night_temp;      // 夜晚基准温度
		float base_day_humidity;    // 白天基准湿度
		float base_night_humidity;  // 夜晚基准湿度

		// 安全防护状态
		bool door_open;     // 门是否打开
		bool window_open;   // 窗是否打开
		bool curtain_open;  // 窗帘是否打开
		bool in_danger;     // 是否处于危险状态

		Climate_data(
			std::string name,
			Room_zone zone,
			float temperature,
			float humidity,
			float pm25 = 35.0f,
			float co = 10.0f,
			glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
		) :
			name(std::move(name)),
			zone(zone),
			temperature(temperature),
			humidity(humidity),
			pm25(pm25),
			co(co),
			color(color),
			visible(true),
			expanded(false),
			show_room_gauges(true),
			temp_variation(0.0f),
			humidity_variation(0.0f),
			pm25_variation(0.0f),
			base_day_temp(temperature + 2.0f),
			base_night_temp(temperature - 3.0f),
			base_day_humidity(humidity - 5.0f),
			base_night_humidity(humidity + 10.0f),
			door_open(false),
			window_open(false),
			curtain_open(false),
			in_danger(false)
		{}
	};

	class Climate_viewer
	{
		std::vector<Climate_data> rooms;

		// 显示选项
		bool show_temperature = true;
		bool show_humidity = true;
		bool show_pm25 = true;
		bool show_co = true;
		bool show_heatmap = true;
		bool show_comparison = true;  // 显示对比表
		bool enable_realtime_update = true;

		// 安全防护系统
		bool fire_alarm_active = false;        // 火灾报警是否激活
		float alarm_timer = 0.0f;              // 报警计时器（用于闪烁效果）
		float pm25_danger_threshold = 100.0f;  // PM2.5危险阈值 (μg/m³)
		float co_danger_threshold = 35.0f;     // CO危险阈值 (ppm)
		bool enable_fire_detection = true;     // 启用火灾检测

		// 排序选项
		enum class SortMode
		{
			None,
			Temperature,
			Humidity,
			PM25,
			CO
		};
		SortMode sort_mode = SortMode::None;

		// 实时更新
		float update_timer = 0.0f;
		float update_interval = 0.5f;  // 秒

		// 颜色映射函数
		glm::vec4 get_temperature_color(float temperature) const noexcept;
		glm::vec4 get_humidity_color(float humidity) const noexcept;
		glm::vec4 get_pm25_color(float pm25) const noexcept;
		glm::vec4 get_co_color(float co) const noexcept;

		// 获取数值对应的渐变颜色（用于对比显示）
		glm::vec4 get_gradient_color(float value, float min_val, float max_val) const noexcept;

		// 绘制房间卡片
		void draw_room_card(size_t index) noexcept;
		void draw_room_card_header(Climate_data& room, size_t index) noexcept;
		void draw_room_card_controls(Climate_data& room) noexcept;
		void draw_room_thermometers(Climate_data& room) noexcept;
		void draw_room_gauges(Climate_data& room) noexcept;

		// 绘制热力图可视化
		void draw_heatmap() noexcept;

		// 绘制对比图表
		void draw_comparison_chart() noexcept;

		// 绘制仪表盘（返回是否值被修改）
		bool draw_gauge(
			float& value,
			float max_value,
			const char* label,
			glm::vec4 color,
			float pos_x,
			float pos_y,
			float radius
		) noexcept;
		void draw_gauges_panel() noexcept;

		// 绘制垂直温度计样式（返回是否值被修改）
		bool draw_thermometer(
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
		) noexcept;

		// 实时更新数据
		void update_realtime_data(float delta_time) noexcept;

		// 火灾检测和安全防护
		void check_fire_hazard() noexcept;
		void trigger_fire_alarm() noexcept;
		void reset_fire_alarm() noexcept;
		void open_all_doors_and_windows() noexcept;
		void draw_fire_alarm_panel() noexcept;
		void draw_door_window_status(const Climate_data& room, float x, float y, float width) noexcept;

	  public:

		Climate_viewer();

		// 获取房间数据
		const std::vector<Climate_data>& get_rooms() const noexcept { return rooms; }
		// 获取火灾报警状态
		bool is_fire_alarm_active() const noexcept { return fire_alarm_active; }
		// 获取指定区域是否处于危险状态
		bool is_zone_in_danger(Room_zone zone) const noexcept;
		// UI控制
		void control_ui() noexcept;

		// 根据时间更新温度和湿度
		void update_from_time_of_day(float time_of_day) noexcept;
	};
}
