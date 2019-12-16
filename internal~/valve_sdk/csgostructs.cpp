#include "csgostructs.hpp"
#include "../Helpers/Math.hpp"
#include "../Helpers/Utils.hpp"
#include "sdk.hpp"

bool C_BaseEntity::IsPlayer()
{
	//index: 152
	//ref: "effects/nightvision"
	//sig: 8B 92 ? ? ? ? FF D2 84 C0 0F 45 F7 85 F6
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 157)(this);
}

bool C_BaseEntity::IsLoot() {
	return GetClientClass()->m_ClassID == ClassId_CPhysPropAmmoBox ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropLootCrate ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropRadarJammer ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropWeaponUpgrade ||
		GetClientClass()->m_ClassID == ClassId_CDrone ||
		GetClientClass()->m_ClassID == ClassId_CDronegun ||
		GetClientClass()->m_ClassID == ClassId_CItem_Healthshot ||
		GetClientClass()->m_ClassID == ClassId_CItemCash ||
		GetClientClass()->m_ClassID == ClassId_CBumpMine;
}

bool C_BaseEntity::IsWeapon()
{
	//index: 160
	//ref: "CNewParticleEffect::DrawModel"
	//sig: 8B 80 ? ? ? ? FF D0 84 C0 74 6F 8B 4D A4
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 165)(this);
}


bool C_BaseEntity::IsPlantedC4()
{
	return GetClientClass()->m_ClassID == ClassId_CPlantedC4;
}

bool C_BaseEntity::IsDefuseKit()
{
	return GetClientClass()->m_ClassID == ClassId_CBaseAnimating;
}

//bool C_BaseEntity::IsEnemy()
//{
//	auto* LocalPlayer = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
//
//	if (!LocalPlayer)
//		return false;
//
//	if (this->m_iTeamNum() != LocalPlayer->m_iTeamNum())
//		return true;
//
//	if (this->m_iTeamNum() != TEAM_TERRORIST)
//		return false;
//
//	static ConVar* game_type = nullptr;
//	static ConVar* game_mode = nullptr;
//
//	if (!game_type)
//		game_type = g_CVar->FindVar("game_type");
//
//	if (!game_mode)
//		game_mode = g_CVar->FindVar("game_mode");
//
//	if (game_type->GetInt() == 6 && game_mode->GetInt() == 0) // Danger Zone
//	{
//		static c_Baserule* GameRulesProxy = *(CBaseGameRulesProxy * *)offsets.GameRulesProxy;
//		
//		if (!GameRulesProxy)
//			return false;
//
//		CBaseSurvivalGameRules* SurvivalRules = GameRulesProxy->GetSurvivalGameRules();
//
//		if (SurvivalRules->GetPlayerTeam(LocalPlayer->EntIndex()) == -1)
//			return true;
//
//		if (SurvivalRules->GetPlayerTeam(LocalPlayer->EntIndex()) != SurvivalRules->GetPlayerTeam(this->EntIndex()))
//			return true;
//	}
//
//	return false;
//}

CCSWeaponInfo* C_BaseCombatWeapon::GetCSWeaponData()
{
	return CallVFunction<CCSWeaponInfo * (__thiscall*)(void*)>(this, 456)(this);
	/*
	static auto fnGetWpnData
	= reinterpret_cast<CCSWeaponInfo*(__thiscall*)(void*)>(
	Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 81 EC ? ? ? ? 53 8B D9 56 57 8D 8B")
	);
	return fnGetWpnData(this);*/
}

int& C_BaseCombatWeapon::getWeaponId() 
{
	return *(int*)((DWORD)this + 0x2F88 );
}

bool C_BaseCombatWeapon::HasBullets()
{
	return !IsReloading() && m_iClip1() > 0;
}

bool C_BaseCombatWeapon::CanFire()
{
	auto owner = this->m_hOwnerEntity().Get();
	if (!owner)
		return false;

	if (IsReloading() || m_iClip1() <= 0)
		return false;

	auto flServerTime = owner->m_nTickBase() * g_GlobalVars->interval_per_tick;

	if (owner->m_flNextAttack() > flServerTime)
		return false;


	return m_flNextPrimaryAttack() <= flServerTime;
}

bool C_BaseCombatWeapon::IsGrenade()
{
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_GRENADE;
}

