#pragma once
#include "aimbot.hpp"
#include "helpers/utils.hpp"
#include "options.hpp"
#include "helpers/math.hpp"
#include "autowall.hpp"

Aimbot aimbot;
C_BasePlayer* Aimbot::getEntityClosestToCrosshair()
{
	auto temp = g_LocalPlayer->GetVAngles();
	QAngle viewAngles(temp->pitch, temp->yaw, temp->roll);
	viewAngles += g_LocalPlayer->m_aimPunchAngle() * 2.f;
	Vector ang(viewAngles.pitch, viewAngles.yaw, viewAngles.roll);
	C_BasePlayer* bestEntity = nullptr;
	float bestDelta = FLT_MAX;
	for (int i = 0; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
		if (!entity || entity->IsDormant() || !entity->IsAlive() || entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() || entity == g_LocalPlayer || entity->m_bGunGameImmunity())
			continue;

		if (!entity->isVisible(g_LocalPlayer, entity))
			continue;

		float delta = Math::ClampAngles((Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), entity->GetEyePos())) - viewAngles)).Length();
		if (delta < bestDelta && delta < g_Options.aim_aimbot_fov)
			bestDelta = delta, bestEntity = entity; continue;

		float temp = Math::ClampAngles((Math::ClampAngles(Math::CalcAngle(g_LocalPlayer->GetEyePos(), entity->GetEyePos())) - viewAngles)).Length();
		if (temp < bestDelta && temp < g_Options.aim_aimbot_fov)
			bestDelta = temp, bestEntity = entity;
	}
	return bestEntity;
}

void Aimbot::Smooth(QAngle& viewAngle, QAngle& angle, int smoothValue)
{
	angle = Math::ClampAngles((viewAngle + Math::ClampAngles(angle - viewAngle) / smoothValue));
}

void Aimbot::legitaimb(CUserCmd* pCmd, bool& bSendPackets)
{
	if (g_Options.aim_legitaimbot_enabled)
	{
		if (!g_LocalPlayer)
			return;
		if (g_LocalPlayer->m_iHealth() < 0)
			return;

		bool pSilentWorking = false;
		bool otherAng = false;
		bool btThing = false;

		auto curTick = g_GlobalVars->tickcount;
		auto oldView = pCmd->viewangles;
		auto oldSidemove = pCmd->sidemove;
		auto oldForwardmove = pCmd->forwardmove;

		C_BasePlayer* entity = getEntityClosestToCrosshair();
		if (!entity || entity->m_iHealth() < 0 || entity->IsDormant() || entity->m_bGunGameImmunity() || entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			return;

		auto temp = g_LocalPlayer->GetVAngles();
		QAngle viewAngles(temp->pitch, temp->yaw, temp->roll);

		auto angle = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), entity->GetBonePos(8)));
		auto ang = Math::ClampAngles(Math::CalcAngle(g_LocalPlayer->GetEyePos(), entity->GetBonePos(8)));

		if (g_Options.aim_aimbot_rcs) {
			angle -= g_LocalPlayer->m_aimPunchAngle() * 2.f;
			ang -= g_LocalPlayer->m_aimPunchAngle() * 2.f;

		}

		if (Math::ClampAngles(angle).Length() > FLT_MAX) {
			otherAng = false;
			pSilentWorking = false;
		}


		if (g_Options.aim_aimbot_smoothing != 0) {
			Smooth(viewAngles, angle, g_Options.aim_aimbot_smoothing);
			Smooth(viewAngles, ang, g_Options.aim_aimbot_smoothing);
		}

		if (!g_Options.aim_aimbot_silentA_enabled && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)) {
			g_EngineClient->SetViewAngles(&Math::ClampAngles(angle));
			otherAng = true;
		}

		if (!otherAng && !g_Options.aim_aimbot_silentA_enabled && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)) {
			g_EngineClient->SetViewAngles(&Math::ClampAngles(ang));
			otherAng = false;
		}
			
			
		if (g_Options.aim_aimbot_silentA_enabled && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)) {
			pCmd->viewangles = Math::ClampAngles(angle);
			pSilentWorking = true;
		}

		if (!pSilentWorking && g_Options.aim_aimbot_silentA_enabled && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)) {
			pCmd->viewangles = Math::ClampAngles(ang);
			pSilentWorking = false;
		}
		Math::CorrectMovement(oldView, pCmd, oldForwardmove, oldSidemove);
	}
}

