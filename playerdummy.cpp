#include "client.h"

cl_entity_s playerdummy;

deque<playermodel_t> PlayerModel;

float modelscreenw, modelscreenh;
int model_type;
float esph;
float model_pos_x = 100, model_pos_y = 100;

void CalcForward(Vector vForward, float& forwards, Vector vRight, float rights, Vector vUp, float ups)
{
	float screensizex = modelscreenw * 1.5f / 2.f;
	float screensizey = modelscreenh / 2.f;
	
	float screensize;
	if (screensizex < screensizey)
		screensize = screensizex;
	if (screensizex >= screensizey)
		screensize = screensizey;

	if (int(esph) < int(screensize - 1))
		forwards -= 0.1f * (screensize - esph);
	if (int(esph) > int(screensize + 1))
		forwards += 0.1f * (esph - screensize);
}

void CalcRight(float x, float y, Vector vForward, float forwards, Vector vRight, float& rights, Vector vUp, float ups)
{
	for (int i = 0; i < 8; i++)
	{
		Vector origin = pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * (cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups);

		float screen[2];

		g_Engine.pTriAPI->WorldToScreen(origin, screen);
		screen[0] = screen[0] * (ImGui::GetIO().DisplaySize.x / 2) + (ImGui::GetIO().DisplaySize.x / 2);
		screen[1] = -screen[1] * (ImGui::GetIO().DisplaySize.y / 2) + (ImGui::GetIO().DisplaySize.y / 2);

		if (int(screen[0]) < int(x))
			rights += 0.1f * (x - screen[0]);
		else if (int(screen[0]) > int(x))
			rights -= 0.1f * (screen[0] - x);
		else
			break;
	}
}

void CalcUp(float x, float y, Vector vForward, float forwards, Vector vRight, float rights, Vector vUp, float& ups)
{
	for (int i = 0; i < 8; i++)
	{
		Vector origin = pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * (cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups);

		float screen[2];

		g_Engine.pTriAPI->WorldToScreen(origin, screen);
		screen[0] = screen[0] * (ImGui::GetIO().DisplaySize.x / 2) + (ImGui::GetIO().DisplaySize.x / 2);
		screen[1] = -screen[1] * (ImGui::GetIO().DisplaySize.y / 2) + (ImGui::GetIO().DisplaySize.y / 2);

		if (int(screen[1]) < int(y))
			ups += 0.1f * (y - screen[1]);
		else if (int(screen[1]) > int(y))
			ups -= 0.1f * (screen[1] - y);
		else
			break;
	}
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
	static float forwards = 250, rights = 0, ups = 0;
	float x = model_pos_x + (modelscreenw / 2), y = model_pos_y + (modelscreenh / 2) + 40;
	Vector vForward, vRight, vUp;
	g_Engine.pfnAngleVectors(viewangle, vForward, vRight, vUp);
	Vector origin = pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * (cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups);
	if (!WorldToScreen(origin))
		forwards = 150, rights = 0, ups = 0;
	CalcForward(vForward, forwards, vRight, rights, vUp, ups);
	CalcRight(x, y, vForward, forwards, vRight, rights, vUp, ups);
	CalcUp(x, y, vForward, forwards, vRight, rights, vUp, ups);
	return pmove->origin + pmove->view_ofs + vForward * (cvar.visual_chase_cam ? forwards - cvar.visual_chase_back : forwards) + vRight * rights - vUp * (cvar.visual_chase_cam ? ups - cvar.visual_chase_up : ups);
}

void Playerdummy()
{
	if (bShowMenu && (pmove->iuser1 == OBS_NONE || pmove->iuser1 == OBS_ROAMING) && cvar.model_preview && DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
	{
		for (int i = 0; i < 256; i++)
		{
			model_t* mod = g_Studio.GetModelByIndex(i);
			if (!mod || !strstr(mod->name, "/player/")) continue;
			bool saved = false;
			for (playermodel_t Model : PlayerModel)
			{
				char out[256];
				strcpy(out, getfilename(mod->name).c_str());
				int len = strlen(out);
				if (len > 1)out[len - 1] = (char)0;
				if (strstr(Model.mod->name, mod->name) || !strcmp(getfilename(Model.mod->name).c_str(), out))
					saved = true;
			}
			if (saved) continue;

			playermodel_t model;
			model.mod = mod;
			PlayerModel.push_back(model);
		}

		if (PlayerModel.size() && model_type > PlayerModel.size() - 1)
			model_type = PlayerModel.size() - 1;
		if (PlayerModel.size() == 0)
			model_type = 0;

		Vector viewangle;
		g_Engine.GetViewAngles(viewangle);
		Vector origin = screenshit(viewangle);

		if (!modelmenu)return;
		
		for (int i = 0; i < PlayerModel.size(); i++)
		{
			struct model_s* mod = g_Engine.CL_LoadModel(PlayerModel[i].mod->name, NULL);
			if (!mod)
			{
				if (model_type > i)
					model_type--;
				PlayerModel.erase(PlayerModel.begin() + i);
				continue;
			}

			if (i != model_type)
				continue;
			playerdummy.model = mod;
			playerdummy.curstate.angles = Vector(viewangle.x, viewangle.y + 180, viewangle.z);
			playerdummy.curstate.sequence = 1;
			playerdummy.origin = origin;
			g_Engine.CL_CreateVisibleEntity(ET_TEMPENTITY, &playerdummy);
		}
	}
}