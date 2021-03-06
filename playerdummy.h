#ifndef _PLAYERDUMMY_
#define _PLAYERDUMMY_

extern cl_entity_s playerdummy;

typedef struct
{
	model_s* mod;
} playermodel_t;

extern deque<playermodel_t> PlayerModel;

extern float modelscreenw, modelscreenh;
extern int model_type;
extern float espx, espy, espw, esph;
extern float model_pos_x, model_pos_y;
extern float modelscale;
extern bool drawdummy;
extern bool drawgetdummy;
extern bool drawbackdummy;

void GetDummyAngle(ref_params_s* pparams);
void GetDummyModels(); 
void DrawDummyBack();
void Playerdummy();

#endif