bool Aimbot::HitChance(QAngle angles, C_BasePlayer* ent, float chance)
{
	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return false;

	Vector forward, right, up;
	Vector src = g_LocalPlayer->GetEyePos();
	Math::AngleVectors(angles, forward, right, up);

	int cHits = 0;
	int maxTraces = 150;
	int cNeededHits = static_cast<int>(maxTraces * (chance / 100.f));
	
	weapon->UpdateAccuracyPenalty();
	float weap_spread = weapon->GetSpread();
	float weap_inaccuracy = weapon->GetInaccuracy();

	for (int i = 0; i < maxTraces; i++)
	{
		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, 2.f * PI_F);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, 2.f * PI_F);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->getWeaponId() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewAnglesSpread;
		Math::VectorAngles(direction, up, viewAnglesSpread);
		Math::NormalizeAngles(viewAnglesSpread);

		Vector viewForward;
		Math::AngleVectors(viewAnglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = src + (viewForward * weapon->GetCSWeaponData()->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init(src, viewForward);
		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, ent, &tr);

		if (tr.hit_entity == ent)
			++cHits;

		if (static_cast<int>((static_cast<float>(cHits) / maxTraces) * 100.f) >= chance)
			return true;

		if ((maxTraces - i + cHits) < cNeededHits)
			return false;
	}
	return false;
}

