#pragma once

#include "valve_sdk/csgostructs.hpp"
#include <vector>

class Backtrack {
public:
	struct EntityData
	{
		EntityData(Vector headP, float SimTime, Vector ori, int tickCount, QAngle Ang) {
			headPos = headP;
			simulationTime = SimTime;
			origin = ori;
			tick = tickCount;
			ang = Ang;
		}
		Vector headPos;
		float simulationTime;
		Vector origin;
		QAngle ang;
		int tick;
		matrix3x4_t boneMatrix[128];
		bool isTickValid;
	};
	void BacktrackStart(CUserCmd* cmd);
	void BacktrackCalc(CUserCmd* pCmd);
	void BactrackDots();
	void BacktrackAimbot(CUserCmd* pCmd, bool bSendPacket);
	void BacktrackChams(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix);
	
private:
	void Smooth(QAngle& viewAngle, QAngle& angle, int smoothValue);
	void autoshoot(CUserCmd* pCmd);
	void SetInvalidTicks(C_BaseEntity* ent);
	bool CheckValidTick(int tick, std::vector<EntityData> &data);
	std::vector<EntityData>entityData[64];

};extern Backtrack backtrack;