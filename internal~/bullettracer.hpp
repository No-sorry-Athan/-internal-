#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"
class cbullet_tracer
{
	class bullet_impact_listener : public IGameEventListener2 
	{
		void start() {

		}

		void stop() {

		}

		void FireGameEvent(IGameEvent* event) override {

		}

		int GetEventDebugID(void) override {
			return EVENT_DEBUG_ID_INIT;
		}

		void OnStudioRender()
		{

		}
	};
public: 
	void log(IGameEvent* event);
	void render();
private:
	class cbullet_tracer_info {
	public:
		cbullet_tracer_info(Vector src, Vector dst, float time, Color color) {
			this->src = src;
			this->dst = dst;
			this->time = time;
			this->color = color;
		}
		Vector src, dst;
		float time;
		Color color;
	};
	std::vector<cbullet_tracer_info> logs;
	bullet_impact_listener _listener;
}; extern cbullet_tracer tracer;