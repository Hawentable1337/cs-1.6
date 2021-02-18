#include "client.h"

int StudioCheckBBox(void)
{
	if ((cvar.rage_active && cvar.rage_fov > 45) ||
		(cvar.legit[g_Local.weapon.m_iWeaponID].active && cvar.legit[g_Local.weapon.m_iWeaponID].fov > 45) ||
		(cvar.knifebot_active && cvar.knifebot_fov > 45))
		return 1;
	return g_Studio.StudioCheckBBox();
}

void StudioSetRemapColors(int top, int bottom)
{
	GetHitboxes();
	g_Studio.StudioSetRemapColors(top, bottom);
}

void StudioDrawPoints()
{
	ViewModelSkin();
	g_Studio.StudioDrawPoints();
}

void StudioEntityLight(struct alight_s* plight)
{
	cl_entity_s* ent = g_Studio.GetCurrentEntity();
	if (ent && ent == &playerdummy)
	{
		plight->ambientlight = 128;
		plight->shadelight = 192;
		plight->color.x = 1.f;
		plight->color.y = 1.f;
		plight->color.z = 1.f;
	}
	g_Studio.StudioEntityLight(plight);
}

void HookStudioFunctions()
{
	g_pStudio->StudioEntityLight = StudioEntityLight;
	g_pStudio->StudioDrawPoints = StudioDrawPoints;
	g_pStudio->StudioSetRemapColors = StudioSetRemapColors;
	g_pStudio->StudioCheckBBox = StudioCheckBBox;
}