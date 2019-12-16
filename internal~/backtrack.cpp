#include "backtrack.hpp"
#include "helpers/math.hpp"
#include "options.hpp"
#include "hooks.hpp"
#include "imgui/imgui.h"
#include "autowall.hpp"

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )

Backtrack backtrack;
float clamp(float val, float minVal, float maxVal)
{
	if (maxVal < minVal)
		return maxVal;
	else if (val < minVal)
		return minVal;
	else if (val > maxVal)
		return maxVal;
	else
		return val;
}

ConVar* minupdate;
ConVar* maxupdate;
ConVar* updaterate;
ConVar* interprate;
ConVar* cmin;
ConVar* cmax;
ConVar* interp;

float GetLerpTime()
{
	if (!minupdate)
		minupdate = g_CVar->FindVar(("sv_minupdaterate"));
	if (!maxupdate)
		maxupdate = g_CVar->FindVar(("sv_maxupdaterate"));
	if (!updaterate)
		updaterate = g_CVar->FindVar(("cl_updaterate"));
	if (!interprate)
		interprate = g_CVar->FindVar(("cl_interp_ratio"));
	if (!cmin)
		cmin = g_CVar->FindVar(("sv_client_min_interp_ratio"));
	if (!cmax)
		cmax = g_CVar->FindVar(("sv_client_max_interp_ratio"));
	if (!interp)
		interp = g_CVar->FindVar(("cl_interp"));

	auto updateRate = updaterate->GetFloat();
	auto interpRatio = static_cast<float>(interprate->GetInt());
	auto minInterpRatio = cmin->GetFloat();
	auto maxInterpRatio = cmax->GetFloat();
	auto minUpdateRate = static_cast<float>(minupdate->GetInt());
	auto maxUpdateRate = static_cast<float>(maxupdate->GetInt());

	auto clampedUpdateRate = clamp(updateRate, minUpdateRate, maxUpdateRate);
	auto clampedInterpRatio = clamp(interpRatio, minInterpRatio, maxInterpRatio);

	auto lerp = clampedInterpRatio / clampedUpdateRate;

	if (lerp <= interprate->GetFloat())
		lerp = interprate->GetFloat();

	return lerp;
}

ConVar* sv_maxunlag;
bool Backtrack::CheckValidTick(int tick, std::vector<EntityData> &data)
{
	float correct = 0;

	auto* nci = g_EngineClient->GetNetChannelInfo();
	if (nci) {
		correct += nci->GetLatency(FLOW_OUTGOING);
		correct += nci->GetLatency(FLOW_INCOMING);
	}
	correct += GetLerpTime();

	if (!sv_maxunlag)
		sv_maxunlag = g_CVar->FindVar("sv_maxunlag");
	correct = clamp(correct, 0, sv_maxunlag->GetFloat());

	float deltaTime = correct - (g_GlobalVars->realtime - data.at(tick).simulationTime);

	if (fabsf(deltaTime) > 0.2f)
		return true;

	return false;
}

void Backtrack::SetInvalidTicks(C_BaseEntity* ent)
{
	for (uint i = 0; i < entityData[ent->EntIndex()].size(); i++)
	{
		bool isValid = CheckValidTick(i, entityData[ent->EntIndex()]);

		entityData[ent->EntIndex()].at(i).isTickValid = isValid;
	}
}

void Backtrack::BacktrackStart(CUserCmd* pCmd) {
	if (!g_EngineClient->IsConnected())
		return;
	if (!g_EngineClient->IsInGame())
		return;
	auto* local = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
	if (!local)
		return;

	for (int i = 0; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
		if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity == local)
			continue;
		if (g_Options.backtrack_teamcheck && entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			continue;

		auto temp = entity->GetVAngles();
		QAngle ang(temp->pitch, temp->yaw, temp->roll);
		entityData[i].insert(entityData[i].begin(), EntityData(entity->GetBonePos(8), entity->m_flSimulationTime(), entity->m_vecOrigin(), pCmd->tick_count, ang));
		entity->SetupBones(entityData[i].front().boneMatrix, 128, 0x100, g_GlobalVars->curtime);
		SetInvalidTicks(entity);
		if (entityData[i].size() > 64)
				entityData[i].pop_back();
	}
}

