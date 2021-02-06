#include "client.h"

deque<worldbone_t> WorldBone;
deque<worldhitbox_t> WorldHitbox;

deque<playerbone_t> PlayerBone;
deque<playerhitbox_t> PlayerHitbox;
deque<playerhitboxnum_t> PlayerHitboxNum;

int HeadBox[33];
int HitboxBone[33];

void VectorTransform(Vector in1, float in2[3][4], float* out)
{
	out[0] = in1.Dot(in2[0]) + in2[0][3];
	out[1] = in1.Dot(in2[1]) + in2[1][3];
	out[2] = in1.Dot(in2[2]) + in2[2][3];
}

bool IsSHield(Vector* origin)
{
	int from[12] = { 7, 23, 7, 44, 23, 7, 23, 44, 44, 44, 7, 23 };
	int to[12] = { 8, 27, 8, 52, 27, 8, 27, 52, 52, 52, 8, 27 };
	for (unsigned int x = 0; x < 12; x++)
	{
		int distance = round((origin[SkeletonHitboxMatrix[x][0]]).Distance(origin[SkeletonHitboxMatrix[x][1]]));
		if (distance < from[x] || distance > to[x]) return false;
	}
	return true;
}

void GetHitboxes(cl_entity_s* ent)
{
	if (ent && ent->model && ent->model->name)
	{
		studiohdr_t* pStudioHeader = (studiohdr_t*)g_Studio.Mod_Extradata(ent->model);
		if (!pStudioHeader)
			return;
		mstudiobbox_t* pHitbox = (mstudiobbox_t*)((byte*)pStudioHeader + pStudioHeader->hitboxindex);
		mstudiobone_t* pbones = (mstudiobone_t*)((byte*)pStudioHeader + pStudioHeader->boneindex);
		BoneMatrix_t* pBoneMatrix = (BoneMatrix_t*)g_Studio.StudioGetBoneTransform();

		if (!strstr(ent->model->name, "/player.mdl") && !strstr(ent->model->name, "/player/") && !strstr(ent->model->name, "/p_") && ent != g_Engine.GetViewModel())
		{
			if (cvar.skeleton_world_bone && pBoneMatrix && pbones)
			{
				for (unsigned int i = 0; i < pStudioHeader->numbones; i++)
				{
					worldbone_t Bones;
					Bones.vBone[0] = (*pBoneMatrix)[i][0][3];
					Bones.vBone[1] = (*pBoneMatrix)[i][1][3];
					Bones.vBone[2] = (*pBoneMatrix)[i][2][3];
					Bones.vBoneParent[0] = (*pBoneMatrix)[pbones[i].parent][0][3];
					Bones.vBoneParent[1] = (*pBoneMatrix)[pbones[i].parent][1][3];
					Bones.vBoneParent[2] = (*pBoneMatrix)[pbones[i].parent][2][3];
					Bones.parent = pbones[i].parent;
					WorldBone.push_back(Bones);
				}
			}
			worldesp_t Esp;
			Esp.index = ent->curstate.owner;
			Esp.origin = ent->origin;
			strcpy(Esp.name, getfilename(ent->model->name).c_str());
			if (pBoneMatrix && pHitbox)
			{
				for (unsigned int i = 0; i < pStudioHeader->numhitboxes; i++)
				{
					Vector vBBMax, vBBMin, vCubePointsTrans[8], vCubePoints[8];
					Vector vEye = pmove->origin + pmove->view_ofs;
					VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vBBMin);
					VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vBBMax);

					vCubePoints[0] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
					vCubePoints[1] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
					vCubePoints[2] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
					vCubePoints[3] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
					vCubePoints[4] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
					vCubePoints[5] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
					vCubePoints[6] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
					vCubePoints[7] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
					for (unsigned int x = 0; x < 8; x++)
						VectorTransform(vCubePoints[x], (*pBoneMatrix)[pHitbox[i].bone], vCubePointsTrans[x]);

					if (cvar.skeleton_world_hitbox)
					{
						worldhitbox_t Hitboxes;
						for (unsigned int x = 0; x < 8; x++)
							Hitboxes.vCubePointsTrans[x] = vCubePointsTrans[x];
						WorldHitbox.push_back(Hitboxes);
					}

					esphitbox_t EspHitbox;
					for (unsigned int x = 0; x < 8; x++)
						EspHitbox.HitboxMulti[x] = vCubePointsTrans[x];
					Esp.WorldEspHitbox.push_back(EspHitbox);
				}
			}
			WorldEsp.push_back(Esp);
		}
		if (ent == g_Engine.GetViewModel())
		{
			if (cvar.skeleton_view_model_bone && pBoneMatrix && pbones && DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
			{
				glDisable(GL_TEXTURE_2D);
				glEnable(GL_BLEND);
				glEnable(GL_LINE_SMOOTH);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				for (unsigned int i = 0; i < pStudioHeader->numbones; i++)
				{
					if (pbones[i].parent >= 0)
					{
						glLineWidth(1);
						glColor4f(color_red, color_green, color_blue, 1.f);
						glBegin(GL_LINES);
						glVertex3f((*pBoneMatrix)[pbones[i].parent][0][3], (*pBoneMatrix)[pbones[i].parent][1][3], (*pBoneMatrix)[pbones[i].parent][2][3]);
						glVertex3f((*pBoneMatrix)[i][0][3], (*pBoneMatrix)[i][1][3], (*pBoneMatrix)[i][2][3]);
						glEnd();

						glPointSize(3);
						glColor4f(color_blue, color_red, color_green, 1.f);
						glBegin(GL_POINTS);
						if (pbones[pbones[i].parent].parent != -1)
							glVertex3f((*pBoneMatrix)[pbones[i].parent][0][3], (*pBoneMatrix)[pbones[i].parent][1][3], (*pBoneMatrix)[pbones[i].parent][2][3]);
						glVertex3f((*pBoneMatrix)[i][0][3], (*pBoneMatrix)[i][1][3], (*pBoneMatrix)[i][2][3]);
						glEnd();
					}
					else
					{
						glPointSize(3);
						glColor4f(color_blue, color_red, color_green, 1.f);
						glBegin(GL_POINTS);
						glVertex3f((*pBoneMatrix)[i][0][3], (*pBoneMatrix)[i][1][3], (*pBoneMatrix)[i][2][3]);
						glEnd();
					}
				}
				glEnable(GL_TEXTURE_2D);
				glDisable(GL_BLEND);
				glDisable(GL_LINE_SMOOTH);
			}
			if (cvar.skeleton_view_model_hitbox && pBoneMatrix && pHitbox && DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
			{
				glDisable(GL_TEXTURE_2D);
				glEnable(GL_BLEND);
				glEnable(GL_LINE_SMOOTH);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				for (unsigned int i = 0; i < pStudioHeader->numhitboxes; i++)
				{
					Vector vCubePointsTrans[8], vCubePoints[8];

					vCubePoints[0] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
					vCubePoints[1] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
					vCubePoints[2] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
					vCubePoints[3] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
					vCubePoints[4] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
					vCubePoints[5] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
					vCubePoints[6] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
					vCubePoints[7] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);

					for (unsigned int x = 0; x < 8; x++)
						VectorTransform(vCubePoints[x], (*pBoneMatrix)[pHitbox[i].bone], vCubePointsTrans[x]);

					glLineWidth(1);
					glColor4f(color_blue, color_red, color_green, 1.f);
					for (unsigned int x = 0; x < 12; x++)
					{
						glBegin(GL_LINES);
						glVertex3fv(vCubePointsTrans[SkeletonHitboxMatrix[x][0]]);
						glVertex3fv(vCubePointsTrans[SkeletonHitboxMatrix[x][1]]);
						glEnd();
					}
					glPointSize(3);
					glColor4f(color_red, color_green, color_blue, 1.f);
					glBegin(GL_POINTS);
					for (unsigned int x = 0; x < 8; x++)
						glVertex3fv(vCubePointsTrans[x]);
					glEnd();
				}
				glEnable(GL_TEXTURE_2D);
				glDisable(GL_BLEND);
				glDisable(GL_LINE_SMOOTH);
			}
		}
	}

	if (ent && !ent->player && ent->model && ent->model->name && strstr(ent->model->name, "/player/") && ent->curstate.owner > 0 && ent->curstate.owner <= g_Engine.GetMaxClients())
	{
		studiohdr_t* pStudioHeader = (studiohdr_t*)g_Studio.Mod_Extradata(ent->model);
		if (!pStudioHeader)
			return;
		mstudiobbox_t* pHitbox = (mstudiobbox_t*)((byte*)pStudioHeader + pStudioHeader->hitboxindex);
		mstudiobone_t* pbones = (mstudiobone_t*)((byte*)pStudioHeader + pStudioHeader->boneindex);
		BoneMatrix_t* pBoneMatrix = (BoneMatrix_t*)g_Studio.StudioGetBoneTransform();

		HitboxBone[ent->curstate.owner] = -1;
		if (pBoneMatrix && pbones)
		{
			for (unsigned int i = 0; i < pStudioHeader->numbones; i++)
			{
				char modelname[255];
				strcpy(modelname, pbones[i].name);
				strlwr(modelname);

				if (strstr(modelname, "head"))
				{
					HitboxBone[ent->curstate.owner] = i;
					break;
				}
			}
		}

		if (HitboxBone[ent->curstate.owner] != -1)
		{
			bool found = false;
			if (pBoneMatrix && pHitbox)
			{
				for (unsigned int i = 0; i < pStudioHeader->numhitboxes; i++)
				{
					if (pHitbox[i].bone == HitboxBone[ent->curstate.owner])
					{
						found = true;
						HeadBox[ent->curstate.owner] = i;
						break;
					}
				}
			}
			if (!found)HeadBox[ent->curstate.owner] = 0;
		}
		else
			HeadBox[ent->curstate.owner] = 0;

		if (pBoneMatrix && pbones && cvar.skeleton_player_bone)
		{
			for (unsigned int i = 0; i < pStudioHeader->numbones; i++)
			{
				if (pbones[i].parent >= 0)
				{
					playerbone_t Bones;
					Bones.vBone[0] = (*pBoneMatrix)[i][0][3];
					Bones.vBone[1] = (*pBoneMatrix)[i][1][3];
					Bones.vBone[2] = (*pBoneMatrix)[i][2][3];
					Bones.vBoneParent[0] = (*pBoneMatrix)[pbones[i].parent][0][3];
					Bones.vBoneParent[1] = (*pBoneMatrix)[pbones[i].parent][1][3];
					Bones.vBoneParent[2] = (*pBoneMatrix)[pbones[i].parent][2][3];
					Bones.index = ent->curstate.owner;
					Bones.dummy = false;
					PlayerBone.push_back(Bones);
				}
			}
		}
		if (pBoneMatrix && pHitbox)
		{
			playeresp_t Esp;
			Esp.index = ent->curstate.owner;
			Esp.origin = ent->origin;
			Esp.sequence = ent->curstate.sequence;
			Esp.weaponmodel = ent->curstate.weaponmodel;
			strcpy(Esp.model, getfilename(ent->model->name).c_str());
			Esp.dummy = false;

			playeraim_t Aim;
			Aim.index = ent->curstate.owner;
			Aim.origin = ent->origin;
			Aim.sequence = ent->curstate.sequence;
			strcpy(Aim.modelname, ent->model->name);

			int numhitboxes = 0;
			for (unsigned int i = 0; i < pStudioHeader->numhitboxes; i++)
			{
				Vector vBBMax, vBBMin, vCubePointsTrans[8], vCubePoints[8];
				Vector vEye = pmove->origin + pmove->view_ofs;
				VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vBBMin);
				VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vBBMax);

				vCubePoints[0] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
				vCubePoints[1] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
				vCubePoints[2] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
				vCubePoints[3] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
				vCubePoints[4] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
				vCubePoints[5] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
				vCubePoints[6] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
				vCubePoints[7] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
				for (unsigned int x = 0; x < 8; x++)
					VectorTransform(vCubePoints[x], (*pBoneMatrix)[pHitbox[i].bone], vCubePointsTrans[x]);

				if (!IsSHield(vCubePointsTrans))
				{
					if (cvar.visual_model_hitbox)
					{
						playerhitboxnum_t HitboxesNum;
						HitboxesNum.HitboxPos = (vBBMax + vBBMin) * 0.5f;
						HitboxesNum.Hitbox = i;
						HitboxesNum.dummy = false;
						PlayerHitboxNum.push_back(HitboxesNum);
					}

					if (cvar.skeleton_player_hitbox)
					{
						playerhitbox_t Hitboxes;
						Hitboxes.index = ent->curstate.owner;
						Hitboxes.dummy = false;
						for (unsigned int x = 0; x < 8; x++)
							Hitboxes.vCubePointsTrans[x] = vCubePointsTrans[x];
						PlayerHitbox.push_back(Hitboxes);
					}

					esphitbox_t EspHitbox;
					for (unsigned int x = 0; x < 8; x++)
						EspHitbox.HitboxMulti[x] = vCubePointsTrans[x];
					Esp.PlayerEspHitbox.push_back(EspHitbox);

					playeraimhitbox_t AimHitbox;
					for (unsigned int x = 0; x < 8; x++)
					{
						AimHitbox.HitboxMulti[x] = vCubePointsTrans[x];
						AimHitbox.HitboxPointsFOV[x] = g_Local.vPrevForward.AngleBetween(AimHitbox.HitboxMulti[x] - vEye);
					}
					AimHitbox.Hitbox = (vBBMax + vBBMin) * 0.5f;
					AimHitbox.HitboxFOV = g_Local.vPrevForward.AngleBetween(AimHitbox.Hitbox - vEye);
					Aim.PlayerAimHitbox.push_back(AimHitbox);

					numhitboxes++;
				}
			}
			PlayerEsp.push_back(Esp);
			PlayerAim.push_back(Aim);

			if (cvar.model_scan)
			{
				bool saved = false;
				for (model_aim_t Models : Model_Aim)
				{
					if (strstr(Models.checkmodel, ent->model->name))
					{
						saved = true;
						break;
					}
				}
				if (!saved)
				{
					model_aim_t Model;
					Model.numhitboxes = numhitboxes;
					strcpy(Model.displaymodel, getfilename(ent->model->name).c_str());
					strcpy(Model.checkmodel, ent->model->name);
					Model_Aim.push_back(Model);
				}
			}
		}
	}

	if (ent && ent->player && ent->index > 0 && ent->index <= g_Engine.GetMaxClients())
	{
		model_t* pModel = g_Studio.SetupPlayerModel(ent->index - 1);
		if (!pModel)
			return;
		studiohdr_t* pStudioHeader = (studiohdr_t*)g_Studio.Mod_Extradata(pModel);
		mstudiobbox_t* pHitbox = (mstudiobbox_t*)((byte*)pStudioHeader + pStudioHeader->hitboxindex);
		mstudiobone_t* pbones = (mstudiobone_t*)((byte*)pStudioHeader + pStudioHeader->boneindex);
		BoneMatrix_t* pBoneMatrix = (BoneMatrix_t*)g_Studio.StudioGetBoneTransform();

		HitboxBone[ent->index] = -1;
		if (pBoneMatrix && pbones)
		{
			for (unsigned int i = 0; i < pStudioHeader->numbones; i++)
			{
				char modelname[255];
				strcpy(modelname, pbones[i].name);
				strlwr(modelname);

				if (strstr(modelname, "head"))
				{
					HitboxBone[ent->index] = i;
					break;
				}
			}
		}

		if (HitboxBone[ent->index] != -1)
		{
			bool found = false;
			if (pBoneMatrix && pHitbox)
			{
				for (unsigned int i = 0; i < pStudioHeader->numhitboxes; i++)
				{
					if (pHitbox[i].bone == HitboxBone[ent->index])
					{
						found = true;
						HeadBox[ent->index] = i;
						break;
					}
				}
			}
			if (!found)HeadBox[ent->index] = 0;
		}
		else
			HeadBox[ent->index] = 0;

		if (pBoneMatrix && pbones && cvar.skeleton_player_bone)
		{
			for (unsigned int i = 0; i < pStudioHeader->numbones; i++)
			{
				if (pbones[i].parent >= 0)
				{
					playerbone_t Bones;
					Bones.vBone[0] = (*pBoneMatrix)[i][0][3];
					Bones.vBone[1] = (*pBoneMatrix)[i][1][3];
					Bones.vBone[2] = (*pBoneMatrix)[i][2][3];
					Bones.vBoneParent[0] = (*pBoneMatrix)[pbones[i].parent][0][3];
					Bones.vBoneParent[1] = (*pBoneMatrix)[pbones[i].parent][1][3];
					Bones.vBoneParent[2] = (*pBoneMatrix)[pbones[i].parent][2][3];
					Bones.index = ent->index;
					Bones.dummy = false;
					PlayerBone.push_back(Bones);
				}
			}
		}
		if (pBoneMatrix && pHitbox)
		{
			playeresp_t Esp;
			Esp.index = ent->index;
			Esp.origin = ent->origin;
			Esp.sequence = ent->curstate.sequence;
			Esp.weaponmodel = ent->curstate.weaponmodel;
			strcpy(Esp.model, getfilename(pModel->name).c_str());
			Esp.dummy = false;

			playeraim_t Aim;
			Aim.index = ent->index;
			Aim.origin = ent->origin;
			Aim.sequence = ent->curstate.sequence;
			strcpy(Aim.modelname, pModel->name);
			
			int numhitboxes = 0;
			for (unsigned int i = 0; i < pStudioHeader->numhitboxes; i++)
			{
				Vector vBBMax, vBBMin, vCubePointsTrans[8], vCubePoints[8];
				Vector vEye = pmove->origin + pmove->view_ofs;
				VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vBBMin);
				VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vBBMax);

				vCubePoints[0] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
				vCubePoints[1] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
				vCubePoints[2] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
				vCubePoints[3] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
				vCubePoints[4] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
				vCubePoints[5] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
				vCubePoints[6] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
				vCubePoints[7] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
				for (unsigned int x = 0; x < 8; x++)
					VectorTransform(vCubePoints[x], (*pBoneMatrix)[pHitbox[i].bone], vCubePointsTrans[x]);

				if (!IsSHield(vCubePointsTrans))
				{
					if (cvar.visual_model_hitbox)
					{
						playerhitboxnum_t HitboxesNum;
						HitboxesNum.HitboxPos = (vBBMax + vBBMin) * 0.5f;
						HitboxesNum.Hitbox = i;
						HitboxesNum.dummy = false;
						PlayerHitboxNum.push_back(HitboxesNum);
					}

					if (cvar.skeleton_player_hitbox)
					{
						playerhitbox_t Hitboxes;
						Hitboxes.index = ent->index;
						Hitboxes.dummy = false;
						for (unsigned int x = 0; x < 8; x++)
							Hitboxes.vCubePointsTrans[x] = vCubePointsTrans[x];
						PlayerHitbox.push_back(Hitboxes);
					}

					esphitbox_t EspHitbox;
					for (unsigned int x = 0; x < 8; x++)
						EspHitbox.HitboxMulti[x] = vCubePointsTrans[x];
					Esp.PlayerEspHitbox.push_back(EspHitbox);

					playeraimhitbox_t AimHitbox;
					for (unsigned int x = 0; x < 8; x++)
					{
						AimHitbox.HitboxMulti[x] = vCubePointsTrans[x];
						AimHitbox.HitboxPointsFOV[x] = g_Local.vPrevForward.AngleBetween(AimHitbox.HitboxMulti[x] - vEye);
					}
					AimHitbox.Hitbox = (vBBMax + vBBMin) * 0.5f;
					AimHitbox.HitboxFOV = g_Local.vPrevForward.AngleBetween(AimHitbox.Hitbox - vEye);
					Aim.PlayerAimHitbox.push_back(AimHitbox);

					numhitboxes++;
				}
			}
			PlayerEsp.push_back(Esp);
			PlayerAim.push_back(Aim);

			if (cvar.model_scan)
			{
				bool saved = false;
				for (model_aim_t Models : Model_Aim)
				{
					if (strstr(Models.checkmodel, pModel->name))
					{
						saved = true;
						break;
					}
				}
				if (!saved)
				{
					model_aim_t Model;
					Model.numhitboxes = numhitboxes;
					strcpy(Model.displaymodel, getfilename(pModel->name).c_str());
					strcpy(Model.checkmodel, pModel->name);
					Model_Aim.push_back(Model);
				}
			}
		}
	}

	if (ent && ent->model && ent == &playerdummy)
	{
		studiohdr_t* pStudioHeader = (studiohdr_t*)g_Studio.Mod_Extradata(ent->model);
		mstudiobbox_t* pHitbox = (mstudiobbox_t*)((byte*)pStudioHeader + pStudioHeader->hitboxindex);
		mstudiobone_t* pbones = (mstudiobone_t*)((byte*)pStudioHeader + pStudioHeader->boneindex);
		BoneMatrix_t* pBoneMatrix = (BoneMatrix_t*)g_Studio.StudioGetBoneTransform();
		if (pBoneMatrix && pbones && cvar.skeleton_player_bone)
		{
			for (unsigned int i = 0; i < pStudioHeader->numbones; i++)
			{
				if (pbones[i].parent >= 0)
				{
					playerbone_t Bones;
					Bones.vBone[0] = (*pBoneMatrix)[i][0][3];
					Bones.vBone[1] = (*pBoneMatrix)[i][1][3];
					Bones.vBone[2] = (*pBoneMatrix)[i][2][3];
					Bones.vBoneParent[0] = (*pBoneMatrix)[pbones[i].parent][0][3];
					Bones.vBoneParent[1] = (*pBoneMatrix)[pbones[i].parent][1][3];
					Bones.vBoneParent[2] = (*pBoneMatrix)[pbones[i].parent][2][3];
					Bones.index = ent->index;
					Bones.dummy = true;
					PlayerBone.push_back(Bones);
				}
			}
		}
		if (pBoneMatrix && pHitbox)
		{
			playeresp_t Esp;
			Esp.index = ent->index;
			Esp.origin = ent->origin;
			Esp.dummy = true;

			for (unsigned int i = 0; i < pStudioHeader->numhitboxes; i++)
			{
				Vector vBBMax, vBBMin, vCubePointsTrans[8], vCubePoints[8];
				Vector vEye = pmove->origin + pmove->view_ofs;
				VectorTransform(pHitbox[i].bbmin, (*pBoneMatrix)[pHitbox[i].bone], vBBMin);
				VectorTransform(pHitbox[i].bbmax, (*pBoneMatrix)[pHitbox[i].bone], vBBMax);

				vCubePoints[0] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
				vCubePoints[1] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
				vCubePoints[2] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmin.z);
				vCubePoints[3] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmin.z);
				vCubePoints[4] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
				vCubePoints[5] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmax.y, pHitbox[i].bbmax.z);
				vCubePoints[6] = Vector(pHitbox[i].bbmin.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
				vCubePoints[7] = Vector(pHitbox[i].bbmax.x, pHitbox[i].bbmin.y, pHitbox[i].bbmax.z);
				for (unsigned int x = 0; x < 8; x++)
					VectorTransform(vCubePoints[x], (*pBoneMatrix)[pHitbox[i].bone], vCubePointsTrans[x]);

				if (!IsSHield(vCubePointsTrans))
				{
					if (cvar.visual_model_hitbox)
					{
						playerhitboxnum_t HitboxesNum;
						HitboxesNum.HitboxPos = (vBBMax + vBBMin) * 0.5f;
						HitboxesNum.Hitbox = i;
						HitboxesNum.dummy = true;
						PlayerHitboxNum.push_back(HitboxesNum);
					}

					if (cvar.skeleton_player_hitbox)
					{
						playerhitbox_t PlayerHitboxes;
						PlayerHitboxes.index = ent->index;
						PlayerHitboxes.dummy = true;
						for (unsigned int x = 0; x < 8; x++)
							PlayerHitboxes.vCubePointsTrans[x] = vCubePointsTrans[x];
						PlayerHitbox.push_back(PlayerHitboxes);
					}

					esphitbox_t EspHitbox;
					for (unsigned int x = 0; x < 8; x++)
						EspHitbox.HitboxMulti[x] = vCubePointsTrans[x];
					Esp.PlayerEspHitbox.push_back(EspHitbox);
				}
			}
			PlayerEsp.push_back(Esp);
		}
	}
}

