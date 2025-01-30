#pragma once
#include <memory>
#include "../math/qangle.hpp"
#include <imgui.h> /*important for the imvec4*/

namespace aether {
	class context;

	class aimbot_state {
	public:
		bool enabled{ false };
		bool show_fov{ false };
		float fov{ 3.0f };
		float dps{ 10.0f };
	};

	class esp_state {
	public:
		bool enabled{ false };
		bool show_snaplines{ false };
		bool show_name{ false };
		bool show_box_2d{ false };
		bool show_health_bar{ false };
		bool show_hitboxes{ false };

		ImVec4 box_color{ 0.0f, 1.0f, 0.0f, 1.0f };
		ImVec4 box_outline_color{ 0.0f, 0.0f, 0.0f, 0.2f };
		float box_thickness{ 1.0f };
		float box_outline_thickness{ 1.0f };

		bool healthbar_outline{ false };
		ImVec4 healthbar_outline_color{ 0.0f, 0.0f, 0.0f, 0.2f };
		float healthbar_thickness{ 2.0f };
		float healthbar_outline_thickness{ 0.5f };

		float healthbar_offset_x{ 4.0f };
		float name_offset_y{ -4.0f };
	};

	class config {
	public:
		config(context& cfg);

		std::unique_ptr<aimbot_state> aimbot;
		std::unique_ptr<esp_state> esp;

	protected:
		context& m_cfg;
	};
}
