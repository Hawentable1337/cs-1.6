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
extern float esph;
extern float model_pos_x, model_pos_y;
extern float modelscale;

void Playerdummy();

#endif