void Backtrack::BacktrackCalc(CUserCmd* pCmd) {
	if (!g_EngineClient->IsConnected())
		return;
	if (!g_EngineClient->IsInGame())
		return;
	auto* local = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
	if (!local)
		return;
	auto temp = local->GetVAngles();
	QAngle viewAngle(temp->pitch, temp->yaw, temp->roll);
	viewAngle += local->m_aimPunchAngle() * 2.f;
	int closestPlayer = -1;
	float delta = FLT_MAX;
	for (int i = 0; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
		if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity == local)
			continue;

		if (g_Options.backtrack_teamcheck && entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			continue;

		QAngle ang = Math::CalcAng(local->GetEyePos(), entity->GetEyePos()) - viewAngle;
		float temp = Math::ClampAngles(ang).Length();
		if (temp < delta) {
			delta = temp;
			closestPlayer = i;	
		}
	}

	float bestSimTime = -1;
	if (closestPlayer != -1) {
		for (size_t i = 0; i < entityData[closestPlayer].size(); i++)
		{
			if (!entityData[closestPlayer].at(i).isTickValid)
				continue;

			if (!g_Options.fakelag_enabled)
				if (i > 12)
					break;

			if (g_Options.fakelag_enabled && g_Options.fakelag == .2f)
				if (i > 25)
					break;

			if (g_Options.fakelag_enabled && g_Options.fakelag == .8f)
				if (i < 40)
					continue;

			QAngle ang = Math::CalcAng(local->GetEyePos(), entityData[closestPlayer].at(i).headPos) - viewAngle;
			float temp = Math::ClampAngles(ang).Length();
			if (temp < delta && entityData[closestPlayer].at(i).simulationTime > local->m_flSimulationTime() - 1) {
				bestSimTime = entityData[closestPlayer].at(i).simulationTime;
				delta = temp;
			}
		}

		if (bestSimTime != -1 && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)) {
			pCmd->tick_count = TIME_TO_TICKS(bestSimTime);
		}
		
	}
}

void Backtrack::BactrackDots() {
	if (g_Options.backtrack_dots_enabled) {
		if (!g_EngineClient->IsConnected())
			return;
		if (!g_EngineClient->IsInGame())
			return;
		for (size_t i = 0; i < g_EngineClient->GetMaxClients(); i++)
		{
			auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
			if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity == g_LocalPlayer || !entity->IsAlive())
				continue;

			if (g_Options.backtrack_teamcheck && entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
				continue;

			for (size_t j = 0; j < entityData[entity->EntIndex()].size(); j++) {

				if (entityData[entity->EntIndex()].size() <= 1)
					return;

				if (!entityData[entity->EntIndex()].at(j).isTickValid)
					continue;

				if (!g_Options.fakelag_enabled)
					if (j > 12)
						break;

				if (g_Options.fakelag_enabled && g_Options.fakelag == .2f)
					if (j > 25)
						break;


				if (g_Options.fakelag_enabled && g_Options.fakelag == .8f)
					if (j < 45)
						continue;

				Vector screen;
				g_DebugOverlay->ScreenPosition(entityData[entity->EntIndex()].at(j).headPos, screen);
				g_VGuiSurface->DrawSetColor(Color::White);
				g_VGuiSurface->DrawLine(screen.x, screen.y + 2, screen.x, screen.y - 2);
				g_VGuiSurface->DrawLine(screen.x - 2, screen.y, screen.x + 2, screen.y);
			}
		}
	}
}

void Backtrack::BacktrackChams(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix) {
	if (g_Options.chams_backtrack_enabled && g_Options.backtrack_enabled){
	const auto mdl = info.pModel;
	static IMaterial* mat = g_MatSystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL);
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	if (!mat)
		return;

	mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, g_Options.backtrack_chams_wireframe);
	
	int i = info.entity_index;
	if (i > 64)
		return;

	auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
	if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity == g_LocalPlayer)
		return;

	if (g_Options.backtrack_teamcheck && entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		return;

	Color plrClr = (g_LocalPlayer->m_iTeamNum() == entity->m_iTeamNum()) ? g_Options.color_backtrack_chamsT : g_Options.color_backtrack_chamsE;
	for (size_t j = 0; j < entityData[entity->EntIndex()].size(); j++) {

		if (entityData[entity->EntIndex()].size() <= 1)
			return;

		if (!entityData[entity->EntIndex()].at(j).isTickValid)
			continue;

		if (!g_Options.fakelag_enabled)
			if (j > 12)
				break;

		if (g_Options.fakelag_enabled && g_Options.fakelag == .2f) 
			if (j > 25)
				break;
			
		if (g_Options.fakelag_enabled && g_Options.fakelag == .8f) 
			if (j < 49)
				continue;

		auto record = entityData[entity->EntIndex()].at(j);

		g_MdlRender->ForcedMaterialOverride(mat);
		
		mat->ColorModulate(plrClr.r() / 255.f, plrClr.g() / 255.f, plrClr.b() / 255.f);
		mat->AlphaModulate(.05f); 

		fnDME(g_MdlRender, 0, ctx, state, info, record.boneMatrix);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}
	}
}

void Backtrack::Smooth(QAngle& viewAngle, QAngle& angle, int smoothValue)
{
	angle = Math::ClampAngles((viewAngle + Math::ClampAngles(angle - viewAngle) / smoothValue));
}

