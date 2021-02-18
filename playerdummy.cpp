#include "client.h"

cl_entity_s playerdummy;

deque<playermodel_t> PlayerModel;

float modelscreenw = 200, modelscreenh = 300;
int model_type;
float espx, espy, espw, esph;
float model_pos_x = 100, model_pos_y = 100;
float modelscale = 0.03;
bool drawdummy = false;
bool drawgetdummy = false;
bool drawbackdummy = false;

float vViewanglesDummy[3];

void GetDummyAngle(ref_params_s* pparams)
{
	vViewanglesDummy[0] = pparams->viewangles[0];
	vViewanglesDummy[1] = pparams->viewangles[1] + 180;
	vViewanglesDummy[2] = -pparams->viewangles[2];
}

void DrawDummyBack()
{
	if (drawbackdummy)
	{
		Vector world1, screen1 = Vector(model_pos_x, model_pos_y, 0);
		Vector world2, screen2 = Vector(model_pos_x + modelscreenw, model_pos_y, 0);
		Vector world3, screen3 = Vector(model_pos_x + modelscreenw, model_pos_y + modelscreenh, 0);
		Vector world4, screen4 = Vector(model_pos_x, model_pos_y + modelscreenh, 0);
		ScreenToWorld(screen1, world1);
		ScreenToWorld(screen2, world2);
		ScreenToWorld(screen3, world3);
		ScreenToWorld(screen4, world4);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.06f, 0.06f, 0.06f, 0.94f);
		glBegin(GL_QUADS);
		glVertex3fv(world1);
		glVertex3fv(world2);
		glVertex3fv(world3);
		glVertex3fv(world4);
		glEnd();
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
		drawbackdummy = false;
	}
}

void GetDummyModels()
{
	if (drawgetdummy)
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
		drawgetdummy = false;
	}
}

void Playerdummy()
{
	if (drawdummy)
	{
		Vector world, screen = Vector(model_pos_x + (modelscreenw / 2), model_pos_y + (modelscreenh / 2) + 40, 0);
		ScreenToWorld(screen, world);

		if (PlayerModel.size() && model_type > PlayerModel.size() - 1)
			model_type = PlayerModel.size() - 1;
		if (PlayerModel.size() == 0)
			model_type = 0;

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
			playerdummy.curstate.angles = vViewanglesDummy;
			playerdummy.curstate.sequence = 1;
			playerdummy.origin = world;

			g_Engine.CL_CreateVisibleEntity(ET_TEMPENTITY, &playerdummy);
		}
		drawdummy = false;
	}
}