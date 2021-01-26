#ifndef _VIEWMODEL_
#define _VIEWMODEL_

typedef struct
{
	mstudiomesh_t pmesh;
	int meshindex;
	char name[64];
	int numbodyparts;
	int nummodels;
	int nummesh;
	char modelname[MAX_MODEL_NAME];
} viewmodel_t;
extern deque<viewmodel_t> ViewModel;

typedef struct
{
	char name[64];
} viewmodeltexture_t;
extern deque<viewmodeltexture_t> ViewModelTexture;

void SaveView();
void LoadView();
void ResetViewModelSkin();
void ViewModelSkin();
void ViewModelFov(ref_params_s* pparams);

#endif