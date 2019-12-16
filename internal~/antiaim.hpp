#pragma once
#include "helpers/utils.hpp"
#include "helpers/math.hpp"
#include "valve_sdk/csgostructs.hpp"

class Antiaim {
public: 
	void slowStep(CUserCmd* pCmd, bool& bSendPackets);
};
extern Antiaim antiaim;