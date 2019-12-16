#include "antiaim.hpp"
#include "options.hpp"

//void Antiaim::slowStep(CUserCmd* pCmd, bool &bSendPackets) {
//	if (g_Options.antiaim_enabled && g_Options.basic_antiaim_enabled) {
//		if (GetAsyncKeyState(VK_MENU)) {
//			auto temp = g_LocalPlayer->GetVAngles();
//			QAngle ang(temp->pitch, temp->yaw, temp->roll);
//
//			auto oldForward = pCmd->forwardmove;
//			auto oldSidemove = pCmd->sidemove;
//
//			pCmd->buttons |= IN_WALK;
//			pCmd->forwardmove = -pCmd->forwardmove;
//			pCmd->sidemove = -pCmd->sidemove;
//			pCmd->upmove = 0;	
//
//			Math::CorrectMovement(ang, pCmd, oldForward, oldSidemove);
//		}
//		
//	}
//}

Antiaim antiaim;