bool C_BaseCombatWeapon::IsScout() {
	switch (GetCSWeaponData()->iWeaponPrice)
	{
	case 1700:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsAWP() {
	switch (GetCSWeaponData()->iWeaponPrice)
	{
	case 4750:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsAuto() {
	switch (GetCSWeaponData()->iWeaponPrice)
	{
	case 5000:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsGun()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_C4:
		return false;
	case WEAPONTYPE_GRENADE:
		return false;
	case WEAPONTYPE_KNIFE:
		return false;
	case WEAPONTYPE_UNKNOWN:
		return false;
	default:
		return true;
	}
}

bool C_BaseCombatWeapon::IsKnife()
{
	if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER) return false;
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE;
}

bool C_BaseCombatWeapon::IsRifle()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_RIFLE:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsSmg() {
	switch (GetCSWeaponData()->iWeaponType) {
	case WEAPONTYPE_SUBMACHINEGUN:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsLmg() {
	switch (GetCSWeaponData()->iWeaponType) {
	case WEAPONTYPE_MACHINEGUN:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsShotgun() {
	switch (GetCSWeaponData()->iWeaponType) {
	case WEAPONTYPE_SHOTGUN:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsPistol()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_PISTOL:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsSniper()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_SNIPER_RIFLE:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsReloading()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90") + 2);
	return *(bool*)((uintptr_t)this + inReload);
}

float C_BaseCombatWeapon::GetInaccuracy()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 478)(this);
}

float C_BaseCombatWeapon::GetSpread()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 448)(this);
}

void C_BaseCombatWeapon::UpdateAccuracyPenalty()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 479)(this);
}

CUtlVector<IRefCounted*>& C_BaseCombatWeapon::m_CustomMaterials()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "83 BE ? ? ? ? ? 7F 67") + 2) - 12;
	return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + inReload);
}

bool* C_BaseCombatWeapon::m_bCustomMaterialInitialized()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "C6 86 ? ? ? ? ? FF 50 04") + 2);
	return (bool*)((uintptr_t)this + currentCommand);
}

CUserCmd*& C_BasePlayer::m_pCurrentCommand()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "89 BE ? ? ? ? E8 ? ? ? ? 85 FF") + 2);
	return *(CUserCmd * *)((uintptr_t)this + currentCommand);
}

int C_BasePlayer::GetNumAnimOverlays()
{
	return *(int*)((DWORD)this + 0x298C);
}

AnimationLayer* C_BasePlayer::GetAnimOverlays()
{
	return *(AnimationLayer * *)((DWORD)this + 0x2980);
}

AnimationLayer* C_BasePlayer::GetAnimOverlay(int i)
{
	if (i < 15)
		return &GetAnimOverlays()[i];
	return nullptr;
}

int C_BasePlayer::GetSequenceActivity(int sequence)
{
	auto hdr = g_MdlInfo->GetStudiomodel(this->GetModel());

	if (!hdr)
		return -1;

	// sig for stuidohdr_t version: 53 56 8B F1 8B DA 85 F6 74 55
	// sig for C_BaseAnimating version: 55 8B EC 83 7D 08 FF 56 8B F1 74 3D
	// c_csplayer vfunc 242, follow calls to find the function.
	// Thanks @Kron1Q for merge request
	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "55 8B EC 53 8B 5D 08 56 8B F1 83"));

	return get_sequence_activity(this, hdr, sequence);
}

CCSGOPlayerAnimState* C_BasePlayer::GetPlayerAnimState()
{
	return *(CCSGOPlayerAnimState * *)((DWORD)this + 0x3900);
}

void C_BasePlayer::UpdateAnimationState(CCSGOPlayerAnimState* state, QAngle angle)
{
	static auto UpdateAnimState = Utils::PatternScan(
		GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");

	if (!UpdateAnimState)
		return;

	__asm {
		push 0
	}

	__asm
	{
		mov ecx, state

		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}

void C_BasePlayer::ResetAnimationState(CCSGOPlayerAnimState* state)
{
	using ResetAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}

void C_BasePlayer::CreateAnimationState(CCSGOPlayerAnimState* state)
{
	using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, C_BasePlayer*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, this);
}

void C_BaseEntity::SetOrigin(Vector origin)
{
	using SetAbsOriginFn = void(__thiscall*)(C_BaseEntity*, const Vector & origin);
	static SetAbsOriginFn SetAbsOrigin = (SetAbsOriginFn)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1");

	SetAbsOrigin(this, origin);
}

void C_BaseEntity::SetAngles(const QAngle& angles)
{
	using SetAbsAnglesFn = void(__thiscall*)(C_BaseEntity*, const QAngle & angles);
	static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1");

	SetAbsAngles(this, angles);
}


Vector C_BasePlayer::GetEyePos()
{
	return m_vecOrigin() + m_vecViewOffset();
}

player_info_t C_BasePlayer::GetPlayerInfo()
{
	player_info_t info;
	g_EngineClient->GetPlayerInfo(EntIndex(), &info);
	return info;
}

bool C_BasePlayer::isVisible(C_BasePlayer* local, C_BasePlayer* enemy)
{
	Ray_t ray;
	trace_t tr;
	CTraceFilter filter;
	filter.pSkip = local;

	ray.Init(local->GetEyePos(), enemy->GetBonePos(8));
	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	
	if (tr.fraction > .97f || (tr.DidHit() && reinterpret_cast<C_BasePlayer*>(tr.hit_entity) == enemy) || tr.m_pEnt == enemy || tr.endpos == enemy->GetBonePos(8))
		return true;
	if (tr.fraction > .97f || tr.m_pEnt == enemy)
		return true;
	if (tr.fraction > .97f)
		return true;
	if (tr.IsVisible())
		return true;

	ray.Init(local->GetEyePos(), enemy->GetBonePos(7)); 
	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	if (tr.fraction > .97f || (tr.DidHit() && reinterpret_cast<C_BasePlayer*>(tr.hit_entity) == enemy) || tr.m_pEnt == enemy || tr.endpos == enemy->GetBonePos(7))
		return true;
	if (tr.fraction > .97f || tr.m_pEnt == enemy)
		return true;
	if (tr.fraction > .97f)
		return true;
	if (tr.IsVisible())
		return true;

	ray.Init(local->GetEyePos(), enemy->GetEyePos());
	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	if (tr.fraction > .97f || (tr.DidHit() && reinterpret_cast<C_BasePlayer*>(tr.hit_entity) == enemy) || tr.m_pEnt == enemy)
		return true;
	if (tr.fraction > .97f || tr.m_pEnt == enemy)
		return true;
	if (tr.fraction > .97f)
		return true;
	if (tr.IsVisible())
		return true;

	ray.Init(local->GetEyePos(), enemy->m_vecOrigin());
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	if (tr.fraction > .97f || (tr.DidHit() && reinterpret_cast<C_BasePlayer*>(tr.hit_entity) == enemy) || tr.m_pEnt == enemy)
		return true;
	if (tr.fraction > .97f || tr.m_pEnt == enemy)
		return true;
	if (tr.fraction > .97f)
		return true;
	if (tr.IsVisible())
		return true;

	return false;
}

bool C_BasePlayer::IsAlive()
{
	return m_lifeState() == LIFE_ALIVE;
}

bool C_BasePlayer::IsFlashed()
{
	static auto m_flFlashMaxAlpha = NetvarSys::Get().GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");
	return *(float*)((uintptr_t)this + m_flFlashMaxAlpha - 0x8) > 200.0;
}

bool C_BasePlayer::HasC4()
{
	static auto fnHasC4
		= reinterpret_cast<bool(__thiscall*)(void*)>(
			Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "56 8B F1 85 F6 74 31")
			);

	return fnHasC4(this);
}

