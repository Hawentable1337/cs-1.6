#include "client.h"

cl_entity_s playerdummy;

deque<playermodel_t> PlayerModel;

float modelscreenw = 100, modelscreenh = 100;
int model_type;
float esph;
float model_pos_x = 100, model_pos_y = 100;
float modelscale = 0.01;

void Playerdummy()
{
	if (bShowMenu && (pmove->iuser1 == OBS_NONE || pmove->iuser1 == OBS_ROAMING) && cvar.model_preview && DrawVisuals && GetTickCount() - HudRedraw <= 100)
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

		float screen[] = { model_pos_x + (modelscreenw / 2), model_pos_y + (modelscreenh / 2) + 40 };
		Vector world;
		if (!ScreenToWorld(screen, world) || !modelmenu)
			return;

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
			playerdummy.curstate.angles = Vector(pmove->angles.x, pmove->angles.y + 180, pmove->angles.z);
			playerdummy.curstate.sequence = 1;
			playerdummy.origin = world;

			g_Engine.CL_CreateVisibleEntity(ET_TEMPENTITY, &playerdummy);
		}
	}
}