#pragma once
#include "autowall.hpp"
#include "options.hpp"

#define HITGROUP_GENERIC	 0
#define HITGROUP_HEAD		 1
#define HITGROUP_CHEST		 2
#define HITGROUP_STOMACH	 3
#define HITGROUP_LEFTARM	 4
#define HITGROUP_RIGHTARM	 5
#define HITGROUP_LEFTLEG	 6 
#define HITGROUP_RIGHTLEG	 7
#define HITGROUP_GEAR		 10


void TraceLine(Vector& absStart, Vector& absEnd, unsigned int mask, IClientEntity* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	g_EngineTrace->TraceRay(ray, mask, &filter, ptr);
}

void ClipTraceToPlayers(const Vector& absStart, const Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr)
{

	auto* pLocal = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)pLocal->m_hActiveWeapon();
	static DWORD dwAddress = (DWORD)Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");

	if (!dwAddress)
		return;
	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, absEnd
		LEA		ECX, absStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}

bool IsBreakableEntity(IClientEntity* entity) {
	typedef bool(__thiscall * isBreakbaleEntityFn)(IClientEntity*);
	static isBreakbaleEntityFn IsBreakableEntityFn = (isBreakbaleEntityFn)Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 51 56 8B F1 85 F6 74 68");

	if (IsBreakableEntityFn)
	{
		auto backupval = *reinterpret_cast<int*>((uint32_t)entity + 0x280);
		auto className = entity->GetClientClass()->m_pNetworkName;

		if (entity != g_EntityList->GetClientEntity(0))
		{

			if ((className[1] == 'B' && className[9] == 'e' && className[10] == 'S' && className[16] == 'e') || (className[1] != 'B' || className[5] != 'D')) 
			{
				*reinterpret_cast<int*>((uint32_t)entity + 0x280) = 2;
			}
		}

		bool retn = IsBreakableEntityFn(entity);

		*reinterpret_cast<int*>((uint32_t)entity + 0x280) = backupval;

		return retn;
	}
	else
		return false;
}

bool DidHitNonWorldEntity(IClientEntity* hit_entity)
{
	return hit_entity != nullptr && hit_entity == g_EntityList->GetClientEntity(0);
}

bool TraceToExit(Vector& end, trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace)
{
	auto distance = 0.0f;

	while (distance < 90.1f)
	{
		distance += 4.0f;
		end = start + dir * distance;

		auto point_contents = g_EngineTrace->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);
		if (point_contents & MASK_SHOT_HULL && (!(point_contents & CONTENTS_HITBOX)))
			continue;
		auto new_end = end - (dir * 4.0f);

		Ray_t ray;
		ray.Init(end, new_end);
		g_EngineTrace->TraceRay(ray, 0x4600400B, nullptr, exit_trace);

		if (!exit_trace->hit_entity)
			return false;

		if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX)
		{
			CTraceFilter filter;
			filter.pSkip = exit_trace->hit_entity;

			ray.Init(end, start);
			g_EngineTrace->TraceRay(ray, 0x600400B, &filter, exit_trace);

			if ((exit_trace->fraction < 1.0f || exit_trace->allsolid) && !exit_trace->startsolid)
			{
				end = exit_trace->endpos;
				return true;

			}

			continue;
		}


		if (!(exit_trace->fraction < 1.0 || exit_trace->allsolid || exit_trace->startsolid) || exit_trace->startsolid)
		{
			if (exit_trace->hit_entity)
			{
				if (!DidHitNonWorldEntity(enter_trace->hit_entity) && IsBreakableEntity(enter_trace->hit_entity))
					return true;
			}

			continue;
		}

		if (((exit_trace->surface.flags >> 7) & 1) && !((enter_trace->surface.flags >> 7) & 1))
			continue;

		if (exit_trace->plane.normal.Dot(dir) < 1.1f)
		{
			float fraction = exit_trace->fraction * 4.0f;
			end = end - (dir * fraction);

			return true;
		}

	}

	return false;
}

void CalcAngle(Vector src, Vector dst, Vector& angles)
{
	Vector delta = src - dst;
	double hyp = delta.Length2D(); //delta.Length
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.00;

	if (delta.x >= 0.0)
		angles.y += 180.0f;
}

