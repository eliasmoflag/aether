#include <imgui.h>
#include <format>
#include "../../cs2/client/player_pawn.hpp"
#include "../../context.hpp"
#include "../../ui/ui_manager.hpp"
#include "../../config/config.hpp"
#include "aimbot_view.hpp"

namespace aether {
	aimbot_view::aimbot_view() {}

	void aimbot_view::render() {
        auto& ui{ *context::get().ui() };
        auto& cfg{ *context::get().cfg()->aimbot };

        if (cfg.enabled) {
            auto& draw_list{ *ImGui::GetBackgroundDrawList() };
            if (cfg.show_fov) {
                draw_list.AddCircleFilled(
                    ImGui::GetIO().DisplaySize * 0.5f,
                    ImGui::GetIO().DisplaySize.x * (cfg.fov / 90.0f) * 0.5f,
                    ImColor(120, 81, 169, 75), 100
                );

                draw_list.AddCircle(
                    ImGui::GetIO().DisplaySize * 0.5f,
                    ImGui::GetIO().DisplaySize.x * (cfg.fov / 90.0f) * 0.5f,
                    ImColor(120, 81, 169, 200), 100, 1.0f
                );
            }
        }

        if (!ui.is_open()) {
            return;
        }

        ImGui::SetNextWindowPos({ 700.0f, 100.0f }, ImGuiCond_Once);
        ImGui::SetNextWindowSize({ 250.0f, 250.0f }, ImGuiCond_Once);
        if (ImGui::Begin("Aimbot")) {
            ImGui::Checkbox("Enabled", &cfg.enabled);
            ImGui::Checkbox("Show FOV", &cfg.show_fov);
            ImGui::SliderFloat("FOV", &cfg.fov, 1.0f, 90.0f, "%.1f");
            ImGui::SliderFloat("DPS", &cfg.dps, 0.0f, 25.0f, "%.1f");
			if (ImGui::Button("unload")) {
				context::get().queue_shutdown();
			}
        }
        ImGui::End();
	}
}