Vector C_BasePlayer::GetHitboxPos(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	const model_t* model = this->GetModel();
	if (!model)
		return Vector{};

	auto studio_model = g_MdlInfo->GetStudiomodel(model);
	if (!studio_model)
		return Vector{};

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		if (studio_model) {
			auto temp = studio_model->GetHitboxSet(0);
			if (!temp)
				return Vector{};
			auto hitbox = temp->GetHitbox(hitbox_id);
			if (hitbox) {
				auto
					min = Vector{},
					max = Vector{};

				Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
				Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

				return (min + max) / 2.0f;
			}
		}
	}
	return Vector{};
}

mstudiobbox_t* C_BasePlayer::GetHitbox(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	const model_t* model = this->GetModel();
	if (!model)
		return nullptr;

	auto studio_model = g_MdlInfo->GetStudiomodel(model);
	if (!studio_model)
		return nullptr;

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				return hitbox;
			}
		}
	}
	return nullptr;
}

bool C_BasePlayer::GetHitboxPos(int hitbox, Vector& output)
{
	if (hitbox >= HITBOX_MAX)
		return false;

	const model_t* model = this->GetModel();
	if (!model)
		return false;

	studiohdr_t* studioHdr = g_MdlInfo->GetStudiomodel(model);
	if (!studioHdr)
		return false;

	matrix3x4_t matrix[MAXSTUDIOBONES];
	if (!this->SetupBones(matrix, MAXSTUDIOBONES, 0x100, 0))
		return false;

	mstudiobbox_t* studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(hitbox);
	if (!studioBox)
		return false;

	Vector min, max;

	Math::VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
	Math::VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;

	return true;
}

Vector C_BasePlayer::GetBonePos(int bone)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f)) {
		//return boneMatrix[bone].at(3);
		return Vector{ boneMatrix[bone].GetOrigin()};
	}
	return Vector{};
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, int hitbox)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	auto endpos = player->GetHitboxPos(hitbox);

	ray.Init(GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, const Vector& pos)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(GetEyePos(), pos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

void C_BasePlayer::UpdateClientSideAnimation()
{
	return CallVFunction<void(__thiscall*)(void*)>(this, 221)(this);
}

void C_BasePlayer::InvalidateBoneCache()
{
	static DWORD addr = (DWORD)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");

	*(int*)((uintptr_t)this + 0xA30) = g_GlobalVars->framecount; //we'll skip occlusion checks now
	*(int*)((uintptr_t)this + 0xA28) = 0;//clear occlusion flags

	unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
	*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}

int C_BasePlayer::m_nMoveType()
{
	return *(int*)((uintptr_t)this + 0x25C);
}

QAngle* C_BasePlayer::GetVAngles()
{
	static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
	return (QAngle*)((uintptr_t)this + deadflag + 0x4);
}

void C_BaseAttributableItem::SetGloveModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

void C_BaseViewModel::SendViewModelMatchingSequence(int sequence)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 244)(this, sequence);
}

float_t C_BasePlayer::m_flSpawnTime()
{
	return *(float_t*)((uintptr_t)this + 0xA360);
}