#include <imgui.h>
#include <imgui_internal.h>
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
	esp_view::esp_view(ImFont* esp_font)
        : m_esp_font(esp_font) {}

	void esp_view::render() {
        auto& ui{ *context::get().ui() };
        auto& cfg{ *context::get().cfg()->esp };

        if (ui.is_open()) {
            ImGui::SetNextWindowPos({ 375.0f, 100.0f }, ImGuiCond_Once);
            ImGui::SetNextWindowSize({ 250.0f, 250.0f }, ImGuiCond_Once);

            if (ImGui::Begin("ESP")) {
                ImGui::Checkbox("Enabled", &cfg.enabled);
                ImGui::Checkbox("Show Snaplines", &cfg.show_snaplines);
                ImGui::Checkbox("Show Name", &cfg.show_name);
                if (ImGui::TreeNode("Box Settings")) {
                    ImGui::Checkbox("Show Box 2D", &cfg.show_box_2d);
                    ImGui::ColorEdit4("Box Color", (float*)&cfg.box_color,
                        ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
                    ImGui::ColorEdit4("Outline Color", (float*)&cfg.box_outline_color,
                        ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
                    ImGui::SliderFloat("Box Thickness", &cfg.box_thickness, 0.1f, 3.0f);
                    ImGui::SliderFloat("Outline Thickness", &cfg.box_outline_thickness, 0.1f, 3.0f);
                    ImGui::SliderFloat("Horizontal Offset", &cfg.healthbar_offset_x,-10.0f, 10.0f, "%.1f");
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Healthbar Settings")) {
                    ImGui::Checkbox("Show Health Bar", &cfg.show_health_bar);
                    ImGui::Checkbox("Outline", &cfg.healthbar_outline);

                    if (cfg.healthbar_outline) {
                        ImGui::ColorEdit4("Outline Color", (float*)&cfg.healthbar_outline_color,
                            ImGuiColorEditFlags_AlphaBar |
                            ImGuiColorEditFlags_AlphaPreviewHalf);
                        ImGui::SliderFloat("Outline Thickness", &cfg.healthbar_outline_thickness, 0.2f, 1.5f);
                        ImGui::SliderFloat("Vertical Offset", &cfg.name_offset_y, -20.0f, 20.0f, "%.1f");
                        ImGui::SliderFloat("Healthbar Thickness", &cfg.healthbar_thickness, 1.0f, 5.0f);
                    }
                    ImGui::TreePop();
                }

#ifdef _DEBUG
                ImGui::Checkbox("Show Hitboxes", &cfg.show_hitboxes);
#endif
            }
            ImGui::End();
        }

        if (cfg.enabled) {
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

                draw_player_esp(player, player_pawn);
            }
        }
	}

    static bool world_to_screen(const vec3& position, ImVec2& screen);

    void esp_view::draw_player_esp(cs2::CCSPlayerController* player, cs2::C_CSPlayerPawn* player_pawn) {
        auto& cfg{ *context::get().cfg()->esp };

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
        const auto& display_size{ ImGui::GetIO().DisplaySize };

        if (cfg.show_snaplines) {
            draw_list.AddLine(
                { display_size.x * 0.5f, display_size.y },
                { left + (right - left) * 0.5f, top }, ImColor(120, 81, 169, 200), 1.0f
            );
        }

        if (cfg.show_name) {
            ImGui::PushFont(m_esp_font);

            const auto name{ player->get_name() };
            const auto name_size{ ImGui::CalcTextSize(name) };
            const ImVec2 name_pos{left + (right - left) * 0.5f - name_size.x * 0.5f, bottom - name_size.y + cfg.name_offset_y};
            draw_list.AddText(name_pos, ImColor(255, 255, 255, 255), name);

            ImGui::PopFont();
        }

        if (cfg.show_box_2d) {
            // outline
            draw_list.AddRect(
                { left - 1, top - 1 },
                { right + 1, bottom + 1 },
                ImColor(cfg.box_outline_color),
                0.0f,
                ImDrawFlags_RoundCornersNone,
                cfg.box_outline_thickness
            );

            // box
            draw_list.AddRect(
                { left, top },
                { right, bottom },
                ImColor(cfg.box_color),
                0.0f,
                ImDrawFlags_RoundCornersNone,
                cfg.box_thickness
            );
        }

        if (cfg.show_health_bar) {
            const float health{ static_cast<float>(player_pawn->health()) / 100.0f };
            const float health_height = (bottom - top) * health;
            const float healthbar_left = left - cfg.healthbar_thickness - cfg.healthbar_offset_x;

            if (cfg.healthbar_outline) {
                const float outline_offset = cfg.healthbar_outline_thickness * 0.5f;
                draw_list.AddRect(
                    {
                        healthbar_left - outline_offset,
                        top - outline_offset
                    },
            {
                healthbar_left + cfg.healthbar_thickness + outline_offset,
                top + health_height + outline_offset
            },
                    ImColor(cfg.healthbar_outline_color),
                    0.0f,
                    ImDrawFlags_RoundCornersNone,
                    cfg.healthbar_outline_thickness
                );
            }

            ImColor health_colour = ImColor::HSV(health * 0.33f, 1.0f, 1.0f);
            health_colour.Value.w = 0.8f;

            draw_list.AddRectFilled(
                { healthbar_left, top },
                { healthbar_left + cfg.healthbar_thickness, top + health_height },
                health_colour
            );
        }

        if (cfg.show_hitboxes) {
            const auto skeleton{ player_pawn->anim_graph()->skeleton() };

            for (std::int32_t bone_idx{ 0 }; bone_idx < skeleton->bone_count(); bone_idx++) {

                if (ImVec2 pos; world_to_screen(skeleton->get_bone(bone_idx).position.xyz(), pos)) {

                    auto& draw_list{ *ImGui::GetBackgroundDrawList() };

                    draw_list.AddText(pos, ImColor(120, 81, 169, 200), std::to_string(bone_idx).c_str());
                }
            }
        }
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
