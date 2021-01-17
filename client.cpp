#include "client.h"

cl_clientfunc_t *g_pClient;
cl_enginefunc_t *g_pEngine;
engine_studio_api_t *g_pStudio;
r_studio_interface_t* g_pInterface;
StudioModelRenderer_t* g_pStudioModelRenderer;
playermove_t* pmove;

CL_Move_t CL_Move_s;
PreS_DynamicSound_t PreS_DynamicSound_s;
cl_clientfunc_t g_Client;
cl_enginefunc_t g_Engine;
engine_studio_api_t g_Studio;
r_studio_interface_t g_Interface;
StudioModelRenderer_t g_StudioModelRenderer;
PColor24 Console_TextColor;

DWORD HudRedraw;

void HUD_Redraw(float time, int intermission)
{
	g_Client.HUD_Redraw(time, intermission);

	HudRedraw = GetTickCount();

	DrawOverviewLayer();
	KzFameCount();
}

int HUD_Key_Event(int down, int keynum, const char* pszCurrentBinding)
{
	//preset keys bind
	bool keyrush = cvar.rage_active && keynum == cvar.route_rush_key;
	bool keyquick = cvar.misc_quick_change && keynum == cvar.misc_quick_change_key;
	bool keystrafe = cvar.kz_strafe && keynum == cvar.kz_strafe_key;
	bool keyfast = cvar.kz_fast_run && keynum == cvar.kz_fastrun_key;
	bool keygstrafe = cvar.kz_ground_strafe && keynum == cvar.kz_ground_strafe_key;
	bool keybhop = cvar.kz_bhop && keynum == cvar.kz_bhop_key;
	bool keyjump = cvar.kz_jump_bug && keynum == cvar.kz_jumpbug_key;
	bool keyrage = cvar.rage_active && !cvar.rage_auto_fire && !cvar.rage_always_fire && keynum == cvar.rage_auto_fire_key;
	bool keylegit = !cvar.rage_active && cvar.legit[g_Local.weapon.m_iWeaponID].active && IsCurWeaponGun() && keynum == cvar.legit_key;
	bool keylegittrigger = !cvar.rage_active && cvar.legit[g_Local.weapon.m_iWeaponID].trigger_active && !cvar.legit[g_Local.weapon.m_iWeaponID].active && IsCurWeaponGun() && keynum == cvar.legit_trigger_key;
	bool keychat = cvar.gui_chat && keynum == cvar.gui_chat_key;
	bool keychatteam = cvar.gui_chat && keynum == cvar.gui_chat_key_team;
	bool keymenu = keynum == cvar.gui_key;

	//send keys to menu
	keysmenu[keynum] = down;

	//send chat message
	if (bInputActive && keynum == K_ENTER && down)
	{
		char SayText[255];
		if (iInputMode == 1)sprintf(SayText, "say \"%s\"", InputBuf);
		if (iInputMode == 2)sprintf(SayText, "say_team \"%s\"", InputBuf);
		g_Engine.pfnClientCmd(SayText);
		bInputActive = false;
		for (unsigned int i = 0; i < 256; i++) ImGui::GetIO().KeysDown[i] = false;
		return false;
	}
	//return game bind if chat active
	if (bInputActive && GetTickCount() - HudRedraw <= 100)
		return false;

	//return game bind for chat bind
	if (keychat && down)
	{
		bInputActive = true, iInputMode = 1;
		return false;
	}
	if (keychatteam && down)
	{
		bInputActive = true, iInputMode = 2;
		return false;
	}
	
	//return game bind for menu bind
	if (keymenu && down)
	{
		bShowMenu = !bShowMenu;
		if(!bShowMenu)SaveCvar();
		return false;
	}

	//return game bind if menu active
	if (bShowMenu && GetTickCount() - HudRedraw <= 100)
		return false;

	//check if alive
	if (bAliveLocal())
	{
		//do function bind
		if (keyrush)
		{
			if (down)
			{
				if(cvar.route_activate) cvar.route_auto = 1;
				cvar.misc_wav_speed = 64;
			}
			else
			{
				if (cvar.route_activate) cvar.route_auto = 0;
				cvar.misc_wav_speed = 1;
			}
		}
		if (keyquick && down)
		{
			cvar.rage_active = !cvar.rage_active, ModeChangeDelay = GetTickCount(), SaveCvar();
			if (!cvar.rage_active)cvar.route_auto = 0, cvar.misc_wav_speed = 1, RageKeyStatus = false;
		}
		if (keystrafe)
			Strafe = down;
		if (keyfast)
			Fastrun = down;
		if (keygstrafe)
			Gstrafe = down;
		if (keybhop)
			Bhop = down;
		if (keyjump)
			Jumpbug = down;
		if (keyrage)
			RageKeyStatus = down;
		if (keylegit)
			LegitKeyStatus = down;
		if (keylegittrigger && down)
			TriggerKeyStatus = !TriggerKeyStatus;
		
		//return game bind for function bind
		if ((keystrafe || keyfast || keygstrafe || keybhop || keyjump || keyrage || keylegittrigger || keylegit || keyquick || keyrush) && down)
			return false;
	}
	
	return g_Client.HUD_Key_Event(down, keynum, pszCurrentBinding);
}

