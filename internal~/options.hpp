#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "valve_sdk/Misc/Color.hpp"

#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}

template <typename T = bool>
class Var {
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) {
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T* () { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};

class Options
{
public:
	// 
	// ESP
	// 
	OPTION(bool, esp_enabled, false);
	OPTION(bool, esp_enemies_only, false);
	OPTION(bool, esp_player_boxes, false);
	OPTION(bool, esp_player_names, false);
	OPTION(bool, esp_player_health, false);
	OPTION(bool, esp_player_armour, false);
	OPTION(bool, esp_player_weapons, false);
	OPTION(bool, esp_player_snaplines, false);
	OPTION(bool, esp_crosshair, false);
	OPTION(bool, esp_recoil_crosshair, false);
	OPTION(bool, esp_dropped_weapons, false);
	OPTION(bool, esp_defuse_kit, false);
	OPTION(bool, esp_planted_c4, false);
	OPTION(bool, esp_items, false);
	OPTION(bool, esp_bones, false);
	OPTION(bool, esp_render_team, false);

	// 
	// GLOW
	// 
	OPTION(bool, glow_enabled, false);
	OPTION(bool, glow_enemies_only, false);
	OPTION(bool, glow_players, false);
	OPTION(bool, glow_chickens, false);
	OPTION(bool, glow_c4_carrier, false);
	OPTION(bool, glow_planted_c4, false);
	OPTION(bool, glow_defuse_kits, false);
	OPTION(bool, glow_weapons, false);

	//
	// CHAMS
	//
	OPTION(bool, chams_player_enabled, false);
	OPTION(bool, chams_player_enemies_only, false);
	OPTION(bool, chams_player_wireframe, false);
	OPTION(bool, chams_player_flat, false);
	OPTION(bool, chams_player_ignorez, false);
	OPTION(bool, chams_player_glass, false);
	OPTION(bool, chams_arms_enabled, false);
	OPTION(bool, chams_arms_wireframe, false);
	OPTION(bool, chams_arms_flat, false);
	OPTION(bool, chams_arms_ignorez, false);
	OPTION(bool, chams_arms_glass, false);
	OPTION(bool, chams_backtrack_enabled, false);
	OPTION(bool, chams_weapon_wireframe, false);

	//
	// Legit Bot
	//
	OPTION(bool, aim_aimbot_enabled, false);
	OPTION(bool, aim_legitaimbot_enabled, false);
	OPTION(bool, aim_aimbot_silentA_enabled, false);
	OPTION(bool, aim_aimbot_rcs, true);
	OPTION(float, aim_aimbot_fov, 1.f);
	OPTION(int, aim_aimbot_smoothing, 5);

	//
	// Backtrack
	//
	OPTION(bool, backtrack_enabled, true);
	OPTION(bool, backtrack_teamcheck, true);
	OPTION(bool, backtrack_chams_wireframe, true);
	OPTION(bool, backtrack_dots_enabled, false);
	// ~Backtrack Aimbot
	OPTION(bool, aim_backtrack_aimbot, false);
	OPTION(bool, aim_bt_rcs, true);
	OPTION(bool, aim_bt_silent, false);
	OPTION(int, aim_bt_fov, 180);
	OPTION(int, aim_bt_smoothing, 5);
	OPTION(bool, aim_bt_autoshoot, false);
	

	//
	// Rage Bot
	//
	OPTION(bool, aim_aimbot_ragebot, false);
	OPTION(int, aim_ragebot_fov, 180);
	OPTION(bool, aim_ragebot_autoshoot, true);

	// PISTOLS
	OPTION(int, pistol_mindmg, 10);
	OPTION(float, pistol_hitchance, 75);

	// RIFLES
	OPTION(int, rifle_mindmg, 25);
	OPTION(float, rifle_hitchance, 75);

	// SHOTGUNS
	OPTION(int, shotgun_mindmg, 40);
	OPTION(float, shotgun_hitchance, 10);

	// SMGS 
	OPTION(int, smg_mindmg, 10);
	OPTION(float, smg_hitchance, 35);

	// LMGS
	OPTION(int, lmg_mindmg, 30);
	OPTION(float, lmg_hitchance, 40);

	// ~Snipers~
	OPTION(bool, aim_autoscope, true);

	// SCOUT
	OPTION(int, scout_mindmg, 40);
	OPTION(float, scout_hitchance, 75);

	// AWP
	OPTION(int, awp_mindmg, 70);
	OPTION(float, awp_hitchance, 80);

	// AUTOSNIPER
	OPTION(int, auto_minDmg, 55);
	OPTION(float, auto_hitchance, 80);

	//
	// Tick-related
	//
	OPTION(bool, fakelag_enabled, false);
	OPTION(float, fakelag, 0);
	OPTION(bool, chokePackets, false);
	OPTION(bool, fakeDuck, false);
	OPTION(bool, fakeStand, false);
	OPTION(bool, tickBaseManip, false);
	OPTION(bool, instantPlant, false);

	//
	// MISC
	//
	OPTION(bool, misc_bhop, false);
	OPTION(bool, misc_no_hands, false);
	OPTION(bool, misc_thirdperson, false);
	OPTION(bool, misc_showranks, true);
	OPTION(bool, misc_watermark, true);
	OPTION(float, misc_thirdperson_dist, 50.f);
	OPTION(int, viewmodel_fov, 68);
	OPTION(int, viewmodel_offset_x, 3);
	OPTION(int, viewmodel_offset_y, 2);
	OPTION(int, viewmodel_offset_z, -2);
	OPTION(float, mat_ambient_light_r, 0.0f);
	OPTION(float, mat_ambient_light_g, 0.0f);
	OPTION(float, mat_ambient_light_b, 0.0f);
	OPTION(bool, misc_DUCK_hitsound, false);
	OPTION(bool, draw_tracers_enabled, false);
	OPTION(float, draw_tracer_duration, 5.f);

	// 
	// COLORS
	// 
	OPTION(Color, color_skeleton_clr, Color(255, 255, 255));
	OPTION(Color, color_esp_ally_visible, Color(0, 128, 255));
	OPTION(Color, color_esp_enemy_visible, Color(255, 0, 0));
	OPTION(Color, color_esp_ally_occluded, Color(0, 128, 255));
	OPTION(Color, color_esp_enemy_occluded, Color(255, 0, 0));
	OPTION(Color, color_esp_crosshair, Color(255, 255, 255));
	OPTION(Color, color_esp_weapons, Color(128, 0, 128));
	OPTION(Color, color_esp_defuse, Color(0, 128, 255));
	OPTION(Color, color_esp_c4, Color(255, 255, 0));
	OPTION(Color, color_esp_item, Color(255, 255, 255));

	OPTION(Color, color_glow_ally, Color(0, 128, 255));
	OPTION(Color, color_glow_enemy, Color(255, 0, 0));
	OPTION(Color, color_glow_chickens, Color(0, 128, 0));
	OPTION(Color, color_glow_c4_carrier, Color(255, 255, 0));
	OPTION(Color, color_glow_planted_c4, Color(128, 0, 128));
	OPTION(Color, color_glow_defuse, Color(255, 255, 255));
	OPTION(Color, color_glow_weapons, Color(255, 128, 0));

	OPTION(Color, color_chams_player_ally_visible, Color(0, 128, 255));
	OPTION(Color, color_chams_player_ally_occluded, Color(0, 255, 128));
	OPTION(Color, color_chams_player_enemy_visible, Color(255, 0, 0));
	OPTION(Color, color_chams_player_enemy_occluded, Color(255, 128, 0));
	OPTION(Color, color_chams_arms_visible, Color(0, 128, 255));
	OPTION(Color, color_chams_arms_occluded, Color(0, 128, 255));
	OPTION(Color, color_watermark, Color(255, 255, 255)); 

	OPTION(Color, color_tracer_hit, Color(255, 0, 0));
	OPTION(Color, color_tracer_miss, Color(0, 130, 255));
	OPTION(Color, color_tracer_impact_box, Color(255, 255, 255));

	OPTION(Color, color_backtrack_chamsT, Color(255, 255, 255));
	OPTION(Color, color_backtrack_chamsE, Color(255, 255, 255));
};

inline Options g_Options;
inline bool   g_Unload;