#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "config.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"


// =========================================================
// 
// These are the tabs on the sidebar
// 
// =========================================================
static char* sidebar_tabs[] = {
    "ESP",
    "AIM",
    "MISC",
    "CONFIG"
};

constexpr static float get_sidebar_item_width() { return 150.0f; }
constexpr static float get_sidebar_item_height() { return  50.0f; }

enum {
	TAB_ESP,
	TAB_AIMBOT,
	TAB_MISC,
	TAB_CONFIG
};

namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha)) {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
            return true;
        }
        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline)
{
    bool values[N] = { false };

    values[activetab] = true;

    for(auto i = 0; i < N; ++i) {
        if(ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) {
            activetab = i;
        }
        if(sameline && i < N - 1)
            ImGui::SameLine();
    }
}

ImVec2 get_sidebar_size()
{
    constexpr float padding = 10.0f;
    constexpr auto size_w = padding * 2.0f + get_sidebar_item_width();
    constexpr auto size_h = padding * 2.0f + (sizeof(sidebar_tabs) / sizeof(char*)) * get_sidebar_item_height();

    return ImVec2{ size_w, ImMax(325.0f, size_h) };
}

int get_fps()
{
    using namespace std::chrono;
    static int count = 0;
    static auto last = high_resolution_clock::now();
    auto now = high_resolution_clock::now();
    static int fps = 0;

    count++;

    if(duration_cast<milliseconds>(now - last).count() > 1000) {
        fps = count;
        count = 0;
        last = now;
    }

    return fps;
}

