#include "forwardtrack.hpp"
#include "options.hpp"
#include "helpers/math.hpp"
#include "hooks.hpp"

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TIME_TO_TICKS( dt )		((int)( 0.5f + (float)(dt) / TICK_INTERVAL ))
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )

Forwardtrack forwardtrack;
void Forwardtrack::ForwardtrackRun(CUserCmd* pCmd) {
	auto* local = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
	if (!local)
		return;

	for (size_t i = 0; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
		if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity == local || !entity->IsPlayer() || !entity->IsAlive())
			continue;
		auto temp = entity->GetVAngles();
		QAngle ang(temp->pitch, temp->yaw, temp->roll);
		Vector futureHead = extrapolate(entity, TIME_TO_TICKS(g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_INCOMING)));
		int tickDifference = TIME_TO_TICKS(g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_INCOMING));
		entData[i].insert(entData[i].begin(), EntData(futureHead, tickDifference));
		entity->SetupBones(entData[entity->EntIndex()].front().boneMatrix, 128, 256, g_GlobalVars->tickcount + tickDifference);
		if (entData[i].size() > 13)
			entData[i].pop_back();
	}
}

void Forwardtrack::ForwardtrackCalc(CUserCmd* pCmd) {
	auto* local = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
	if (!local)
		return;

	auto temp = local->GetVAngles();
	QAngle viewAngle(temp->pitch, temp->yaw, temp->roll);
	viewAngle += local->m_aimPunchAngle() * 2.f;

	int closestPlayer = -1;
	float delta = FLT_MAX;

	for (size_t i = 0; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
		if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity == local || !entity->IsPlayer() || !entity->IsAlive())
			continue;
		QAngle ang = Math::CalcAng(local->GetEyePos(), entity->GetEyePos()) - viewAngle;
		float temp = Math::ClampAngles(ang).Length();
		if (temp < delta) {
			delta = temp;
			closestPlayer = i;
		}
	}

	int index = -1;
	if (closestPlayer != -1) {

		for (size_t i = 0; i < entData[closestPlayer].size(); i++)
		{
			QAngle ang = Math::CalcAng(local->GetEyePos(), entData[closestPlayer].at(i).headPos) - viewAngle;
			float temp = Math::ClampAngles(ang).Length();
			if (temp < delta) {
				delta = temp;
				index = i;
			}
		}

		if (index != -1 && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)) 
			pCmd->tick_count = entData[closestPlayer].at(index).tick;
		
	}
}


void Forwardtrack::ForwardtrackDots() {
	for (size_t i = 0; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
		if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity == g_LocalPlayer || !entity->IsPlayer() || !entity->IsAlive())
			continue;

		for (size_t j = 0; j < entData[entity->EntIndex()].size(); j++) {
			Vector screen;
			g_DebugOverlay->ScreenPosition(entData[entity->EntIndex()].at(j).headPos, screen);
			g_VGuiSurface->DrawSetColor(Color::White);
			g_VGuiSurface->DrawLine(screen.x, screen.y + 2, screen.x, screen.y - 2);
			g_VGuiSurface->DrawLine(screen.x - 2, screen.y, screen.x + 2, screen.y);
		}
	}
}

//void Forwardtrack::ForwardtrackChams(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix) {
//
//	static IMaterial* mat = g_MatSystem->FindMaterial("reflective_normal", TEXTURE_GROUP_MODEL);
//	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);
//
//	if (!mat)
//		return;
//
//	mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
//
//	mat->AlphaModulate(g_Options.Forwardtrack_chams_alpha);
//
//	const auto i = info.entity_index;
//	if (i > 64)
//		return;
//
//	auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
//	if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity == g_LocalPlayer || !entity->IsPlayer() || !entity->IsAlive())
//		return;
//
//	Color temp = (entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() ? g_Options.color_Forwardtrack_chamsT : g_Options.color_Forwardtrack_chamsE);
//	g_RenderView->SetColorModulation(temp.r(), temp.g(), temp.b());
//	g_MdlRender->ForcedMaterialOverride(mat);
//	if (g_Options.chams_Forwardtrack_enabled && g_Options.Forwardtrack_enabled) {
//		for (size_t j = 0; j < entData[entity->EntIndex()].size(); j++) {
//			if (entData[entity->EntIndex()].size() <= 1)
//				return;
//
//			auto record = entData[entity->EntIndex()].at(j);
//			g_RenderView->SetBlend(.35);
//			fnDME(g_MdlRender, 0, ctx, state, info, record.boneMatrix);
//		}
//		g_MdlRender->ForcedMaterialOverride(nullptr);
//	}
//	else {
//		if (entData[entity->EntIndex()].size() <= 1)
//			return;
//		auto record = entData[entity->EntIndex()].back();
//		g_RenderView->SetBlend(.35);
//		fnDME(g_MdlRender, 0, ctx, state, info, record.boneMatrix);
//		g_MdlRender->ForcedMaterialOverride(nullptr);
//	}
//}

Vector Forwardtrack::extrapolate(C_BasePlayer* entity, int value) {
	return entity->m_vecOrigin() + (entity->m_vecVelocity() * (g_GlobalVars->tickcount * (float)value));
}
