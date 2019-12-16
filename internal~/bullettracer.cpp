#include "bullettracer.hpp"
#include "options.hpp"

cbullet_tracer tracer;
void cbullet_tracer::log(IGameEvent* event) {
	if (strstr(event->GetName(), "bullet_impact")) {
		auto index = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));

		if (index == g_EngineClient->GetLocalPlayer()) {

			auto local = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
			if (!local)
				return;

			Vector position(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));
			Ray_t ray;
			ray.Init(local->GetEyePos(), position);

			CTraceFilter filter;
			filter.pSkip = local;

			trace_t trace;
			g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

			auto color = (trace.hit_entity && reinterpret_cast<C_BasePlayer*>(trace.hit_entity)->IsPlayer() || trace.fraction > .97f) ? g_Options.color_tracer_miss : g_Options.color_tracer_hit;

			this->logs.push_back(cbullet_tracer_info(local->GetEyePos(), position, g_GlobalVars->curtime, color));
		}
	}
}

void cbullet_tracer::render() 
{
	if (!g_Options.draw_tracers_enabled)
		return;

	auto local = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(g_EngineClient->GetLocalPlayer()));
	if (!local)
		return;

	Color tempClr = Color(g_Options.color_tracer_impact_box);
	
	for (size_t i = 0; i < this->logs.size(); i++) {
		auto current = this->logs.at(i);

		g_DebugOverlay->AddLineOverlay(current.src, current.dst, current.color.r(), current.color.g(), current.color.b(), true, -1.f);
		
		//g_DebugOverlay->AddBoxOverlay(current.dst, Vector(-2, -2, -2), Vector(2, 2, 2), QAngle(0, 0, 0), tempClr.r(), tempClr.g(), tempClr.b(), 255, -1.f);

		if (fabs(g_GlobalVars->curtime - current.time) > g_Options.draw_tracer_duration)
			this->logs.erase(this->logs.begin() + i);
	}
}