void RenderEspTab()
{
    static char* esp_tab_names[] = { "ESP", "GLOW", "CHAMS" };
    static int   active_esp_tab = 0;

    bool placeholder_true = true;

    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    {
        render_tabs(esp_tab_names, active_esp_tab, group_w / _countof(esp_tab_names), 25.0f, true);
    }
    ImGui::PopStyleVar();
    ImGui::BeginGroupBox("##body_content");
    {
        if(active_esp_tab == 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::Checkbox("Enabled", g_Options.esp_enabled);
			ImGui::Checkbox("Skeleton", g_Options.esp_bones);
			if (g_Options.esp_bones)
				ImGui::Checkbox("FriendSkele", g_Options.esp_render_team);
            ImGui::Checkbox("Team check", g_Options.esp_enemies_only);
            ImGui::Checkbox("Boxes", g_Options.esp_player_boxes);
            ImGui::Checkbox("Names", g_Options.esp_player_names);
            ImGui::Checkbox("Health", g_Options.esp_player_health);
            ImGui::Checkbox("Armour", g_Options.esp_player_armour);
            ImGui::Checkbox("Weapon", g_Options.esp_player_weapons);
            ImGui::Checkbox("Snaplines", g_Options.esp_player_snaplines);

            ImGui::NextColumn();

            ImGui::Checkbox("Crosshair", g_Options.esp_crosshair);
			ImGui::Checkbox("RecoilCrosshair", g_Options.esp_recoil_crosshair);
            ImGui::Checkbox("Dropped Weapons", g_Options.esp_dropped_weapons);
            ImGui::Checkbox("Defuse Kit", g_Options.esp_defuse_kit);
            ImGui::Checkbox("Planted C4", g_Options.esp_planted_c4);
			ImGui::Checkbox("Item Esp", g_Options.esp_items);

            ImGui::NextColumn();

            ImGui::PushItemWidth(100);
			ImGuiEx::ColorEdit3("SkeletonClr", g_Options.color_skeleton_clr);
            ImGuiEx::ColorEdit3("Allies Visible", g_Options.color_esp_ally_visible);
            ImGuiEx::ColorEdit3("Enemies Visible", g_Options.color_esp_enemy_visible);
            ImGuiEx::ColorEdit3("Allies Occluded", g_Options.color_esp_ally_occluded);
            ImGuiEx::ColorEdit3("Enemies Occluded", g_Options.color_esp_enemy_occluded);
            ImGuiEx::ColorEdit3("Crosshair", g_Options.color_esp_crosshair);
            ImGuiEx::ColorEdit3("Dropped Weapons", g_Options.color_esp_weapons);
            ImGuiEx::ColorEdit3("Defuse Kit", g_Options.color_esp_defuse);
            ImGuiEx::ColorEdit3("Planted C4", g_Options.color_esp_c4);
			ImGuiEx::ColorEdit3("Item Esp", g_Options.color_esp_item);
            ImGui::PopItemWidth();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        } else if(active_esp_tab == 1) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::Checkbox("Enabled", g_Options.glow_enabled);
            ImGui::Checkbox("Team check", g_Options.glow_enemies_only);
            ImGui::Checkbox("Players", g_Options.glow_players);
            ImGui::Checkbox("Chickens", g_Options.glow_chickens);
            ImGui::Checkbox("C4 Carrier", g_Options.glow_c4_carrier);
            ImGui::Checkbox("Planted C4", g_Options.glow_planted_c4);
            ImGui::Checkbox("Defuse Kits", g_Options.glow_defuse_kits);
            ImGui::Checkbox("Weapons", g_Options.glow_weapons);

            ImGui::NextColumn();

            ImGui::PushItemWidth(100);
            ImGuiEx::ColorEdit3("Ally", g_Options.color_glow_ally);
            ImGuiEx::ColorEdit3("Enemy", g_Options.color_glow_enemy);
            ImGuiEx::ColorEdit3("Chickens", g_Options.color_glow_chickens);
            ImGuiEx::ColorEdit3("C4 Carrier", g_Options.color_glow_c4_carrier);
            ImGuiEx::ColorEdit3("Planted C4", g_Options.color_glow_planted_c4);
            ImGuiEx::ColorEdit3("Defuse Kits", g_Options.color_glow_defuse);
            ImGuiEx::ColorEdit3("Weapons", g_Options.color_glow_weapons);
            ImGui::PopItemWidth();

            ImGui::NextColumn();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        } else if(active_esp_tab == 2) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::BeginGroupBox("Players");
            {
                ImGui::Checkbox("Enabled", g_Options.chams_player_enabled); ImGui::SameLine();
                ImGui::Checkbox("Team Check", g_Options.chams_player_enemies_only);
                ImGui::Checkbox("Wireframe", g_Options.chams_player_wireframe);
                ImGui::Checkbox("Flat", g_Options.chams_player_flat);
                ImGui::Checkbox("Ignore-Z", g_Options.chams_player_ignorez); ImGui::SameLine();
                ImGui::Checkbox("Glass", g_Options.chams_player_glass);
                ImGui::PushItemWidth(110);
                ImGuiEx::ColorEdit4("Ally (Visible)", g_Options.color_chams_player_ally_visible);
                ImGuiEx::ColorEdit4("Ally (Occluded)", g_Options.color_chams_player_ally_occluded);
                ImGuiEx::ColorEdit4("Enemy (Visible)", g_Options.color_chams_player_enemy_visible);
                ImGuiEx::ColorEdit4("Enemy (Occluded)", g_Options.color_chams_player_enemy_occluded);
                ImGui::PopItemWidth();
            }
            ImGui::EndGroupBox();

            ImGui::NextColumn();

            ImGui::BeginGroupBox("Arms");
            {
                ImGui::Checkbox("Enabled", g_Options.chams_arms_enabled);
                ImGui::Checkbox("Wireframe", g_Options.chams_arms_wireframe);
				ImGui::Checkbox("WeaponWireframe", g_Options.chams_weapon_wireframe);
                ImGui::Checkbox("Flat", g_Options.chams_arms_flat);
                ImGui::Checkbox("Glass", g_Options.chams_arms_glass);
                ImGui::PushItemWidth(110);
                ImGuiEx::ColorEdit4("Color (Visible)", g_Options.color_chams_arms_visible);
                ImGuiEx::ColorEdit4("Color (Occluded)", g_Options.color_chams_arms_occluded);
                ImGui::PopItemWidth();
            }
            ImGui::EndGroupBox();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        }
    }
    ImGui::EndGroupBox();
}

