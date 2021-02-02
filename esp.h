#ifndef _ESP_
#define _ESP_

typedef struct
{
	Vector HitboxMulti[8];
} esphitbox_t;

typedef struct
{
	int index;
	int sequence;
	int weaponmodel;
	bool dummy;
	char model[MAX_MODEL_NAME];
	Vector origin;
	deque<esphitbox_t> PlayerEspHitbox;
} playeresp_t;
extern deque<playeresp_t> PlayerEsp;

typedef struct
{
	int index;
	int numhitboxes;
	Vector origin;
	char name[MAX_MODEL_NAME];
	deque<esphitbox_t> WorldEspHitbox;
} worldesp_t;
extern deque<worldesp_t> WorldEsp;

void DrawWorldEsp();
void DrawPlayerEsp();
void DrawPlayerSoundIndexEsp();
void DrawPlayerSoundNoIndexEsp();

#endif