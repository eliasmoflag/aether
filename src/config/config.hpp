#pragma once
#include <memory>
#include <functional>
#include "../math/qangle.hpp"

namespace aether {
	class context;

	class aimbot_state {
	public:
		bool enabled{ false };
		bool show_fov{ false };
		float fov{ 10.0f };
	};

	class esp_state {
	public:
		bool enabled{ false };
		bool show_snaplines{ false };
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