void RenderMiscTab()
{
    bool placeholder_true = true;

    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::ToggleButton("MISC", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
        ImGui::Columns(3, nullptr, false);
        ImGui::SetColumnOffset(1, group_w / 3.0f);
        ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
        ImGui::SetColumnOffset(3, group_w);

        ImGui::Checkbox("Bunny hop", g_Options.misc_bhop);
		ImGui::Checkbox("Third Person", g_Options.misc_thirdperson);
		if(g_Options.misc_thirdperson)
			ImGui::SliderFloat("Distance", g_Options.misc_thirdperson_dist, 0.f, 150.f);
        ImGui::Checkbox("No hands", g_Options.misc_no_hands);
		ImGui::Checkbox("Rank reveal", g_Options.misc_showranks);
		ImGui::Checkbox("Watermark##hc", g_Options.misc_watermark);

		ImGui::NextColumn();
        ImGui::SliderInt("viewmodel_fov:", g_Options.viewmodel_fov, -120, 120);
		ImGui::SliderInt("viewmodel_x:", g_Options.viewmodel_offset_x, -25, 25);
		ImGui::SliderInt("viewmodel_y:", g_Options.viewmodel_offset_y, -25, 25);
		ImGui::SliderInt("viewmodel_z:", g_Options.viewmodel_offset_z, -25, 25);
		ImGui::Text("Postprocessing:");
        ImGui::SliderFloat("Red", g_Options.mat_ambient_light_r, 0, 1);
        ImGui::SliderFloat("Green", g_Options.mat_ambient_light_g, 0, 1);
        ImGui::SliderFloat("Blue", g_Options.mat_ambient_light_b, 0, 1);

		//ImGui::NextColumn();
		//ImGui::Checkbox("Tracers", g_Options.draw_tracers_enabled);
		//if (g_Options.draw_tracers_enabled) {
		//	ImGui::SliderFloat("Duration", g_Options.draw_tracer_duration, 0, 20);
		//	ImGuiEx::ColorEdit3("TracerHit", g_Options.color_tracer_hit);
		//	ImGuiEx::ColorEdit3("TracerMiss", g_Options.color_tracer_miss);
		//	//ImGuiEx::ColorEdit3("TracerBox", g_Options.color_tracer_impact_box);
		//}
		//ImGui::Checkbox("AGGRESSIVE DUCK", g_Options.misc_DUCK_hitsound);
        //ImGui::PopItemWidth();

        ImGui::Columns(1, nullptr, false);
        ImGui::PopStyleVar();
    }
    ImGui::EndGroupBox();
}

