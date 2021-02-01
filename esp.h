#ifndef _ESP_
#define _ESP_

typedef struct
{
	Vector HitboxMulti[8];
} playeresphitbox_t;

typedef struct
{
	int index;
	int sequence;
	int weaponmodel;
	bool dummy;
	Vector origin;
	deque<playeresphitbox_t> PlayerEspHitbox;
} playeresp_t;
extern deque<playeresp_t> PlayerEsp;

void DrawPlayerEsp();
void DrawPlayerSoundIndexEsp();
void DrawPlayerSoundNoIndexEsp();

#endif