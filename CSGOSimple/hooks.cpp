#include "hooks.hpp"
#include <intrin.h>  

#include "render.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/misc/misc.hpp"
#include "features/visuals/visuals.hpp"
#include "features/skins/skins.h"
#include "ui.hpp"
#include "runtime_saving.h"
#include "features/misc/nightmode.h"
#include "features/misc/griefer.hpp"
#include "features/legit/aimbot.hpp"
#include "features/legit/prediction.hpp"
#include "valve_sdk/misc/GlobalVars.hpp"
#include "d3d_render/render_framework.hpp"


#pragma intrinsic(_ReturnAddress)  

void anti_cheat_fix()
{
	const char* modules[] { "client.dll", "engine.dll", "server.dll", "studiorender.dll", "materialsystem.dll", "shaderapidx9.dll", "vstdlib.dll", "vguimatsurface.dll" };
	long long long_long = 0x69690004C201B0;
	for (auto test : modules)
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)Utils::PatternScan(GetModuleHandleA(test), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &long_long, 7, 0);
}

namespace Hooks {

	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);

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
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();

		Glow::Get().Shutdown();
	}
	//--------------------------------------------------------------------------------
	bool __fastcall send_net_msg(void* ecx, void* edx, INetMessage* msg, bool reliable, bool voice)
	{
		static auto oFireEvent = hk_netchannel.get_original<sendnetmsg_fn>(40);

		if (msg->GetType() == 14) // Return and don't send messsage if its FileCRCCheck
			return false;
		if (msg->GetGroup() == 9) // Fix lag when transmitting voice and fakelagging
			voice = true;

		return oFireEvent(ecx, msg, reliable, voice);
	}

	bool __stdcall hkFireEvent(IGameEvent* pEvent) {
		static auto oFireEvent = gameevents_hook.get_original<FireEvent>(index::FireEvent);

		if (!pEvent)
			return oFireEvent(g_GameEvents, pEvent);

		const char* szEventName = pEvent->GetName();


		if (!strcmp(szEventName, "server_spawn"))
		{

			const auto net_channel = g_EngineClient->GetNetChannelInfo();

			if (net_channel != nullptr)
			{
				const auto index = 40;
				Hooks::hk_netchannel.setup(net_channel);
				Hooks::hk_netchannel.hook_index(index, send_net_msg);
			}
		}

		if (!strcmp(szEventName, "cs_game_disconnected"))
		{
			if (hk_netchannel.setup())
			{
				hk_netchannel.unhook_all();
			}
		}

		return oFireEvent(g_GameEvents, pEvent);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);
		
		if (g_Options.custom_viewmodel) {
			static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
			static auto viewmodel_offset_x = g_CVar->FindVar("viewmodel_offset_x");
			static auto viewmodel_offset_y = g_CVar->FindVar("viewmodel_offset_y");
			static auto viewmodel_offset_z = g_CVar->FindVar("viewmodel_offset_z");
			viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_fov->SetValue(g_Options.viewmodel_fov);
			viewmodel_offset_x->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_offset_x->SetValue(g_Options.viewmodel_offset_x);
			viewmodel_offset_y->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_offset_y->SetValue(g_Options.viewmodel_offset_y);
			viewmodel_offset_z->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_offset_z->SetValue(g_Options.viewmodel_offset_z);
		}
		else {
			static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
			static auto viewmodel_offset_x = g_CVar->FindVar("viewmodel_offset_x");
			static auto viewmodel_offset_y = g_CVar->FindVar("viewmodel_offset_y");
			static auto viewmodel_offset_z = g_CVar->FindVar("viewmodel_offset_z");
			viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_fov->SetValue(68);
			viewmodel_offset_x->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_offset_x->SetValue(2);
			viewmodel_offset_y->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_offset_y->SetValue(2);
			viewmodel_offset_z->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_offset_z->SetValue(-2);
		}

		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
		static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
		static auto crosshair_cvar = g_CVar->FindVar("crosshair");

		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_fov->SetValue(g_Options.viewmodel_fov);

		crosshair_cvar->SetValue(!(g_Options.dotcrosshair));
		crosshair_cvar->SetValue(!(g_Options.rendercrosshair));

		DWORD colorwrite, srgbwrite;
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vert_dec);
		pDevice->GetVertexShader(&vert_shader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);


		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		auto esp_drawlist = Render::Get().RenderScene();

		Menu::Get().Render();


		ImGui::Render(esp_drawlist);

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vert_dec);
		pDevice->SetVertexShader(vert_shader);

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
	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);

		oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);
		static auto prediction = new PredictionSystem();
		prediction->StartPrediction(cmd);

		g_Legitbot->Run(cmd);
		float max_radias = D3DX_PI * 2;
		float step = max_radias / 128;
		float xThick = 23;

		if (!cmd || !cmd->command_number)
			return;

		if (Menu::Get().IsVisible())
			cmd->buttons &= ~IN_ATTACK;

		if (g_Options.misc_bhop)
			Misc::OnCreateMove(cmd);

		if (g_Options.skyboxchanger) {

			static auto r_3dsky = g_CVar->FindVar(("r_3dsky"));

			r_3dsky->SetValue("0");


			static auto sv_skyname = g_CVar->FindVar(("sv_skyname"));

			// csgo\materials\skybox
			if (g_Options.skybox == 0)
			{
				sv_skyname->SetValue("cs_tibet"); 
			}
			if (g_Options.skybox == 1)
			{
				sv_skyname->SetValue("cs_baggage_skybox_");

			}
			if (g_Options.skybox == 2)
			{
				sv_skyname->SetValue("embassy");

			}
			if (g_Options.skybox == 3)
			{
				sv_skyname->SetValue("italy");

			}
			if (g_Options.skybox == 4)
			{
				sv_skyname->SetValue("jungle");

			}
			if (g_Options.skybox == 5)
			{
				sv_skyname->SetValue("office");
			}
			if (g_Options.skybox == 6)
			{
				sv_skyname->SetValue("sky_cs15_daylight01_hdr");
			}
			if (g_Options.skybox == 7)
			{
				sv_skyname->SetValue("vertigoblue_hdr");
			}
			if (g_Options.skybox == 8)
			{
				sv_skyname->SetValue("sky_cs15_daylight02_hdr");
			}
			if (g_Options.skybox == 9)
			{
				sv_skyname->SetValue("vertigo");
			}
			if (g_Options.skybox == 10)
			{
				sv_skyname->SetValue("sky_day02_05_hdr");
			}
			if (g_Options.skybox == 11)
			{
				sv_skyname->SetValue("nukeblank");
			}
			if (g_Options.skybox == 12)
			{
				sv_skyname->SetValue("sky_venice");
			}
			if (g_Options.skybox == 13)
			{
				sv_skyname->SetValue("sky_cs15_daylight03_hdr");
			}
			if (g_Options.skybox == 14)
			{
				sv_skyname->SetValue("sky_cs15_daylight04_hdr");
			}
			if (g_Options.skybox == 15)
			{
				sv_skyname->SetValue("sky_csgo_cloudy01");
			}
			if (g_Options.skybox == 16)
			{
				sv_skyname->SetValue("sky_csgo_night02");
			}
			if (g_Options.skybox == 17)
			{
				sv_skyname->SetValue("sky_csgo_night02b");
			}
			if (g_Options.skybox == 18)
			{
				sv_skyname->SetValue("sky_csgo_night_flat");
			}
			if (g_Options.skybox == 19)
			{
				sv_skyname->SetValue("sky_lunacy");
			}
			if (g_Options.skybox == 20)
			{
				sv_skyname->SetValue("sky_dust"); 
			}
			if (g_Options.skybox == 21)
			{
				sv_skyname->SetValue("vietnam");
			}
			if (g_Options.skybox == 22)
			{
				sv_skyname->SetValue("amethyst");
			}
			if (g_Options.skybox == 23)
			{
				sv_skyname->SetValue("sky_descent");
			}
			if (g_Options.skybox == 24)
			{
				sv_skyname->SetValue("clear_night_sky");
			}
			if (g_Options.skybox == 25)
			{
				sv_skyname->SetValue("otherworld");
			}
			if (g_Options.skybox == 26)
			{
				sv_skyname->SetValue("cloudynight");
			}
			if (g_Options.skybox == 27)
			{
				sv_skyname->SetValue("dreamyocean");
			}
			if (g_Options.skybox == 28)
			{
				sv_skyname->SetValue("grimmnight");
			}
			if (g_Options.skybox == 29)
			{
				sv_skyname->SetValue("sky051");
			}
			if (g_Options.skybox == 30)
			{
				sv_skyname->SetValue("sky081");
			}
			if (g_Options.skybox == 31)
			{
				sv_skyname->SetValue("sky091");
			}
			if (g_Options.skybox == 32)
			{
				sv_skyname->SetValue("sky561");
			}
		}
		else {
			static auto sv_skyname = g_CVar->FindVar(("sv_skyname"));

			sv_skyname->SetValue(0);
		}

		if (g_Options.misc_showranks && cmd->buttons & IN_SCORE)
			g_CHLClient->DispatchUserMessage(CS_UM_ServerRankRevealAll, 0, 0, nullptr);

		if (g_Options.VisibleCrosshair && !g_LocalPlayer->m_bIsScoped())
			g_CVar->FindVar("weapon_debug_spread_show")->SetValue(3);
		else
			g_CVar->FindVar("weapon_debug_spread_show")->SetValue(0);

		//if (g_Options.leftknife)
			//Visuals::Player::leftknife(); // make this shit static
		if (g_Options.blockbot && GetAsyncKeyState(g_Options.blockbotkey)) {
			g_BlockBot->cmove(cmd);
			g_BlockBot->draw();
			verified->m_cmd = *cmd;
			verified->m_crc = cmd->GetChecksum();
		}

		/*if (g_Options.spotifyapi)
		{
			std::string song_title = "";

			static HWND spotify_hwnd = nullptr;
			static float last_hwnd_time = 0.f;
			int text_width = 0;
			if ((!spotify_hwnd || spotify_hwnd == INVALID_HANDLE_VALUE) && last_hwnd_time < interfaces::globals->realtime + 2.f) {
				for (HWND hwnd = GetTopWindow(0); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {

					last_hwnd_time = interfaces::globals->realtime;

					if (!(IsWindowVisible)(hwnd))
						continue;

					int length = (GetWindowTextLengthW)(hwnd);
					if (length == 0)
						continue;

					WCHAR filename[300];
					DWORD pid;
					(GetWindowThreadProcessId)(hwnd, &pid);

					const auto spotify_handle = (OpenProcess)(PROCESS_QUERY_INFORMATION, FALSE, pid);
					(K32GetModuleFileNameExW)(spotify_handle, nullptr, filename, 300);

					std::wstring sane_filename{ filename };

					(CloseHandle)(spotify_handle);

					if (sane_filename.find((L"Spotify.exe")) != std::string::npos)
						spotify_hwnd = hwnd;
				}
			}
			else if (spotify_hwnd && spotify_hwnd != INVALID_HANDLE_VALUE) {
				WCHAR title[300];

				if (!(GetWindowTextW)(spotify_hwnd, title, 300)) {
					spotify_hwnd = nullptr;
				}
				else {
					std::wstring sane_title{ title };
					std::string Title = " ";
					std::string Song(sane_title.begin(), sane_title.end());
					Title += Song;
					if (sane_title.find((L"-")) != std::string::npos) {
						text_width = renderer::text_width(Title);
						renderer::draw_text(11, 11, color(255, 255, 255), Title.c_str());
						song_title = Title;
					}
					else if (sane_title.find((L"Advertisment")) != std::string::npos) {
						text_width = renderer::text_width("advertisment");
						song_title = "advertisment";
					}
					else if (sane_title.find((L"Spotify")) != std::string::npos) {
						text_width = renderer::text_width("stopped / not playing");
						song_title = "stopped / not playing";
					}
					else {
						text_width = renderer::text_width("advertisment");
						song_title = "advertisment";
					}

				}
			}


			renderer::draw_text(10 + 1, 11 + 1, color(0, 0, 0), song_title);
			renderer::draw_text(10, 11, color(255, 255, 255), song_title);
		}*/

		if (g_Options.leftknife)
		{
			static auto left_knife = g_CVar->FindVar("cl_righthand");

			if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
			{
				left_knife->SetValue(0);
				return;
			}

			auto& weapon = g_LocalPlayer->m_hActiveWeapon();

			if (!weapon)
				return;

			left_knife->SetValue(!weapon->IsKnife());
		}
	}
	//--------------------------------------------------------------------------------
	__declspec(naked) void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx; not sure if we need this
			push esp
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

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (g_Options.aspectratio)
			Visuals::Get().acpect(g_Options.aspectvalue);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel)
		{
			//Ignore 50% cuz it called very often
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			Render::Get().BeginScene();
		}

		nightmode::doNightmode();

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

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

		//pResolver::Run();

		if (g_LocalPlayer && g_EngineClient->IsInGame()) {
			//if (stage != ClientFrameStage_t::FRAME_RENDER_START && stage != ClientFrameStage_t::FRAME_RENDER_END)
			//	return;
			if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
				skins::on_frame_stage_notify(false);
			else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
				skins::on_frame_stage_notify(true);

			static int originalIdx = 0;
			
			if (!g_LocalPlayer) {
				originalIdx = 0;
				return;
			}

			constexpr auto getModel = [](int team) constexpr noexcept -> const char* {
				constexpr std::array models{
				"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
				"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
				"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
				"models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
				"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
				"models/player/custom_player/legacy/ctm_st6_variante.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
				"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
				"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
				"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
				"models/player/custom_player/legacy/tm_leet_variantf.mdl",
				"models/player/custom_player/legacy/tm_leet_variantg.mdl",
				"models/player/custom_player/legacy/tm_leet_varianth.mdl",
				"models/player/custom_player/legacy/tm_leet_varianti.mdl",
				"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
				"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
				"models/player/custom_player/legacy/tm_phoenix_varianth.mdl"
				};

				switch (team) {
				case 2: return static_cast<std::size_t>(g_Options.playerModelT - 1) < models.size() ? models[g_Options.playerModelT - 1] : nullptr;
				case 3: return static_cast<std::size_t>(g_Options.playerModelCT - 1) < models.size() ? models[g_Options.playerModelCT - 1] : nullptr;
				default: return nullptr;
				}
			};

			if (const auto model = getModel(g_LocalPlayer->m_iTeamNum())) {
				if (stage == FRAME_RENDER_START)
					originalIdx = g_LocalPlayer->m_nModelIndex();

				const auto idx = stage == FRAME_RENDER_END && originalIdx ? originalIdx : g_MdlInfo->GetModelIndex(model);

				g_LocalPlayer->setModelIndex(idx);

				if (const auto ragdoll = g_LocalPlayer->get_entity_from_handle(g_LocalPlayer->m_hRagdoll()))
					ragdoll->setModelIndex(idx);
			}
		}
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

		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}

		Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}



	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall*)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}

	void RecvProxy(const CRecvProxyData* pData, void* entity, void* output)
	{
		static auto ofunc = sequence_hook->get_original_function();

		if (g_LocalPlayer && g_LocalPlayer->IsAlive()) {
			const auto proxy_data = const_cast<CRecvProxyData*>(pData);
			const auto view_model = static_cast<C_BaseViewModel*>(entity);

			if (view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid()) {
				const auto owner = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(view_model->m_hOwner()));
				if (owner == g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer())) {
					const auto view_model_weapon_handle = view_model->m_hWeapon();
					if (view_model_weapon_handle.IsValid()) {
						const auto view_model_weapon = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
						if (view_model_weapon) {
							if (k_weapon_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex())) {
								auto original_sequence = proxy_data->m_Value.m_Int;
								const auto override_model = k_weapon_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
								proxy_data->m_Value.m_Int = skins::GetNewAnimation(override_model, proxy_data->m_Value.m_Int);
							}
						}
					}
				}
			}

		}

		ofunc(pData, entity, output);
	}

	void __fastcall hkDrawModelExecute2(void* _this, int, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags)
	{
		static auto ofunc = stdrender_hook.get_original<decltype(&hkDrawModelExecute2)>(index::DrawModelExecute2);

		if (g_StudioRender->IsForcedMaterialOverride())
			return ofunc(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		static auto flash = g_MatSystem->FindMaterial("effects/flashbang", TEXTURE_GROUP_CLIENT_EFFECTS);
		static auto flash_white = g_MatSystem->FindMaterial("effects/flashbang_white", TEXTURE_GROUP_CLIENT_EFFECTS);
		flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Options.no_flash);
		flash_white->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Options.no_flash);
		std::vector<const char*> vistasmoke_mats =

		{
				"particle/vistasmokev1/vistasmokev1_fire",
				"particle/vistasmokev1/vistasmokev1_smokegrenade",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		};

		for (auto mat_s : vistasmoke_mats)
		{
			IMaterial* mat = g_MatSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, g_Options.no_smoke);
		}


		ofunc(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		g_StudioRender->ForcedMaterialOverride(nullptr);
	}

	void __fastcall Hooks::RenderSmokeOverlay(void* _this, int, const bool unk)
	{
		static auto ofunc = viewrender_hook.get_original<decltype(&RenderSmokeOverlay)>(index::RenderSmokeOverlay);

		if (!g_Options.no_smoke)
			ofunc(g_ViewRender, 0, unk);
	}
}