void DrawSkeletonPlayer()
{
	for (playerbone_t Bones : PlayerBone)
	{
		if (Bones.dummy)
			continue;
		ImColor Player = White();
		if (g_Player[Bones.index].iTeam == 1) Player = Red();
		if (g_Player[Bones.index].iTeam == 2) Player = Blue();
		float CalcAnglesMin[2], CalcAnglesMax[2];
		if (WorldToScreen(Bones.vBone, CalcAnglesMin) && WorldToScreen(Bones.vBoneParent, CalcAnglesMax))
			ImGui::GetCurrentWindow()->DrawList->AddLine({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, { IM_ROUND(CalcAnglesMax[0]), IM_ROUND(CalcAnglesMax[1]) }, Player);
	}
	for (playerhitbox_t Hitbox : PlayerHitbox)
	{
		if (Hitbox.dummy)
			continue;
		ImColor Player = White();
		if (g_Player[Hitbox.index].iTeam == 1) Player = Red();
		if (g_Player[Hitbox.index].iTeam == 2) Player = Blue();
		for (unsigned int x = 0; x < 12; x++)
		{
			float CalcAnglesMin[2], CalcAnglesMax[2];
			if (WorldToScreen(Hitbox.vCubePointsTrans[SkeletonHitboxMatrix[x][0]], CalcAnglesMin) && WorldToScreen(Hitbox.vCubePointsTrans[SkeletonHitboxMatrix[x][1]], CalcAnglesMax))
				ImGui::GetCurrentWindow()->DrawList->AddLine({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, { IM_ROUND(CalcAnglesMax[0]), IM_ROUND(CalcAnglesMax[1]) }, Player);
		}
	}
	for (playerbone_t Bones : PlayerBone)
	{
		if (!Bones.dummy)
			continue;
		ImColor Player = White();
		float CalcAnglesMin[2], CalcAnglesMax[2];
		if (WorldToScreen(Bones.vBone, CalcAnglesMin) && WorldToScreen(Bones.vBoneParent, CalcAnglesMax))
			ImGui::GetCurrentWindow()->DrawList->AddLine({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, { IM_ROUND(CalcAnglesMax[0]), IM_ROUND(CalcAnglesMax[1]) }, Player);
	}
	for (playerhitbox_t Hitbox : PlayerHitbox)
	{
		if (!Hitbox.dummy)
			continue;
		ImColor Player = White();
		for (unsigned int x = 0; x < 12; x++)
		{
			float CalcAnglesMin[2], CalcAnglesMax[2];
			if (WorldToScreen(Hitbox.vCubePointsTrans[SkeletonHitboxMatrix[x][0]], CalcAnglesMin) && WorldToScreen(Hitbox.vCubePointsTrans[SkeletonHitboxMatrix[x][1]], CalcAnglesMax))
				ImGui::GetCurrentWindow()->DrawList->AddLine({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, { IM_ROUND(CalcAnglesMax[0]), IM_ROUND(CalcAnglesMax[1]) }, Player);
		}
	}
	for (playerhitboxnum_t HitboxNum : PlayerHitboxNum)
	{
		if (HitboxNum.dummy)
			continue;
		float CalcAnglesMin[2];
		if (WorldToScreen(HitboxNum.HitboxPos, CalcAnglesMin))
		{
			char str[256];
			sprintf(str, "%d", HitboxNum.Hitbox);
			ImGui::GetCurrentWindow()->DrawList->AddText({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, White(), str);
		}
	}
	for (playerhitboxnum_t HitboxNum : PlayerHitboxNum)
	{
		if (!HitboxNum.dummy)
			continue;
		float CalcAnglesMin[2];
		if (WorldToScreen(HitboxNum.HitboxPos, CalcAnglesMin))
		{
			char str[256];
			sprintf(str, "%d", HitboxNum.Hitbox);
			ImGui::GetCurrentWindow()->DrawList->AddText({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, White(), str);
		}
	}
}

void DrawSkeletonWorld()
{
	for (worldbone_t Bones : WorldBone)
	{
		if (Bones.parent >= 0)
		{
			float CalcAnglesMin[2], CalcAnglesMax[2];
			if (WorldToScreen(Bones.vBone, CalcAnglesMin) && WorldToScreen(Bones.vBoneParent, CalcAnglesMax))
				ImGui::GetCurrentWindow()->DrawList->AddLine({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, { IM_ROUND(CalcAnglesMax[0]), IM_ROUND(CalcAnglesMax[1]) }, Wheel2());
			if (WorldToScreen(Bones.vBoneParent, CalcAnglesMin))
				ImGui::GetCurrentWindow()->DrawList->AddRectFilled({ IM_ROUND(CalcAnglesMin[0]) - 1, IM_ROUND(CalcAnglesMin[1]) - 1 }, { IM_ROUND(CalcAnglesMin[0]) + 2, IM_ROUND(CalcAnglesMin[1]) + 2 }, Wheel1());
			if (WorldToScreen(Bones.vBone, CalcAnglesMin))
				ImGui::GetCurrentWindow()->DrawList->AddRectFilled({ IM_ROUND(CalcAnglesMin[0]) - 1, IM_ROUND(CalcAnglesMin[1]) - 1 }, { IM_ROUND(CalcAnglesMin[0]) + 2, IM_ROUND(CalcAnglesMin[1]) + 2 }, Wheel1());
		}
		else
		{
			float CalcAnglesMin[2];
			if (WorldToScreen(Bones.vBone, CalcAnglesMin))
				ImGui::GetCurrentWindow()->DrawList->AddRectFilled({ IM_ROUND(CalcAnglesMin[0]) - 1, IM_ROUND(CalcAnglesMin[1]) - 1 }, { IM_ROUND(CalcAnglesMin[0]) + 2, IM_ROUND(CalcAnglesMin[1]) + 2 }, Wheel1());
		}
	}

	for (worldhitbox_t Hitbox : WorldHitbox)
	{
		for (unsigned int i = 0; i < 12; i++)
		{
			float CalcAnglesMin[2], CalcAnglesMax[2];
			if (WorldToScreen(Hitbox.vCubePointsTrans[SkeletonHitboxMatrix[i][0]], CalcAnglesMin) && WorldToScreen(Hitbox.vCubePointsTrans[SkeletonHitboxMatrix[i][1]], CalcAnglesMax))
				ImGui::GetCurrentWindow()->DrawList->AddLine({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, { IM_ROUND(CalcAnglesMax[0]), IM_ROUND(CalcAnglesMax[1]) }, Wheel1());
		}
		for (unsigned int i = 0; i < 8; i++)
		{
			float CalcAnglesMin[2];
			if (WorldToScreen(Hitbox.vCubePointsTrans[i], CalcAnglesMin))
				ImGui::GetCurrentWindow()->DrawList->AddRectFilled({ IM_ROUND(CalcAnglesMin[0]) - 1, IM_ROUND(CalcAnglesMin[1]) - 1 }, { IM_ROUND(CalcAnglesMin[0]) + 2, IM_ROUND(CalcAnglesMin[1]) + 2 }, Wheel2());
		}
	}
}