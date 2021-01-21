#include "client.h"

cl_entity_s playerdummy;
cl_entity_s playerdummys[33];

bool CalcForward(Vector vForward, float& forwards, Vector vRight, float rights, Vector vUp, float ups)
{
	for (unsigned int i = 0; i < 65535; i++)
	{
		Vector origin1 = pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * ((cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups) - 32);
		Vector origin2 = pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * ((cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups) + 32);

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
		Vector origin = pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * (cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups);

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
		Vector origin = pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * (cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups);

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
		return true;
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
		Vector origin = pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * (cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups);
		if (!WorldToScreen(origin))
			forwards = 100, rights = 0, ups = 0;
		if (CalcForward(vForward, forwards, vRight, rights, vUp, ups) && CalcRight(x, y, vForward, forwards, vRight, rights, vUp, ups) && CalcUp(x, y, vForward, forwards, vRight, rights, vUp, ups))
			loadmodel = false;
	}

	return pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * (cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups);
}

void Playerdummy()
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
		playerdummy.model = mod;
		playerdummy.curstate.modelindex = modelindex;
		playerdummy.curstate.angles = Vector(viewangle.x, viewangle.y + 180, viewangle.z);
		playerdummy.curstate.sequence = 4;
		playerdummy.curstate.framerate = cvar.model_move;
		playerdummy.curstate.body = 1;
		playerdummy.origin = screenshit(viewangle);
		g_Engine.CL_CreateVisibleEntity(ET_PLAYER, &playerdummy);
	}
}

bool bAliveAll(cl_entity_s* ent)
{
	if (g_Player[ent->index].bAliveInScoreTab &&
		!(g_Engine.GetEntityByIndex(pmove->player_index + 1)->curstate.iuser1 == OBS_IN_EYE && g_Engine.GetEntityByIndex(pmove->player_index + 1)->curstate.iuser2 == ent->index))
		return true;
	return false;
}

void fakeshit(cl_entity_s* ent)
{
	if (ent && ent->player && bAliveAll(ent) && cvar.fake_shit && DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
	{
		playerdummys[ent->index].index = ent->index;
		playerdummys[ent->index].player = ent->player;
		playerdummys[ent->index].model = ent->model;
		playerdummys[ent->index].angles = Vector(-ent->curstate.angles.x * 3, ent->curstate.angles.y, ent->curstate.angles.z);
		playerdummys[ent->index].origin = ent->curstate.origin;
		playerdummys[ent->index].curstate = ent->curstate;
		playerdummys[ent->index].curstate.framerate = 0;
		playerdummys[ent->index].curstate.frame = 0;
		g_Engine.CL_CreateVisibleEntity(ET_PLAYER, &playerdummys[ent->index]);
	}
}