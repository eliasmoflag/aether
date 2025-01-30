#include <imgui.h>
#include <string>
#include <array>
#include "../../context.hpp"
#include "../../ui/ui_manager.hpp"
#include "../../config/config.hpp"
#include "../../cs2/client/entity.hpp"
#include "../../cs2/client/player_pawn.hpp"
#include "../../cs2/client/player_controller.hpp"
#include "../../cs2/client/entity_system.hpp"
#include "../../cs2/client/view_render.hpp"
#include "esp_view.hpp"

namespace aether {
	esp_view::esp_view() {}

	void esp_view::render() {
        auto& ui{ *context::get().ui() };
        auto& cfg{ *context::get().cfg()->esp };

        render_player_esp();

        if (!ui.is_open()) {
            return;
        }

        ImGui::SetNextWindowPos({ 375.0f, 100.0f }, ImGuiCond_Once);
        ImGui::SetNextWindowSize({ 250.0f, 250.0f }, ImGuiCond_Once);

        if (ImGui::Begin("ESP")) {
            ImGui::Checkbox("Enabled", &cfg.enabled);
            ImGui::Checkbox("Show Snaplines", &cfg.show_snaplines);
#ifdef _DEBUG
            ImGui::Checkbox("Show Hitboxes", &cfg.show_hitboxes);
#endif
            ImGui::Checkbox("Show Box 2D", &cfg.show_box_2d);
        }
        ImGui::End();
	}

    static bool world_to_screen(const vec3& position, ImVec2& screen);

    void esp_view::render_player_esp() {

        auto& cfg{ *context::get().cfg()->esp };
        if (!cfg.enabled) {
            return;
        }

        const auto local_player{ cs2::CCSPlayerController::get_local_player() };
        if (!local_player or !local_player->get_pawn()) {
            return;
        }

        for (std::size_t i{ 1 }; i <= 64; i++) {

            const auto player{ cs2::CGameEntitySystem::get()->get_entity<cs2::CCSPlayerController>(i) };

            if (!player or !player->is_alive() or player == local_player or
                std::strcmp(player->get_entity_type_name(), "cs_player_controller") != 0) {

                continue;
            }

            if (player->team_number() == local_player->team_number()) {
                continue;
            }

            const auto player_pawn{ player->get_pawn() };
            if (!player_pawn or player_pawn->is_dormant()) {
                continue;
            }

            draw_snapline(player_pawn);
            draw_hitboxes(player_pawn);
            draw_bounding_box(player_pawn);
        }
    }

    void esp_view::draw_snapline(cs2::C_CSPlayerPawn* player_pawn) {
        auto& cfg{ *context::get().cfg()->esp };
        if (cfg.show_snaplines) {
            return;
        }

        if (ImVec2 pos; world_to_screen(player_pawn->abs_origin(), pos)) {

            auto& draw_list{ *ImGui::GetBackgroundDrawList() };
            const auto& display_size{ ImGui::GetIO().DisplaySize };

            draw_list.AddLine(
                { display_size.x * 0.5f, display_size.y },
                pos, ImColor(120, 81, 169, 200), 1.0f
            );
        }
    }

    void esp_view::draw_hitboxes(cs2::C_CSPlayerPawn* player_pawn) {
        auto& cfg{ *context::get().cfg()->esp };
        if (cfg.show_hitboxes) {
            return;
        }

        const auto skeleton{ player_pawn->anim_graph()->skeleton() };

        for (std::int32_t bone_idx{ 0 }; bone_idx < skeleton->bone_count(); bone_idx++) {

            if (ImVec2 pos; world_to_screen(skeleton->get_bone(bone_idx).position.xyz(), pos)) {

                auto& draw_list{ *ImGui::GetBackgroundDrawList() };

                draw_list.AddText(pos, ImColor(120, 81, 169, 200), std::to_string(bone_idx).c_str());
            }
        }
    }

    void esp_view::draw_bounding_box(cs2::C_CSPlayerPawn* player_pawn) {
        auto& cfg{ *context::get().cfg()->esp };
        if (!cfg.show_box_2d) {
            return;
        }

        const auto collision_property{ player_pawn->collision_property() };
        const auto min{ player_pawn->abs_origin() + collision_property->mins() };
        const auto max{ player_pawn->abs_origin() + collision_property->maxs() };

        std::array<vec3, 8> points{
            vec3{ min.x, min.y, min.z },
            vec3{ min.x, max.y, min.z },
            vec3{ max.x, max.y, min.z },
            vec3{ max.x, min.y, min.z },
            vec3{ max.x, max.y, max.z },
            vec3{ min.x, max.y, max.z },
            vec3{ min.x, min.y, max.z },
            vec3{ max.x, min.y, max.z },
        };

        std::array<ImVec2, 8> screen_points;
        for (std::size_t i{ 0 }; i < 8; i++) {
            if (!world_to_screen(points[i], screen_points[i])) {
                return;
            }
        }

        float left{ screen_points[3].x };
        float top{ screen_points[3].y };
        float right{ screen_points[3].x };
        float bottom{ screen_points[3].y };

        for (std::size_t i{ 1 }; i < 8; i++) {

            if (left > screen_points[i].x) {
                left = screen_points[i].x;
            }

            if (top < screen_points[i].y) {
                top = screen_points[i].y;
            }

            if (right < screen_points[i].x) {
                right = screen_points[i].x;
            }

            if (bottom > screen_points[i].y) {
                bottom = screen_points[i].y;
            }
        }

        auto& draw_list{ *ImGui::GetBackgroundDrawList() };

        draw_list.AddRect({ left, top }, { right, bottom }, ImColor(10, 10, 10, 200), 0.0f, ImDrawFlags_RoundCornersNone, 3.0f);
        draw_list.AddRect({ left, top }, { right, bottom }, ImColor(120, 81, 169, 200), 0.0f, ImDrawFlags_RoundCornersNone, 1.0f);
    }

    bool world_to_screen(const vec3& position, ImVec2& screen) {

        const auto& vm{ cs2::CViewRender::get()->view_matrix() };
        const auto [x, y, z, w] = vm * position;

        if (w < 0.01f) {
            return false;
        }

        const auto& display_size{ ImGui::GetIO().DisplaySize };

        screen = ImVec2{
            (display_size.x / 2.0f) * (1.0f + x / w),
            (display_size.y / 2.0f) * (1.0f - y / w)
        };

        return true;
    }
}