void RenderAimTab()
{
	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

	bool placeholder_true = true;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::ToggleButton("AIM", &placeholder_true, ImVec2{ group_w, 25.0f });
	ImGui::PopStyleVar();

	ImGui::BeginGroupBox("##body_content");
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
		ImGui::Columns(3, nullptr, false);
		ImGui::SetColumnOffset(1, group_w / 3.0f);
		ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
		ImGui::SetColumnOffset(3, group_w);

		ImGui::BeginGroupBox("Aimbot");
		{
			static int selectedValue = -1;
			ImGui::Checkbox("Aimbot Enabled", g_Options.aim_aimbot_enabled); {
				static const char* chars[]{ "Legitbot", "Ragebot", "BTAimbot" };

				if (g_Options.aim_aimbot_enabled)
					ImGui::ListBox("Aimbot Styles", &selectedValue, chars, IM_ARRAYSIZE(chars));

				if (selectedValue == 0) {
					g_Options.aim_legitaimbot_enabled.value = std::make_shared<bool>(true);
					ImGui::Checkbox("Legit Aimbot", g_Options.aim_legitaimbot_enabled);
					if (g_Options.aim_legitaimbot_enabled && selectedValue == 0) {
						g_Options.aim_aimbot_ragebot.value = std::make_shared<bool>(false);
						g_Options.aim_backtrack_aimbot.value = std::make_shared<bool>(false);
						ImGui::Checkbox("silentA", g_Options.aim_aimbot_silentA_enabled);
						ImGui::Checkbox("RCS", g_Options.aim_aimbot_rcs);
						ImGui::SliderFloat("FOV", g_Options.aim_aimbot_fov, 0.f, 3.f);
						ImGui::SliderInt("Smoothing", g_Options.aim_aimbot_smoothing, 0, 20);
					}
				}
				else if (selectedValue == 1) {
					g_Options.aim_aimbot_ragebot.value = std::make_shared<bool>(true);
					ImGui::Checkbox("Ragebot", g_Options.aim_aimbot_ragebot);
					static C_BaseCombatWeapon* currentWeapon;
					if (g_Options.aim_aimbot_ragebot) {
						if (g_LocalPlayer && currentWeapon != g_LocalPlayer->m_hActiveWeapon())
							currentWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
						g_Options.aim_legitaimbot_enabled.value = std::make_shared<bool>(false);
						g_Options.aim_backtrack_aimbot.value = std::make_shared<bool>(false);
						ImGui::Checkbox("RagebotAutoshoot", g_Options.aim_ragebot_autoshoot);
						ImGui::SliderInt("RagebotFov", g_Options.aim_ragebot_fov, 0, 180);
						if (currentWeapon->IsSniper())
							ImGui::Checkbox("Autoscope", g_Options.aim_autoscope);

						if (currentWeapon->IsPistol()) {
							ImGui::Text("Current Weapon Type: Pistol");
							ImGui::SliderInt("MinDmg", g_Options.pistol_mindmg, 0, 100);
							ImGui::SliderFloat("HitChance", g_Options.pistol_hitchance, 0, 100);
						}
						else if (currentWeapon->IsSniper() && currentWeapon->IsAWP()) {
							ImGui::Text("Current Weapon Type: AWP");
							ImGui::SliderInt("MinDmg", g_Options.awp_mindmg, 0, 100);
							ImGui::SliderFloat("HitChance", g_Options.awp_hitchance, 0, 100);
						}
						else if (currentWeapon->IsSniper() && currentWeapon->IsScout()) {
							ImGui::Text("Current Weapon Type: Scout");
							ImGui::SliderInt("MinDmg", g_Options.scout_mindmg, 0, 100);
							ImGui::SliderFloat("HitChance", g_Options.scout_hitchance, 0, 100);
						}
						else if (currentWeapon->IsSniper() && currentWeapon->IsAuto()) {
							ImGui::Text("Current Weapon Type: Autosniper");
							ImGui::SliderInt("MinDmg", g_Options.auto_minDmg, 0, 100);
							ImGui::SliderFloat("HitChance", g_Options.auto_hitchance, 0, 100);
						}
						else if (currentWeapon->IsRifle()) {
							ImGui::Text("Current Weapon Type: Assault Rifle");
							ImGui::SliderInt("MinDmg", g_Options.rifle_mindmg, 0, 100);
							ImGui::SliderFloat("HitChance", g_Options.rifle_hitchance, 0, 100);
						}
						else if (currentWeapon->IsShotgun()) {
							ImGui::Text("Current Weapon Type: Shotgun");
							ImGui::SliderInt("MinDmg", g_Options.shotgun_mindmg, 0, 100);
							ImGui::SliderFloat("HitChance", g_Options.shotgun_hitchance, 0, 100);
						}
						else if (currentWeapon->IsSmg()) {
							ImGui::Text("Current Weapon Type: SMG");
							ImGui::SliderInt("MinDmg", g_Options.smg_mindmg, 0, 100);
							ImGui::SliderFloat("HitChance", g_Options.smg_hitchance, 0, 100);
						}
						else if (currentWeapon->IsLmg()) {
							ImGui::Text("Current Weapon type: LMG");
							ImGui::SliderInt("MinDmg", g_Options.lmg_mindmg, 0, 100);
							ImGui::SliderFloat("HitChance", g_Options.lmg_hitchance, 0, 100);
						}

					}
				}
				else if (selectedValue == 2) {
					g_Options.aim_backtrack_aimbot.value = std::make_shared<bool>(true);
					ImGui::Checkbox("BacktrackAimbot", g_Options.aim_backtrack_aimbot);
					if (g_Options.aim_backtrack_aimbot) {
						g_Options.aim_legitaimbot_enabled.value = std::make_shared<bool>(false);
						g_Options.aim_aimbot_ragebot.value = std::make_shared<bool>(false);
						ImGui::Checkbox("BacktrackAimSilent", g_Options.aim_bt_silent);
						ImGui::Checkbox("BacktrackAimRCS", g_Options.aim_bt_rcs);
						ImGui::Checkbox("BTAutoshoot", g_Options.aim_bt_autoshoot);
						ImGui::SliderInt("BTAimFOV", g_Options.aim_bt_fov, 0, 180);
						ImGui::SliderInt("BTAimSmooth", g_Options.aim_bt_smoothing, 0, 20);
					}
				}
			}
			if (selectedValue != -1 && !g_Options.aim_aimbot_enabled) {
				selectedValue = -1;
				g_Options.aim_legitaimbot_enabled.value = std::make_shared<bool>(false);
				g_Options.aim_aimbot_ragebot.value = std::make_shared<bool>(false);
				g_Options.aim_backtrack_aimbot.value = std::make_shared<bool>(false);
			}
		}
		ImGui::EndGroupBox();
		
		ImGui::NextColumn();
		ImGui::BeginGroupBox("Backtrackkk"); {
			ImGui::Checkbox("Backtrack", g_Options.backtrack_enabled);
			if (g_Options.backtrack_enabled) {
				ImGui::Checkbox("Team Check", g_Options.backtrack_teamcheck);
				ImGui::Checkbox("BacktrackDots", g_Options.backtrack_dots_enabled);
				ImGui::Checkbox("BacktrackChams", g_Options.chams_backtrack_enabled);
				if (g_Options.chams_backtrack_enabled) {
					ImGuiEx::ColorEdit3("BT ColorE", g_Options.color_backtrack_chamsE);
					ImGuiEx::ColorEdit3("BT ColorT", g_Options.color_backtrack_chamsT);
					ImGui::Checkbox("Wireframe", g_Options.backtrack_chams_wireframe);
				}
			}
		}
		ImGui::EndGroupBox();

		ImGui::NextColumn();

		ImGui::BeginGroupBox("Ping/Tick"); {
			ImGui::Checkbox("FakeLag", g_Options.fakelag_enabled);
			if (g_Options.fakelag_enabled) {
				if (g_Options.fakelag == 0.f)
					ImGui::Text("Legit bt");
				else if (g_Options.fakelag == .2f)
					ImGui::Text("400 ms bt");
				else if (g_Options.fakelag == .8f)
					ImGui::Text("1 sec bt");
			}
			if (!g_Options.fakelag_enabled && g_Options.fakelag != 0.f) {
				g_Options.fakelag.value = std::make_shared<float>(0.f);
			}
			ImGui::Checkbox("TickBasedManip", g_Options.tickBaseManip);
			if (g_Options.tickBaseManip) {
				ImGui::Checkbox("ChokePackets", g_Options.chokePackets);
				ImGui::Checkbox("FakeDuck", g_Options.fakeDuck);
				ImGui::Checkbox("FakeStand", g_Options.fakeStand);
				//ImGui::Checkbox("InstantPlant", g_Options.instantPlant);
			}
		}
		ImGui::EndGroupBox();

		ImGui::Columns(3, nullptr, false);
		ImGui::PopStyleVar();
	}
	ImGui::EndGroupBox();
}