void AngleVectors(const Vector& angles, Vector* forward)
{

	float	sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

bool HandleBulletPenetration(CCSWeaponInfo* wpn_data, FireBulletData& data);

float GetHitgroupDamageMult(int iHitGroup) {
	switch (iHitGroup) {
	case HITGROUP_GENERIC:
		return .5f;
	case HITGROUP_HEAD:
		return 2.f;
	case HITGROUP_CHEST:
		return .5f;
	case HITGROUP_STOMACH:
		return .75f;
	case HITGROUP_LEFTARM:
		return .5f;
	case HITGROUP_RIGHTARM:
		return .5f;
	case HITGROUP_LEFTLEG:
		return .375f;
	case HITGROUP_RIGHTLEG:
		return .375f;
	case HITGROUP_GEAR:
		return .5f;
	default:
		return 1.f;
	}
	return 1.f;
}

void ScaleDamage(int iHitGroup, C_BasePlayer* enemy, float weapon_armour_ratio, float& current_damage) {
	current_damage *= GetHitgroupDamageMult(iHitGroup);
	if (enemy->m_ArmorValue() > 0) {
		if (iHitGroup == HITGROUP_HEAD) {
			if (enemy->m_bHasHelmet())
				current_damage *= (weapon_armour_ratio * .5f);
		}
		else
			current_damage *= (weapon_armour_ratio * .5f);
	}
}


bool SimulateBulletFire(C_BasePlayer* local, C_BaseCombatWeapon* weapon, FireBulletData& data) {
	data.penetrate_count = 4;
	data.trace_length = 0.0f;
	auto* wpn_data = weapon->GetCSWeaponData();

	data.current_damage = (float)wpn_data->iDamage;

	while ((data.penetrate_count > 0) && (data.current_damage >= 1.0f))
	{
		data.trace_length_remaining = wpn_data->flRange - data.trace_length;

		Vector end = data.src + data.direction * data.trace_length_remaining;

		TraceLine(data.src, end, 0x4600400B, local, &data.enter_trace);
		ClipTraceToPlayers(data.src, end + data.direction * 40.f, 0x4600400B, &data.filter, &data.enter_trace);
		if (data.enter_trace.fraction == 1.0f)
			break;
		if ((data.enter_trace.hitgroup <= 8) && (data.enter_trace.hitgroup > 0))
		{
			data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
			data.current_damage *= pow(wpn_data->flRangeModifier, data.trace_length * 0.002);
			ScaleDamage(data.enter_trace.hitgroup, (C_BasePlayer*)data.enter_trace.hit_entity, wpn_data->flArmorRatio, data.current_damage);

			return true;
		}
		if (!g_Options.aim_aimbot_ragebot)
			break;

		if (!HandleBulletPenetration(wpn_data, data))
			break;
	}
	return false;
}


bool HandleBulletPenetration(CCSWeaponInfo* wpn_data, FireBulletData& data) {
	surfacedata_t* enter_surface_data = g_PhysSurface->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;

	data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
	data.current_damage *= pow(wpn_data->flRangeModifier, (data.trace_length * 0.002));

	if ((data.trace_length > 3000.f) || (enter_surf_penetration_mod < 0.1f))
		data.penetrate_count = 0;
	if (data.penetrate_count <= 0)
		return false;
	Vector dummy;
	trace_t trace_exit;
	if (!TraceToExit(dummy, &data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit))
		return false;
	surfacedata_t* exit_surface_data = g_PhysSurface->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;

	float exit_surf_penetration_mod = exit_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;

	if (((data.enter_trace.contents & CONTENTS_GRATE) != 0) || (enter_material == 89) || (enter_material == 71))
	{
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f;
	}
	else
	{
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;
	}
	if (enter_material == exit_material)
	{
		if (exit_material == 87 || exit_material == 85)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)
			combined_penetration_modifier = 2.0f;
	}

	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / wpn_data->flPenetration) * 1.25f);
	float thickness = (trace_exit.endpos - data.enter_trace.endpos).Length();

	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;

	float lost_damage = fmaxf(0.0f, v35 + thickness);

	if (lost_damage > data.current_damage)
		return false;

	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;

	if (data.current_damage < 1.0f)
		return false;
	data.src = trace_exit.endpos;
	data.penetrate_count--;
	return true;
}

Vector bestPoint(C_BasePlayer* entity) {
	static int hitgroupArray[]{ 
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	};
	int bestDamage = 0;
	int bestLocation = -1;
	for (int i = 0; i < ARRAYSIZE(hitgroupArray); i++) {
		float currentDamage = 0;
		if (CanHit(entity->GetHitboxPos(hitgroupArray[i]), &currentDamage)) {
			if ((currentDamage > bestDamage)) {
				bestDamage = currentDamage;
				bestLocation = hitgroupArray[i];
				if (static_cast<int32_t>(bestDamage) >= entity->m_iHealth())
					break;
			}
		}
	}
	if (bestLocation != -1)
		return entity->GetHitboxPos(hitgroupArray[bestLocation]);
	else
		return Vector(0,0,0);
}

bool CanHit(const Vector& point, float* damage_given) {
	auto* local = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
	if (!local)
		return false;
	if (local->m_iHealth() < 0)
		return false;
	if (!local->m_hActiveWeapon())
		return false;
	auto data = FireBulletData(local->GetEyePos());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;

	Vector angles;
	CalcAngle(data.src, point, angles);
	AngleVectors(angles, &data.direction);
	data.direction.Normalized();

	if (SimulateBulletFire(local, local->m_hActiveWeapon(), data)) {
		*damage_given = data.current_damage;
		return true;
	}
	return false;

}