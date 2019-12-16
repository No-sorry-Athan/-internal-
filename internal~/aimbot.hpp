#pragma once
#include "singleton.hpp"
#include "helpers/math.hpp"
#include "valve_sdk/csgostructs.hpp"

class Aimbot
{
public:
	void legitaimb(CUserCmd* pCmd, bool &bSendPackets);
	void ragebot(CUserCmd* pCmd, bool& bSendPackets);
private:
	C_BasePlayer* getEntityClosestToCrosshair();
	void Smooth(QAngle& viewAngle, QAngle& angle, int smoothValue);
	bool HitChance(QAngle ang, C_BasePlayer* ent, float chance);
	void autoshoot(CUserCmd* pCmd);
	void autostop(CUserCmd* pCmd);
};
extern Aimbot aimbot;
