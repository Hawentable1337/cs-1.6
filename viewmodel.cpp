#include "client.h"

deque<viewmodel_t> ViewModel;
deque<viewmodeltexture_t> ViewModelTexture;

void ViewModelSkin()
{
	cl_entity_t* ent = g_Engine.GetViewModel();
	if (!ent || !ent->model)
		return;
	studiohdr_t* pStudioHeader = (studiohdr_t*)g_Studio.Mod_Extradata(ent->model);
	if (!pStudioHeader)
		return;
	mstudiobodyparts_t* pBodyPart = (mstudiobodyparts_t*)((byte*)pStudioHeader + pStudioHeader->bodypartindex);
	if (!pBodyPart)
		return;

	if (cvar.visual_skins_viewmodel_nohands && DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
	{
		for (int k = 0; k < pStudioHeader->numbodyparts; k++)
		{
			mstudiomodel_t* pSubModel = (mstudiomodel_t*)((byte*)pStudioHeader + pBodyPart[k].modelindex);
			for (int i = 0; i < pBodyPart[k].nummodels; i++)
			{
				short* pskinref = (short*)((byte*)pStudioHeader + pStudioHeader->skinindex);
				mstudiotexture_t* ptexture = (mstudiotexture_t*)((byte*)pStudioHeader + pStudioHeader->textureindex);
				mstudiomesh_t* pmesh = (mstudiomesh_t*)((byte*)pStudioHeader + pSubModel[i].meshindex);

				for (int j = pSubModel[i].nummesh - 1; j >= 0; j--)
				{
					for (viewmodeltexture_t Tex : ViewModelTexture)
					{
						if (strstr(ptexture[pskinref[pmesh[j].skinref]].name, Tex.name))
						{
							viewmodel_t meshes;
							strcpy(meshes.name, ptexture[pskinref[pmesh[j].skinref]].name);
							meshes.meshindex = j;
							meshes.pmesh = pmesh[j];
							meshes.numbodyparts = k;
							meshes.nummodels = i;
							meshes.nummesh = pSubModel[i].nummesh;
							strcpy(meshes.modelname, ent->model->name);
							ViewModel.push_back(meshes);

							//LogToFile("DISABLE %s", ptexture[pskinref[pmesh[j].skinref]].name);

							for (int x = j; x < pSubModel[i].nummesh - 1; x++)
							{
								//LogToFile("RECPLACING %s WITH %s", ptexture[pskinref[pmesh[x].skinref]].name, ptexture[pskinref[pmesh[x + 1].skinref]].name);
								pmesh[x] = pmesh[x + 1];
							}
							pSubModel[i].nummesh--;
						}
					}
				}
			}
		}
	}
	else
	{
		for (int i = ViewModel.size() - 1; i >= 0; i--)
		{
			if (strstr(ViewModel[i].modelname, ent->model->name))
			{
				short* pskinref = (short*)((byte*)pStudioHeader + pStudioHeader->skinindex);
				mstudiotexture_t* ptexture = (mstudiotexture_t*)((byte*)pStudioHeader + pStudioHeader->textureindex);
				mstudiomodel_t* pSubModel = (mstudiomodel_t*)((byte*)pStudioHeader + pBodyPart[ViewModel[i].numbodyparts].modelindex);
				mstudiomesh_t* pmesh = (mstudiomesh_t*)((byte*)pStudioHeader + pSubModel[ViewModel[i].nummodels].meshindex);

				pSubModel[ViewModel[i].nummodels].nummesh++;

				for (int x = pSubModel[ViewModel[i].nummodels].nummesh - 1; x > ViewModel[i].meshindex; x--)
				{
					//LogToFile("RESTORING %s WITH CURRENT %s", ptexture[pskinref[pmesh[x].skinref]].name, ptexture[pskinref[pmesh[x - 1].skinref]].name);
					pmesh[x] = pmesh[x - 1];
				}

				pmesh[ViewModel[i].meshindex] = ViewModel[i].pmesh;

				//LogToFile("ENABLE %s", ptexture[pskinref[pmesh[ViewModel[i].meshindex].skinref]].name);

				ViewModel.erase(ViewModel.begin() + i);
			}
		}
	}
}

void ResetViewModelSkin()
{
	for (int i = ViewModel.size() - 1; i >= 0; i--)
	{
		struct model_s* model = g_Engine.CL_LoadModel(ViewModel[i].modelname, NULL);
		if (model)
		{
			studiohdr_t* pStudioHeader = (studiohdr_t*)g_Studio.Mod_Extradata(model);
			mstudiobodyparts_t* pBodyPart = (mstudiobodyparts_t*)((byte*)pStudioHeader + pStudioHeader->bodypartindex);
			short* pskinref = (short*)((byte*)pStudioHeader + pStudioHeader->skinindex);
			mstudiotexture_t* ptexture = (mstudiotexture_t*)((byte*)pStudioHeader + pStudioHeader->textureindex);
			mstudiomodel_t* pSubModel = (mstudiomodel_t*)((byte*)pStudioHeader + pBodyPart[ViewModel[i].numbodyparts].modelindex);
			mstudiomesh_t* pmesh = (mstudiomesh_t*)((byte*)pStudioHeader + pSubModel[ViewModel[i].nummodels].meshindex);

			pSubModel[ViewModel[i].nummodels].nummesh++;

			for (int x = pSubModel[ViewModel[i].nummodels].nummesh - 1; x > ViewModel[i].meshindex; x--)
			{
				//LogToFile("RESTORING %s WITH CURRENT %s", ptexture[pskinref[pmesh[x].skinref]].name, ptexture[pskinref[pmesh[x - 1].skinref]].name);
				pmesh[x] = pmesh[x - 1];
			}

			pmesh[ViewModel[i].meshindex] = ViewModel[i].pmesh;

			//LogToFile("ENABLE %s", ptexture[pskinref[pmesh[ViewModel[i].meshindex].skinref]].name);
		}
		ViewModel.erase(ViewModel.begin() + i);
	}
}

void SaveView()
{
	char filename[256];
	sprintf(filename, "%s%s", hackdir, "viewmodel/viewmodel.disable");
	remove(filename);
	ofstream ofs(filename, ios::binary | ios::app);
	for (viewmodeltexture_t Tex : ViewModelTexture)
	{
		char text[256];
		sprintf(text, "Disable: %s ", Tex.name);
		ofs << text << (char)0x0D << (char)0x0A;
	}
	ofs.close();
}

void LoadView()
{
	char filename[256];
	sprintf(filename, "%s%s", hackdir, "viewmodel/viewmodel.disable");
	ifstream ifs(filename);
	while (ifs.good())
	{
		char buf[1024];
		ifs.getline(buf, sizeof(buf));
		char disable[256];
		if (sscanf(buf, "%*s %s", &disable))
		{
			if (disable)
			{
				bool saved = false;
				for (viewmodeltexture_t Tex : ViewModelTexture)
				{
					if (strstr(Tex.name, disable))
						saved = true;
				}
				if (!saved)
				{
					viewmodeltexture_t Tex;
					strcpy(Tex.name, disable);
					ViewModelTexture.push_back(Tex);
				}
			}
		}
	}
	ifs.close();
}

void ViewModelFov(ref_params_s* pparams)
{
	if (DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
	{
		cl_entity_s* vm = g_Engine.GetViewModel();
		if (vm)
		{
			Vector forward = pparams->forward;
			vm->origin += forward * cvar.visual_viewmodel_fov;
		}
	}
}