void AntiAfk(usercmd_s* cmd)
{
	int afktime = cvar.afk_time;
	afktime -= 1;
	afktime *= 1000;
	static DWORD antiafk = GetTickCount();
	static Vector prevorigin;
	static Vector prevangles;
	if (bAliveLocal())
	{
		if (pmove->origin != prevorigin || cmd->viewangles != prevangles)
			antiafk = GetTickCount();
		prevorigin = pmove->origin;
		prevangles = cmd->viewangles;
		if (cvar.afk_anti)
		{
			if (GetTickCount() - antiafk > afktime)
			{
				cmd->buttons |= IN_JUMP;
				cmd->viewangles[1] += 5;
				g_Engine.SetViewAngles(cmd->viewangles);
			}

		}
	}
	else
		antiafk = GetTickCount();
}

void CL_CreateMove(float frametime, struct usercmd_s* cmd, int active)
{
	g_Client.CL_CreateMove(frametime, cmd, active);
	AdjustSpeed(cvar.misc_wav_speed);
	UpdateWeaponData();
	AimBot(cmd);
	ContinueFire(cmd);
	ItemPostFrame(cmd);
	Kz(frametime, cmd);
	NoRecoil(cmd);
	NoSpread(cmd);
	Route(cmd);
	AntiAim(cmd);
	FakeLag(frametime, cmd);
	CustomFOV();
	WallRun();
	CrossHair();
	AntiAfk(cmd);
}

void HUD_PostRunCmd(struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_Client.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);
	ItemPreFrame(from, to, cmd, runfuncs, time, random_seed);
}

void PreV_CalcRefdef(struct ref_params_s* pparams)
{
	g_Local.vPunchangle = pparams->punchangle;
	g_Local.vPrevForward = pparams->forward;
	g_Local.iPrevHealth = pparams->health;
	V_CalcRefdefRecoil(pparams);
}

