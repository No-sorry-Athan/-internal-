#pragma once
#pragma comment(lib, "winmm.lib")
#include "valve_sdk/csgostructs.hpp"
#include "helpers/utils.hpp"
#include "bullettracer.hpp"
#include "options.hpp"



class EventListener : public IGameEventListener2
{
public:
	EventListener()
	{
		g_GameEvents->AddListener(this, "bullet_impact", false);
		g_GameEvents->AddListener(this, "player_hurt", false);
	}
	~EventListener()
	{
		g_GameEvents->RemoveListener(this);
	}

	virtual void FireGameEvent(IGameEvent* event)
	{
		if (g_Options.draw_tracers_enabled)
			if (strstr(event->GetName(), "bullet_impact"))
				tracer.log(event);
		if (g_Options.misc_DUCK_hitsound)
			if (strstr(event->GetName(), "player_hurt")) {
				int attacker = event->GetInt("attacker");
				if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer()) {
					auto pAttacker = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt(("attacker")))));
					auto pLocal = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());
					if (!pLocal)
						return;
					if (pAttacker == pLocal && g_Options.misc_DUCK_hitsound) 
						g_VGuiSurface->PlaySound_(_soundFileName);
				
				}
			}

	}

	void OnStudioRender() 
	{
		tracer.render();
	}

	int GetEventDebugID() override
	{
		return 42;
	}
private:
	const char* _soundFileName = "aggressiveduck.wav";
};