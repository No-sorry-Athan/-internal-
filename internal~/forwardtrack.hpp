#include "helpers/utils.hpp"
#include "valve_sdk/csgostructs.hpp"
#include <vector>
class Forwardtrack {
public:
	struct EntData
	{
		EntData(Vector headP, int tickCount) {
			headPos = headP;
			tick = tickCount;
		}
		Vector headPos;
		int tick;
		matrix3x4_t boneMatrix[128];

	};
	void ForwardtrackRun(CUserCmd* pCmd);
	void ForwardtrackCalc(CUserCmd* pCmd);
	void ForwardtrackChams(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix);
	void ForwardtrackDots();
private:
	Vector extrapolate(C_BasePlayer* entity, int value);
	std::vector<EntData>entData[64];
};
extern Forwardtrack forwardtrack;