void Backtrack::autoshoot(CUserCmd* pCmd) {
	static bool isShooting = false;
	if (g_Options.aim_ragebot_autoshoot && g_LocalPlayer->m_hActiveWeapon()->CanFire()) {
		if (g_LocalPlayer->m_hActiveWeapon()->IsSniper()) {
			pCmd->buttons |= IN_ATTACK;
		}
		else if (g_LocalPlayer->m_hActiveWeapon()->IsPistol()) {
			if (isShooting) {
				pCmd->buttons &= ~IN_ATTACK;
			}
			else {
				pCmd->buttons |= IN_ATTACK;
			}
			isShooting = !isShooting;
		}
		else
			pCmd->buttons |= IN_ATTACK;
	}
}

void Backtrack::BacktrackAimbot(CUserCmd* pCmd, bool bSendPacket) {
	if (!g_LocalPlayer)
		return;
	if (g_LocalPlayer->m_iHealth() < 0)
		return;
	if (!g_Options.backtrack_enabled)
		return;

	auto oldView = pCmd->viewangles;
	auto oldSidemove = pCmd->sidemove;
	auto oldForwardmove = pCmd->forwardmove;

	auto temp = g_LocalPlayer->GetVAngles();
	QAngle viewAngles(temp->pitch, temp->yaw, temp->roll);
	viewAngles += g_LocalPlayer->m_aimPunchAngle() * 2.f;
	C_BasePlayer* bestEntity = nullptr;
	float bestDelta = FLT_MAX;
	for (size_t i = 0; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
		if (!entity || entity->IsDormant() || !entity->IsAlive() || entity == g_LocalPlayer || entity->m_bGunGameImmunity() || entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			continue;

		float delta = Math::ClampAngles((Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), entity->GetEyePos())) - viewAngles)).Length();
		if (delta < bestDelta && delta < g_Options.aim_bt_fov)
			bestDelta = delta, bestEntity = entity; continue;

		float temp = Math::ClampAngles((Math::ClampAngles(Math::CalcAngle(g_LocalPlayer->GetEyePos(), entity->GetEyePos())) - viewAngles)).Length();
		if (temp < bestDelta && temp < g_Options.aim_bt_fov)
			bestDelta = temp, bestEntity = entity;
	}
	
	if (bestEntity != nullptr) {
		int bestTick = -1;
		for (size_t i = 0; i < entityData[bestEntity->EntIndex()].size(); i++) {
			if (entityData[bestEntity->EntIndex()].size() <= 1)
				return;

			if (!entityData[bestEntity->EntIndex()].at(i).isTickValid)
				continue;

			if (!g_Options.fakelag_enabled)
				if (i > 12)
					break;

			if (g_Options.fakelag_enabled && g_Options.fakelag == .2f)
				if (i > 25)
					break;

			if (g_Options.fakelag_enabled && g_Options.fakelag == .8f)
				if (i < 49)
					continue;

			if (!g_LocalPlayer->CanSeePlayer(bestEntity, entityData[bestEntity->EntIndex()].at(i).headPos))
				continue;

			float delta = Math::ClampAngles((Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), entityData[bestEntity->EntIndex()].at(i).headPos)) - viewAngles)).Length();
			if (delta < bestDelta && delta < g_Options.aim_bt_fov)
				bestDelta = delta; bestTick = i; continue;

			float temp = Math::ClampAngles((Math::ClampAngles(Math::CalcAngle(g_LocalPlayer->GetEyePos(), entityData[bestEntity->EntIndex()].at(i).headPos)) - viewAngles)).Length();
			if (temp < bestDelta && temp < g_Options.aim_bt_fov)
				bestDelta = temp; bestTick = i;
		}

		if (bestTick != -1 && g_Options.aim_backtrack_aimbot && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_bt_autoshoot)) {
			if (g_LocalPlayer->m_hActiveWeapon()->IsSniper() && !g_LocalPlayer->m_bIsScoped() && g_Options.aim_bt_autoshoot) {
				pCmd->buttons |= IN_ZOOM;
				pCmd->buttons &= ~IN_ATTACK;
			}
			auto angle = Math::ClampAngles((Math::CalcAng(g_LocalPlayer->GetEyePos(), entityData[bestEntity->EntIndex()].at(bestTick).headPos)));
			if (g_Options.aim_bt_rcs)
				angle -= (g_LocalPlayer->m_aimPunchAngle() * 2.f);
			if (g_Options.aim_bt_smoothing != 0)
				Smooth(viewAngles - g_LocalPlayer->m_aimPunchAngle() * 2.f, angle, g_Options.aim_bt_smoothing);
			if (g_Options.aim_bt_silent) {
				pCmd->viewangles = Math::ClampAngles(angle);
			}
			else {
				g_EngineClient->SetViewAngles(&Math::ClampAngles(angle));
			}
			
			pCmd->tick_count = TIME_TO_TICKS(entityData[bestEntity->EntIndex()].at(bestTick).simulationTime);
			if (g_Options.aim_bt_autoshoot)
				autoshoot(pCmd);
			Math::CorrectMovement(oldView, pCmd, oldForwardmove, oldSidemove);
		}
	}
}