void RenderConfigTab()
{
    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    bool placeholder_true = true;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    ImGui::ToggleButton("CONFIG", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
		if (ImGui::Button("Save cfg")) {
			Config::Get().Save();
		}
		if (ImGui::Button("Load cfg")) {
			Config::Get().Load();
		}
    }
    ImGui::EndGroupBox();
}

void Menu::Initialize()
{
	CreateStyle();

    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;

    if(!_visible)
        return;

    const auto sidebar_size = get_sidebar_size();
    static int active_sidebar_tab = 0;

    //ImGui::PushStyle(_style);

    ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2{ 1000, 380 }, ImGuiSetCond_Once);


	if (ImGui::Begin(":' (",
		&_visible,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar)) {

		//auto& style = ImGui::GetStyle();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        {
            ImGui::BeginGroupBox("##sidebar", sidebar_size);
            {
				//ImGui::GetCurrentWindow()->Flags &= ~ImGuiWindowFlags_ShowBorders;

                render_tabs(sidebar_tabs, active_sidebar_tab, get_sidebar_item_width(), get_sidebar_item_height(), false);
            }
            ImGui::EndGroupBox();
        }
        ImGui::PopStyleVar();
        ImGui::SameLine();

        // Make the body the same vertical size as the sidebar
        // except for the width, which we will set to auto
        auto size = ImVec2{ 0.0f, sidebar_size.y };

		ImGui::BeginGroupBox("##body", size);
        if(active_sidebar_tab == TAB_ESP) {
            RenderEspTab();
        } else if(active_sidebar_tab == TAB_AIMBOT) {
            RenderAimTab();
        } else if(active_sidebar_tab == TAB_MISC) {
            RenderMiscTab();
        } else if(active_sidebar_tab == TAB_CONFIG) {
            RenderConfigTab();
        }
        ImGui::EndGroupBox();

        ImGui::TextColored(ImVec4{ 0.0f, 0.5f, 0.0f, 1.0f }, "FPS: %03d", get_fps());
        ImGui::SameLine(ImGui::GetWindowWidth() - 150 - ImGui::GetStyle().WindowPadding.x);
        if(ImGui::Button("Unload", ImVec2{ 150, 25 })) {
            g_Unload = true;
        }
        ImGui::End();
    }
}

void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
	ImGui::StyleColorsDark();
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	_style.FrameRounding = 0.f;
	_style.WindowRounding = 0.f;
	_style.ChildRounding = 0.f;
	_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.000f, 0.545f, 1.000f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.060f, 0.416f, 0.980f, 1.000f);
	_style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.0f);
	_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.009f, 0.120f, 0.940f);
	_style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.000f);
	ImGui::GetStyle() = _style;
}

