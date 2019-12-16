#include "chams.hpp"
#include <fstream>

#include "valve_sdk/csgostructs.hpp"
#include "options.hpp"
#include "hooks.hpp"
#include "helpers/input.hpp"
#include "backtrack.hpp"

Chams::Chams()
{
	materialRegular = g_MatSystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL);
	materialFlat = g_MatSystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL);
}

Chams::~Chams()
{
}


void Chams::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba)
{
	IMaterial* material = nullptr;

	if (flat) {
		if (ignoreZ)
			material = materialFlatIgnoreZ;
		else
			material = materialFlat;
	}
	else {
		if (ignoreZ)
			material = materialRegularIgnoreZ;
		else
			material = materialRegular;
	}


	if (glass) {
		material = materialFlat;
		material->AlphaModulate(0.45f);
	}
	else {
		material->AlphaModulate(
			rgba.a() / 255.0f);
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

	g_MdlRender->ForcedMaterialOverride(material);
}


void Chams::OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	const auto mdl = info.pModel;

	bool is_arm = strstr(mdl->szName, "arms") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
	bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;

	if (is_player && g_Options.chams_player_enabled) {
		auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

		if (ent && g_LocalPlayer && ent->IsAlive()) {
			const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
			if (!enemy && g_Options.chams_player_enemies_only)
				return;

			const auto clr_front = enemy ? g_Options.color_chams_player_enemy_visible : g_Options.color_chams_player_ally_visible;
			const auto clr_back = enemy ? g_Options.color_chams_player_enemy_occluded : g_Options.color_chams_player_ally_occluded;

			if (g_Options.chams_player_ignorez) {
				OverrideMaterial(
					true,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					false,
					clr_back);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(
					false,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					false,
					clr_front);
			}
			else {
				OverrideMaterial(
					false,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					g_Options.chams_player_glass,
					clr_front);
			}
		}
	}
	else if (is_sleeve && g_Options.chams_arms_enabled) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		g_MdlRender->ForcedMaterialOverride(material);
	}
	else if (is_arm) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		if (g_Options.misc_no_hands) {
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}
		else if (g_Options.chams_arms_enabled) {
			if (g_Options.chams_arms_ignorez) {
				OverrideMaterial(
					true,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					false,
					g_Options.color_chams_arms_occluded);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(
					false,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					false,
					g_Options.color_chams_arms_visible);
			}
			else {
				OverrideMaterial(
					false,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					g_Options.chams_arms_glass,
					g_Options.color_chams_arms_visible);
			}
		}
	}
	else if (is_weapon && g_Options.chams_weapon_wireframe && g_Options.chams_arms_enabled) {
		IMaterial* mat_T = g_MatSystem->FindMaterial("models/gibs/glass/glass", TEXTURE_GROUP_OTHER, true);
		if (!mat_T)
			return;
		if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
			return;
		if (!g_LocalPlayer)
			return;

		mat_T->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
		mat_T->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
		mat_T->IncrementReferenceCount();
		g_RenderView->SetBlend(1.f);
		
		g_MdlRender->ForcedMaterialOverride(mat_T);
		g_RenderView->SetColorModulation(1, 1, 1);
		fnDME(g_MdlRender, 0, ctx, state, info, matrix);
		
		static auto health = g_LocalPlayer->m_iHealth();
		static auto green = int(health * 2.55f);
		static auto red = 255 - green;
		
		static Color clr(red, green, 0);
		if (health != g_LocalPlayer->m_iHealth()) {
			health = g_LocalPlayer->m_iHealth();
			green = int(health * 2.55f);
			red = 255 - green;
			clr = Color(red, green, 0);
		}
		
		OverrideMaterial(false, false, g_Options.chams_weapon_wireframe, false, clr);
		fnDME(g_MdlRender, 0, ctx, state, info, matrix);
	}
}