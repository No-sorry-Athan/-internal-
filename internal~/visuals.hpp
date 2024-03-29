#pragma once
#include "singleton.hpp"
#include "render.hpp" 
#include "helpers/math.hpp"
#include "valve_sdk/csgostructs.hpp"

class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;
	CRITICAL_SECTION cs;
	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct{
			C_BasePlayer* pl;
			bool is_enemy;
			bool is_visible;
			Color clr;
			Vector head_pos;
			Vector feet_pos;
			RECT bbox;
		}ctx;

		bool Begin(C_BasePlayer* pl);
		void RenderBox();
		void RenderName();
		void RenderWeaponName();
		void RenderHealth();
		void RenderArmour();
		void RenderSnapline();
	};

	void RenderCrosshair();
	void RenderWeapon(C_BaseCombatWeapon* weaponEnt);
	void RenderDefuseKit(C_BaseEntity* defuserEnt);
	void RenderPlantedC4(C_BaseEntity* c4ent);
	void RenderItemEsp(C_BaseEntity* itemEnt);
	void DrawBones(C_BasePlayer* ent);
	void ThirdPerson();
public:
	void AddToDrawList();
	void Render();
};