void PostV_CalcRefdef(struct ref_params_s* pparams)
{
	TraceGrenade(pparams);
	if (DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
	{
		cl_entity_s* vm = g_Engine.GetViewModel();
		if (vm)
		{
			Vector forward = pparams->forward;
			vm->origin += forward * cvar.visual_viewmodel_fov;
		}
	}
	g_Local.vPostForward = pparams->forward;
	g_Local.iPostHealth = pparams->health;
	ThirdPerson(pparams);
}

void V_CalcRefdef(struct ref_params_s* pparams)
{
	PreV_CalcRefdef(pparams);
	g_Client.V_CalcRefdef(pparams);
	PostV_CalcRefdef(pparams);
}

void bSendpacket(bool status)
{
	static bool bsendpacket_status = true;
	static DWORD NULLTIME = NULL;
	if (status && !bsendpacket_status)
	{
		bsendpacket_status = true;
		*(DWORD*)(c_Offset.dwSendPacketPointer) = c_Offset.dwSendPacketBackup;
	}
	if (!status && bsendpacket_status)
	{
		bsendpacket_status = false;
		*(DWORD*)(c_Offset.dwSendPacketPointer) = (DWORD)& NULLTIME;
	}
}

void CL_Move()
{
	bSendpacket(true);
	CL_Move_s();
}

void AdjustSpeed(double speed)
{
	static double LastSpeed = 1;
	if (speed != LastSpeed)
	{
		*(double*)c_Offset.dwSpeedPointer = (speed * 1000);

		LastSpeed = speed;
	}
}

void HUD_PlayerMoveInit(struct playermove_s* ppmove)
{
	PM_InitTextureTypes(ppmove);
	return g_Client.HUD_PlayerMoveInit(ppmove);
}

void HUD_ProcessPlayerState(struct entity_state_s* dst, const struct entity_state_s* src)
{
	for (unsigned int i = 0; i < 3; i++)
		src->mins[i] = i == 2 ? -36 : -16;
	for (unsigned int i = 0; i < 3; i++)
		src->maxs[i] = i == 2 ? 36 : 16;
	g_Client.HUD_ProcessPlayerState(dst, src);
}

int HUD_GetHullBounds(int hullnum, float* mins, float* maxs)
{
	if (hullnum == 1)
		maxs[2] = 32.0f;
	return 1;
}

int HUD_AddEntity(int type, struct cl_entity_s* ent, const char* modelname)
{
	return g_Client.HUD_AddEntity(type, ent, modelname);
}

int CL_IsThirdPerson(void)
{
	if (cvar.visual_chase_cam && bAliveLocal() && DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
		return 1;
	return g_Client.CL_IsThirdPerson();
}

void ConsoleMessage()
{
	static bool FirstFrame = true;
	if (FirstFrame)
	{
		g_Engine.pfnClientCmd("toggleconsole");

		ConsolePrintColor(255, 255, 255, "\n\n\t\t\t\tHello, %s ;)\n", g_Engine.pfnGetCvarString("name"));
		ConsolePrintColor(255, 255, 255, "\t\t\t\tYou are injected!\n\n");

		ConsolePrintColor(255, 0, 255, "\t\t\t\tMultihack by:\n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t            [..         [..    [....     [..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t             [..       [..   [..    [..  [..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t     [..      [..     [..  [..        [..[..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t   [.   [..    [..   [..   [..        [..[..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t  [..... [..    [.. [..    [..        [..[..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t  [.             [....       [..     [.. [..      \n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\t    [....         [..          [....     [........\n");
		ConsolePrintColor(255, 0, 255, "\t\t\t\tand team!\n\n");

		ConsolePrintColor(255, 255, 255, "\t\t\t\tSpecial thanks to my friend BloodSharp and oxiKKK <3\n\n");
		ConsolePrintColor(255, 255, 255, "\t\t\t\tATTENTION! Menu only active in game!\n");
		if (cvar.gui_key != -1)
			ConsolePrintColor(0, 255, 0, "\t\t\t\tMenu key is [%s]!\n", KeyEventChar((int)cvar.gui_key));

		FirstFrame = false;
	}
	static float ChangeKey = cvar.gui_key;
	if (ChangeKey != cvar.gui_key)
	{
		if (cvar.gui_key == -1)
			ConsolePrintColor(255, 255, 0, "\t\t\t\tMenu key is [Press key]!\n");
		else
			ConsolePrintColor(0, 255, 0, "\t\t\t\tMenu key is [%s]!\n", KeyEventChar((int)cvar.gui_key));

		ChangeKey = cvar.gui_key;
	}
}

void HUD_Frame(double time)
{
	ConsoleMessage();
	Snapshot();
	FindSpawn();
	LoadTextureWall();
	Sky();
	NoFlash();
	Lightmap();
	PlayerAim.deque::clear();
	g_Client.HUD_Frame(time);
}

bool CalcForward(Vector vForward, float& forwards, Vector vRight, float rights, Vector vUp, float ups )
{
	for (unsigned int i = 0; i < 65535; i++)
	{
		Vector origin1 = pmove->origin + pmove->view_ofs + vForward * forwards + vRight * rights - vUp * (ups - 32);
		Vector origin2 = pmove->origin + pmove->view_ofs + vForward * forwards + vRight * rights - vUp * (ups + 32);

		float Bot[2], Top[2];

		g_Engine.pTriAPI->WorldToScreen(origin1, Bot);
		Bot[0] = Bot[0] * (ImGui::GetIO().DisplaySize.x / 2) + (ImGui::GetIO().DisplaySize.x / 2);
		Bot[1] = -Bot[1] * (ImGui::GetIO().DisplaySize.y / 2) + (ImGui::GetIO().DisplaySize.y / 2);

		g_Engine.pTriAPI->WorldToScreen(origin2, Top);
		Top[0] = Top[0] * (ImGui::GetIO().DisplaySize.x / 2) + (ImGui::GetIO().DisplaySize.x / 2);
		Top[1] = -Top[1] * (ImGui::GetIO().DisplaySize.y / 2) + (ImGui::GetIO().DisplaySize.y / 2);

		float height;
		height = (Top[1] - Bot[1]);
		if (int(height) < 130)
			forwards -= 0.001f;
		else if (int(height) > 130)
			forwards += 0.001f;
		else
			return true;
	}
	return false;
}

bool CalcRight(float x, float y, Vector vForward, float forwards, Vector vRight, float& rights, Vector vUp, float ups)
{
	for (unsigned int i = 0; i < 65535; i++)
	{
		Vector origin = pmove->origin + pmove->view_ofs + vForward * forwards + vRight * rights - vUp * ups;

		float screen[2];

		g_Engine.pTriAPI->WorldToScreen(origin, screen);
		screen[0] = screen[0] * (ImGui::GetIO().DisplaySize.x / 2) + (ImGui::GetIO().DisplaySize.x / 2);
		screen[1] = -screen[1] * (ImGui::GetIO().DisplaySize.y / 2) + (ImGui::GetIO().DisplaySize.y / 2);

		if (int(screen[0]) < int(x))
			rights += 0.001f;
		else if (int(screen[0]) > int(x))
			rights -= 0.001f;
		else
			return true;
	}
	return false;
}

bool CalcUp(float x, float y, Vector vForward, float forwards, Vector vRight, float rights, Vector vUp, float& ups)
{
	for (unsigned int i = 0; i < 65535; i++)
	{
		Vector origin = pmove->origin + pmove->view_ofs + vForward * forwards + vRight * rights - vUp * ups;

		float screen[2];

		g_Engine.pTriAPI->WorldToScreen(origin, screen);
		screen[0] = screen[0] * (ImGui::GetIO().DisplaySize.x / 2) + (ImGui::GetIO().DisplaySize.x / 2);
		screen[1] = -screen[1] * (ImGui::GetIO().DisplaySize.y / 2) + (ImGui::GetIO().DisplaySize.y / 2);

		if (int(screen[1]) < int(y))
			ups += 0.001f;
		else if (int(screen[1]) > int(y))
			ups -= 0.001f;
		else
			return true;
	}
	return false;
}

bool WorldToScreen(float* pflOrigin)
{
	float pflVecScreen[2];
	int iResult = g_Engine.pTriAPI->WorldToScreen(pflOrigin, pflVecScreen);
	if (pflVecScreen[0] < 1.5f && pflVecScreen[1] < 1.5f && pflVecScreen[0] > -1.5f && pflVecScreen[1] > -1.5f && !iResult)
	{
		pflVecScreen[0] = pflVecScreen[0] * (ImGui::GetIO().DisplaySize.x / 2) + (ImGui::GetIO().DisplaySize.x / 2);
		pflVecScreen[1] = -pflVecScreen[1] * (ImGui::GetIO().DisplaySize.y / 2) + (ImGui::GetIO().DisplaySize.y / 2);
		return true;
	}
	return false;
}

Vector screenshit(Vector viewangle)
{
	static bool loadmodel = false;

	static float forwards = 100, rights = 0, ups = 0;

	static float screenx = 0, screeny = 0;
	float x = modelscreenx + (modelscreenw / 2), y = modelscreeny + (modelscreenh / 2) + 25;
	if (screenx != x || screeny != y)
	{
		screenx = x, screeny = y; 
		loadmodel = true;
	}

	static float screensizex = 0, screensizey = 0;
	if (screensizex != ImGui::GetIO().DisplaySize.x || screensizey != ImGui::GetIO().DisplaySize.y)
	{
		screensizex = ImGui::GetIO().DisplaySize.x, screensizey = ImGui::GetIO().DisplaySize.y;
		loadmodel = true;
	}

	Vector vForward, vRight, vUp;
	g_Engine.pfnAngleVectors(viewangle, vForward, vRight, vUp);

	if (loadmodel)
	{
		Vector origin = pmove->origin + pmove->view_ofs + vForward * forwards + vRight * rights - vUp * ups;
		if (!WorldToScreen(origin))
			forwards = 100, rights = 0, ups = 0;
		if(CalcForward(vForward, forwards, vRight, rights, vUp, ups) && CalcRight(x, y, vForward, forwards, vRight, rights, vUp, ups) && CalcUp(x, y, vForward, forwards, vRight, rights, vUp, ups))
			loadmodel = false;
	}

	return pmove->origin + pmove->view_ofs + vForward * forwards + vRight * rights - vUp * ups;
}

void HUD_CreateEntities()
{
	static int modelindexarctic;
	static int modelindexgign;
	static int modelindexgsg9;
	static int modelindexguerilla;
	static int modelindexleet;
	static int modelindexsas;
	static int modelindexterror;
	static int modelindexurban;
	static int modelindexvip;

	static struct model_s* arctic = g_Engine.CL_LoadModel("models/player/arctic/arctic.mdl", &modelindexarctic);
	static struct model_s* gign = g_Engine.CL_LoadModel("models/player/gign/gign.mdl", &modelindexgign);
	static struct model_s* gsg9 = g_Engine.CL_LoadModel("models/player/gsg9/gsg9.mdl", &modelindexgsg9);
	static struct model_s* guerilla = g_Engine.CL_LoadModel("models/player/guerilla/guerilla.mdl", &modelindexguerilla);
	static struct model_s* leet = g_Engine.CL_LoadModel("models/player/leet/leet.mdl", &modelindexleet);
	static struct model_s* sas = g_Engine.CL_LoadModel("models/player/sas/sas.mdl", &modelindexsas);
	static struct model_s* terror = g_Engine.CL_LoadModel("models/player/terror/terror.mdl", &modelindexterror);
	static struct model_s* urban = g_Engine.CL_LoadModel("models/player/urban/urban.mdl", &modelindexurban);
	static struct model_s* vip = g_Engine.CL_LoadModel("models/player/vip/vip.mdl", &modelindexvip);

	if (modelmenu && bAliveLocal() && cvar.model_preview && DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
	{
		int modelindex;
		struct model_s* mod;

		mod = arctic, modelindex = modelindexarctic;
		if (cvar.model_type == 1)
			mod = gign, modelindex = modelindexgign;
		if (cvar.model_type == 2)
			mod = gsg9, modelindex = modelindexgsg9;
		if (cvar.model_type == 3)
			mod = guerilla, modelindex = modelindexguerilla;
		if (cvar.model_type == 4)
			mod = leet, modelindex = modelindexleet;
		if (cvar.model_type == 5)
			mod = sas, modelindex = modelindexsas;
		if (cvar.model_type == 6)
			mod = terror, modelindex = modelindexterror;
		if (cvar.model_type == 7)
			mod = urban, modelindex = modelindexurban;
		if (cvar.model_type == 8)
			mod = vip, modelindex = modelindexvip;
		
		Vector viewangle;
		g_Engine.GetViewAngles(viewangle);
		static cl_entity_s playerdummy;
		playerdummy.model = mod;
		playerdummy.curstate.modelindex = modelindex;
		playerdummy.curstate.angles = Vector(viewangle.x, viewangle.y + 180, viewangle.z);
		playerdummy.curstate.sequence = 4;
		playerdummy.curstate.framerate = cvar.model_move;
		playerdummy.curstate.messagenum = -1337;
		playerdummy.origin = screenshit(viewangle);
		g_Engine.CL_CreateVisibleEntity(ET_PLAYER, &playerdummy);
	}

	g_Client.HUD_CreateEntities();
}

void HookClientFunctions()
{
	g_pClient->HUD_CreateEntities = HUD_CreateEntities;
	g_pClient->HUD_AddEntity = HUD_AddEntity;
	g_pClient->HUD_Frame = HUD_Frame;
	g_pClient->HUD_Redraw = HUD_Redraw;
	g_pClient->CL_CreateMove = CL_CreateMove;
	g_pClient->HUD_PlayerMoveInit = HUD_PlayerMoveInit;
	g_pClient->V_CalcRefdef = V_CalcRefdef;
	g_pClient->HUD_PostRunCmd = HUD_PostRunCmd;
	g_pClient->HUD_Key_Event = HUD_Key_Event;
	g_pClient->HUD_ProcessPlayerState = HUD_ProcessPlayerState;
	g_pClient->HUD_GetHullBounds = HUD_GetHullBounds;
	g_pClient->CL_IsThirdPerson = CL_IsThirdPerson;
}