void Aimbot::autoshoot(CUserCmd* pCmd) {
	static bool isShooting = false;
	if (g_Options.aim_ragebot_autoshoot && g_LocalPlayer->m_hActiveWeapon()->CanFire()) {
		if (g_LocalPlayer->m_hActiveWeapon()->IsKnife() || g_LocalPlayer->m_hActiveWeapon()->IsGrenade())
			return;
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

void Aimbot::autostop(CUserCmd* pCmd) {
	static bool stopping = false;
	static float tempForward, tempSide;
	if (!stopping)
	{
		stopping = true;
		pCmd->forwardmove = -pCmd->forwardmove;
		pCmd->sidemove = -pCmd->sidemove;
		tempForward = pCmd->forwardmove;
		tempSide = pCmd->sidemove;
	}
	else
	{
		pCmd->forwardmove = tempForward;
		pCmd->sidemove = tempSide;
	}

	if (g_LocalPlayer->m_vecVelocity().Length2D() < 20.f)
	{
		pCmd->forwardmove = 0;
		pCmd->sidemove = 0;
		stopping = false;
	}
}

void Aimbot::ragebot(CUserCmd* pCmd, bool& bSendPackets) {
	if (g_Options.aim_aimbot_ragebot) {
		if (!g_LocalPlayer)
			return;
		if (g_LocalPlayer->m_iHealth() < 0)
			return;

		auto oldView = pCmd->viewangles;
		auto oldSidemove = pCmd->sidemove;
		auto oldForwardmove = pCmd->forwardmove;
		auto temp = g_LocalPlayer->GetVAngles();

		QAngle viewAngles(temp->pitch, temp->yaw, temp->roll);
		viewAngles += g_LocalPlayer->m_aimPunchAngle() * 2.f;

		C_BasePlayer* bestEntity = nullptr;
		float bestDelta = FLT_MAX;

		for (int i = 0; i < g_EngineClient->GetMaxClients(); i++)
		{
			auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
			if (!entity || entity->IsDormant() || entity->m_iHealth() < 0 || entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() || entity == g_LocalPlayer || entity->m_bGunGameImmunity())
				continue;

			float delta = Math::ClampAngles((Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), entity->GetEyePos())) - viewAngles)).Length();
			if (delta < bestDelta && delta < g_Options.aim_ragebot_fov)
				bestDelta = delta, bestEntity = entity; continue;

			float temp = Math::ClampAngles((Math::ClampAngles(Math::CalcAngle(g_LocalPlayer->GetEyePos(), entity->GetEyePos())) - viewAngles)).Length();
			if (temp < bestDelta && temp < g_Options.aim_ragebot_fov)
				bestDelta = temp, bestEntity = entity;
		}
		float damageGiven = 0;
		if (bestEntity != nullptr) {
			Vector temp = bestPoint(bestEntity);
			if ( CanHit(temp, &damageGiven)) {
				auto currentWeapon = g_LocalPlayer->m_hActiveWeapon().Get();
				if (currentWeapon->IsPistol()) {
					if (damageGiven >= g_Options.pistol_mindmg && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_ragebot_autoshoot)) {
						if (HitChance(Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f), bestEntity, g_Options.pistol_hitchance)) {
							pCmd->viewangles = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f);
							if (g_Options.aim_ragebot_autoshoot)
								autoshoot(pCmd);
						}
					}
				}
				else if (currentWeapon->IsSniper() && currentWeapon->IsAWP()) {
					if (damageGiven >= g_Options.awp_mindmg && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_ragebot_autoshoot)) {
						if (!g_LocalPlayer->m_bIsScoped() && g_Options.aim_autoscope)
							pCmd->buttons |= IN_ZOOM, pCmd->buttons &= ~IN_ATTACK;
						if (HitChance(Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f), bestEntity, g_Options.awp_hitchance)) {
							pCmd->viewangles = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f);
							if (g_Options.aim_ragebot_autoshoot)
								autoshoot(pCmd);
						}
					}
				}
				else if (currentWeapon->IsSniper() && currentWeapon->IsScout()) {
					if (damageGiven >= g_Options.scout_mindmg && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_ragebot_autoshoot)) {
						if (!g_LocalPlayer->m_bIsScoped() && g_Options.aim_autoscope)
							pCmd->buttons |= IN_ZOOM, pCmd->buttons &= ~IN_ATTACK;
						if (HitChance(Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f), bestEntity, g_Options.scout_hitchance)) {
							pCmd->viewangles = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f);
							if (g_Options.aim_ragebot_autoshoot)
								autoshoot(pCmd);
						}
					}
				}
				else if (currentWeapon->IsSniper() && currentWeapon->IsAuto()) {
					if (damageGiven >= g_Options.auto_minDmg && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_ragebot_autoshoot)) {
						if (!g_LocalPlayer->m_bIsScoped() && g_Options.aim_autoscope)
							pCmd->buttons |= IN_ZOOM, pCmd->buttons &= ~IN_ATTACK;
						if (HitChance(Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f), bestEntity, g_Options.auto_hitchance)) {
							pCmd->viewangles = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f);
							if (g_Options.aim_ragebot_autoshoot)
								autoshoot(pCmd);
						}
					}
				}
				else if (currentWeapon->IsRifle()) {
					if (damageGiven >= g_Options.rifle_mindmg && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_ragebot_autoshoot)) {
						if (HitChance(Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f), bestEntity, g_Options.rifle_hitchance)) {
							pCmd->viewangles = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f);
							if (g_Options.aim_ragebot_autoshoot)
								autoshoot(pCmd);
						}
					}
				}
				else if (currentWeapon->IsShotgun()) {
					if (damageGiven >= g_Options.shotgun_mindmg && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_ragebot_autoshoot)) {
						if (HitChance(Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f), bestEntity, g_Options.shotgun_hitchance)) {
							pCmd->viewangles = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f);
							if (g_Options.aim_ragebot_autoshoot)
								autoshoot(pCmd);
						}
					}
				}
				else if (currentWeapon->IsSmg()) {
					if (damageGiven >= g_Options.smg_mindmg && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_ragebot_autoshoot)) {
						if (HitChance(Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f), bestEntity, g_Options.smg_hitchance)) {
							pCmd->viewangles = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f);
							if (g_Options.aim_ragebot_autoshoot)
								autoshoot(pCmd);
						}
					}
				}
				else if (currentWeapon->IsLmg()) {
					if (damageGiven >= g_Options.lmg_mindmg && (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2 || g_Options.aim_ragebot_autoshoot)) {
						if (HitChance(Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f), bestEntity, g_Options.lmg_hitchance)) {
							pCmd->viewangles = Math::ClampAngles(Math::CalcAng(g_LocalPlayer->GetEyePos(), temp) - g_LocalPlayer->m_aimPunchAngle() * 2.f);
							if (g_Options.aim_ragebot_autoshoot)
								autoshoot(pCmd);
						}
					}
				}
				Math::CorrectMovement(oldView, pCmd, oldForwardmove, oldSidemove);
			}
		}
	}
}
