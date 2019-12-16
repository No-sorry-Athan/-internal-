#include "hooks.hpp"
#include <intrin.h>  

#include "render.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "bhop.hpp"
#include "chams.hpp"
#include "visuals.hpp"
#include "glow.hpp"
#include "aimbot.hpp"
#include "backtrack.hpp"
#include "FakeLatency.hpp"
//#include "EventListener.hpp"
//#include "antiaim.hpp"
//#include "forwardtrack.hpp"

#pragma intrinsic(_ReturnAddress)  

//std::unique_ptr<EventListener> g_EventListener = nullptr;

#define net_frame_backup 64
#define net_frame_mask (net_frame_backup-1)
#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )
namespace Hooks
{
	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode); 
		
		//find_model_hook.setup(g_MdlCache);
		
		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);

		//writeUserCmd_hook.setup(g_CHLClient);
		//writeUserCmd_hook.hook_index(index::writeUserCmd, WriteUsercmdDeltaToBuffer_h);

		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);
		//find_model_hook.hook_index(index::findModel, hkFindModel);
		
		//g_EventListener = std::make_unique<EventListener>();
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		send_datagram.unhook_all();
		//writeUserCmd_hook.unhook_all();
		//g_EventListener->~EventListener();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();
		//find_model_hook.unhook_all();
		Glow::Get().Shutdown();
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);
		
		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");		
		static auto viewmodel_offset_x = g_CVar->FindVar("viewmodel_offset_x");
		static auto viewmodel_offset_y = g_CVar->FindVar("viewmodel_offset_y");
		static auto viewmodel_offset_z = g_CVar->FindVar("viewmodel_offset_z");
		static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
		static auto crosshair_cvar = g_CVar->FindVar("crosshair");

		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_fov->SetValue(g_Options.viewmodel_fov);

		viewmodel_offset_x->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_offset_x->SetValue(g_Options.viewmodel_offset_x);

		viewmodel_offset_y->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_offset_y->SetValue(g_Options.viewmodel_offset_y);

		viewmodel_offset_z->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_offset_z->SetValue(g_Options.viewmodel_offset_z);

		mat_ambient_light_r->SetValue(g_Options.mat_ambient_light_r);
		mat_ambient_light_g->SetValue(g_Options.mat_ambient_light_g);
		mat_ambient_light_b->SetValue(g_Options.mat_ambient_light_b);

		DWORD colorwrite, srgbwrite;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);


		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		ImDrawList* esp_drawlist = nullptr;

		esp_drawlist = Render::Get().RenderScene();

		Menu::Get().Render();

		ImGui::Render(esp_drawlist);

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);

		return oEndScene(pDevice);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);

		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
			Menu::Get().OnDeviceReset();

		return hr;
	}
	//--------------------------------------------------------------------------------
	Vector hitPos;
	static bool firstRun = false;

	static int tickBaseShift = 0;
	static bool inSendMove = false;
	static bool firstSendMovePack = false;

	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);
		oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!cmd || !cmd->command_number)
			return;

		if (Menu::Get().IsVisible())
			cmd->buttons &= ~IN_ATTACK;

		if (g_Options.misc_bhop)
			BunnyHop::OnCreateMove(cmd);

		if (g_Options.fakelag_enabled) {
			if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected() && !firstRun) {
				if (g_ClientState) {
					INetChannel* netChan = (INetChannel*)(g_ClientState->m_NetChannel);
					if (netChan) {
						send_datagram.setup(netChan);
						send_datagram.hook_index(index::SendDatagram, hkSendDatagram);
						firstRun = true;
					}
				}
			}
			fakelatency.UpdateIncomingSequences();
		}

		if (g_Options.backtrack_enabled) {
			backtrack.BacktrackStart(cmd);
			backtrack.BacktrackCalc(cmd);
		}

		if (g_Options.aim_aimbot_enabled) {
			if (g_Options.aim_legitaimbot_enabled)
				aimbot.legitaimb(cmd, bSendPacket);

			if (g_Options.aim_aimbot_ragebot)
				aimbot.ragebot(cmd, bSendPacket);

			if (g_Options.aim_backtrack_aimbot)
				backtrack.BacktrackAimbot(cmd, bSendPacket);
		}

		if (g_Options.esp_recoil_crosshair) {
			auto viewangles = cmd->viewangles + g_LocalPlayer->m_aimPunchAngle() * 2.0f;

			Vector vStart = g_LocalPlayer->GetEyePos();
			Vector vEnd;

			Ray_t pRay;
			trace_t pTrace;
			CTraceFilter pTraceFilter;
			pTraceFilter.pSkip = g_LocalPlayer;

			Math::AngleVectors(viewangles, vEnd);

			vEnd += vStart;

			pRay.Init(vStart, vEnd);

			g_EngineTrace->TraceRay(pRay, MASK_SHOT, &pTraceFilter, &pTrace);

			hitPos = pTrace.endpos;
		}
	
		if (g_Options.tickBaseManip) {
			if (g_Options.chokePackets) {
				if (g_LocalPlayer && g_LocalPlayer->IsAlive() && !(cmd->buttons & IN_ATTACK)) {
					static int chokedTicks;
					if (chokedTicks == 15) {
						bSendPacket = true;
						chokedTicks = 0;
					}
					else {
						bSendPacket = false;
						chokedTicks++;
					}

				}
			}

			if (g_Options.fakeDuck) {
				if (g_LocalPlayer && g_LocalPlayer->m_iHealth() > 0) {
					if (cmd->buttons & IN_DUCK) {
						cmd->buttons |= IN_BULLRUSH;
						static int toast;
						if (toast == 9) {
							bSendPacket = true;
							cmd->buttons |= IN_DUCK;
							toast = 0;
						}
						else {
							bSendPacket = false;
							cmd->buttons &= ~IN_DUCK;
							toast++;
						}
					}
				}
			}

			if (g_Options.fakeStand) {
				if (g_LocalPlayer && g_LocalPlayer->m_iHealth() > 0) {
					if (cmd->buttons & IN_SPEED) {
						cmd->buttons |= IN_BULLRUSH;
						static int bread;
						if (bread == 9) {
							bSendPacket = true;
							bread = 0;
							cmd->buttons &= ~IN_DUCK;
						}
						else {
							bSendPacket = false;
							bread++;
							cmd->buttons |= IN_DUCK;
						}
					}
				}
			}

			if (g_Options.instantPlant && (cmd->buttons & IN_USE)) {
				static int sinceUse = 0;
				if (sinceUse < 3) {
					tickBaseShift = TIME_TO_TICKS(11.f);
					sinceUse = 0;
				}
				else
					sinceUse++;
			}
		}
		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
		return;
	}
	//--------------------------------------------------------------------------------
	__declspec(naked) void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx
			lea  ecx, [esp]
			push ecx
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);


		if (!strcmp("HudZoom", g_VGuiPanel->GetName(panel)))
			return;

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);
		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}

		}
		else if (panelId == panel) {
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			if (g_LocalPlayer && InputSys::Get().IsKeyDown(VK_TAB) && g_Options.misc_showranks)
				Utils::RankRevealAll();

			/*if (g_Options.draw_tracers_enabled) {
				g_EventListener->OnStudioRender();
			}*/

			if (g_Options.esp_recoil_crosshair) {
				if (g_LocalPlayer && g_LocalPlayer->m_iShotsFired() >= 1 && g_EngineClient->IsConnected() && g_EngineClient->IsInGame()) {
					Vector screen;
					g_DebugOverlay->ScreenPosition(hitPos, screen);
					g_VGuiSurface->DrawSetColor(Color::White);
					g_VGuiSurface->DrawLine(screen.x - 4, screen.y, screen.x + 4, screen.y);
					g_VGuiSurface->DrawLine(screen.x, screen.y - 4, screen.x, screen.y + 4);
				}
				else {
					if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
						return;
					int w, h;
					g_EngineClient->GetScreenSize(w, h);
					int cx = w / 2;
					int cy = h / 2;
					g_VGuiSurface->DrawSetColor(Color::White);
					g_VGuiSurface->DrawLine(cx - 4, cy, cx + 4, cy);
					g_VGuiSurface->DrawLine(cx, cy - 4, cx, cy + 4);
				}
			}

			if (g_Options.backtrack_dots_enabled)
				backtrack.BactrackDots();

			if (g_Options.esp_bones) {
				for (int i = 0; i < g_EngineClient->GetMaxClients(); i++) {
					auto entity = reinterpret_cast<C_BasePlayer*>(C_BaseEntity::GetEntityByIndex(i));
					if (!entity || entity->IsDormant() || !entity->IsAlive())
						continue;
					if (!g_Options.esp_render_team)
						if (entity == g_LocalPlayer || entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
							continue;
					Visuals::Get().DrawBones(entity);
				}
			}
			if (g_LocalPlayer && g_LocalPlayer->m_bIsScoped() && g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
				int w, h;
				g_EngineClient->GetScreenSize(w, h);
				g_VGuiSurface->DrawSetColor(Color::White);
				g_VGuiSurface->DrawLine(w / 2, 0, w / 2, h);
				g_VGuiSurface->DrawLine(0, h / 2, w, h / 2);
			}
			
			Render::Get().BeginScene();
		}
			static bool skip = false;
			skip = !skip;
			if (skip)
				return;
			else {
				if (g_Options.fakelag_enabled && firstRun) {
					if (!g_EngineClient->IsInGame()) {
						firstRun = false;
					}
					else if (!g_EngineClient->IsConnected()) {
						firstRun = false;
					}
					else if (g_EngineClient->IsLevelMainMenuBackground())
					{
						firstRun = false;
					}
				}
			}
		
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);
		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				//This will flash the CSGO window on the taskbar
				//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}
		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);
	}
	//--------------------------------------------------------------------------------
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && g_Options.glow_enabled)
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);
		ofunc(g_CHLClient, edx, stage);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

		if (g_EngineClient->IsInGame() && vsView)
			Visuals::Get().ThirdPerson();

		ofunc(g_ClientMode, edx, vsView);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		if (!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame())
			return;

		if (g_Options.chams_backtrack_enabled)
			backtrack.BacktrackChams(ctx, state, pInfo, pCustomBoneToWorld);
		
		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}

		Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}
	//-------------------------------------------------------------------------------
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall*)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}
	//-------------------------------------------------------------------------------
	int __fastcall hkSendDatagram(INetChannel* netchan, void*, bf_write* datagram)
	{
		static auto oSendDatagram = send_datagram.get_original<SendDatagramFn>(index::SendDatagram);

		if (!g_Options.fakelag_enabled) 
			return oSendDatagram(netchan, datagram);
		
		int instate = netchan->m_nInReliableState;
		int insequencenr = netchan->m_nInSequenceNr;

		fakelatency.AddLatencyToNetchan(netchan, g_Options.fakelag);

		int ret = oSendDatagram(netchan, datagram);

		netchan->m_nInReliableState = instate;
		netchan->m_nInSequenceNr = insequencenr;
		
		return ret;
	}
	//--------------------------------------------------------------------------
	//void __fastcall hkFindModel(void* ecx, void* edx, char* filepath)
	//{
	//	/*if (strstr(filepath, "models/player") && !strstr(filepath, "contactshadow")) 
	//		sprintf(filepath, "models/player/ubneptune/neptune_zise/ubneptune.mdl");*/
	//}
	//--------------------------------------------------------------------------
	//bool __fastcall WriteUsercmdDeltaToBuffer_h(IBaseClientDLL* ECX, void* EDX, int nSlot, bf_write* buf, int from, int to, bool isNewCmd){
	//		static auto o_WriteUsercmdDeltaToBuffer = writeUserCmd_hook.get_original<decltype(&WriteUsercmdDeltaToBuffer_h)>(index::writeUserCmd);
	//		static auto WriteUsercmdDeltaToBufferReturn = (DWORD)Utils::PatternScan(GetModuleHandleA("engine.dll"), "84 C0 74 04 B0 01 EB 02 32 C0 8B FE");
	//		if (tickBaseShift <= 0 || (DWORD)_ReturnAddress() != WriteUsercmdDeltaToBufferReturn)
	//			return o_WriteUsercmdDeltaToBuffer(ECX, EDX, nSlot, buf, from, to, isNewCmd);

	//		if (from != -1)
	//			return true;

	//		auto CL_SendMove = []()
	//		{
	//			using CL_SendMove_t = void(__fastcall*)(void);
	//			static CL_SendMove_t CL_SendMoveF = (CL_SendMove_t)Utils::PatternScan(GetModuleHandleA("engine.dll"), "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98");
	//			CL_SendMoveF();
	//		};

	//		auto WriteUsercmd = [](bf_write* buf, CUserCmd* in, CUserCmd* out)
	//		{
	//			using WriteUsercmd_t = void(__fastcall*)(bf_write*, CUserCmd*, CUserCmd*);
	//			static WriteUsercmd_t WriteUsercmdF = (WriteUsercmd_t)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D");
	//			WriteUsercmdF(buf, in, out);
	//		};
	//		int* pNumBackupCommands = (int*)(reinterpret_cast<uintptr_t>(buf) - 0x30);
	//		int* pNumNewCommands = (int*)(reinterpret_cast<uintptr_t>(buf) - 0x2C);
	//		auto net_channel = (INetChannel*)(g_ClientState->m_NetChannel);
	//		int32_t new_commands = *pNumNewCommands;
	//		if (!inSendMove)
	//		{
	//			if (new_commands <= 0)
	//				return false;
	//			inSendMove = true;
	//			firstSendMovePack = true;
	//			tickBaseShift += new_commands;

	//			while (tickBaseShift > 0)
	//			{
	//				CL_SendMove();
	//				//net_channel->transmit(false);
	//				firstSendMovePack = false;
	//			}

	//			inSendMove = false;
	//			return false;
	//		}

	//		if (!firstSendMovePack)
	//		{
	//			int32_t loss = min(tickBaseShift, 10);

	//			tickBaseShift -= loss;
	//			net_channel->m_nOutSequenceNr += loss;
	//		}

	//		int32_t next_cmdnr = g_ClientState->lastoutgoingcommand + g_ClientState->chokedcommands + 1;
	//		int32_t total_new_commands = min(tickBaseShift, 62);
	//		tickBaseShift -= total_new_commands;

	//		from = -1;
	//		*pNumNewCommands = total_new_commands;
	//		*pNumBackupCommands = 0;

	//		for (to = next_cmdnr - new_commands + 1; to <= next_cmdnr; to++)
	//		{
	//			if (!o_WriteUsercmdDeltaToBuffer(ECX, EDX, nSlot, buf, from, to, true))
	//				return false;

	//			from = to;
	//		}

	//		CUserCmd* last_realCmd = g_Input->GetUserCmd(nSlot, from);
	//		CUserCmd fromCmd;

	//		if (last_realCmd)
	//			fromCmd = *last_realCmd;

	//		CUserCmd toCmd = fromCmd;
	//		toCmd.command_number++;
	//		toCmd.tick_count += 200;

	//		for (int i = new_commands; i <= total_new_commands; i++)
	//		{
	//			WriteUsercmd(buf, &toCmd, &fromCmd);
	//			fromCmd = toCmd;
	//			toCmd.command_number++;
	//			toCmd.tick_count++;
	//		}

	//		return o_WriteUsercmdDeltaToBuffer(ECX, EDX, nSlot, buf, from, to, isNewCmd);
	//	}
}


