#include "client.h"

overviewInfo_t m_OverviewData;
int iX, iY, iW, iH;
bool MapLoaded = false;
int xTile = 1, yTile = 1; 
model_s* m_MapSprites;

bool ParseOverview(char* overview_txt)
{
	m_OverviewData.origin[0] = 0.0f;
	m_OverviewData.origin[1] = 0.0f;
	m_OverviewData.origin[2] = 0.0f;
	m_OverviewData.zoom = 1.0f;
	m_OverviewData.layers = 0;
	m_OverviewData.layersHeights[0] = 0.0f;
	m_OverviewData.layersImages[0][0] = 0;
	char token[1024];
	char* pfile = (char*)g_Engine.COM_LoadFile(overview_txt, 5, NULL);
	if (!pfile)
	{
		MapLoaded = false;
		return false;
	}
	for (;;)
	{
		pfile = g_Engine.COM_ParseFile(pfile, token);
		if (!pfile)
			break;
		if (!stricmp(token, "global"))
		{
			pfile = g_Engine.COM_ParseFile(pfile, token);
			if (strcmp(token, "{"))
			{
				MapLoaded = false;
				return false;
			}
			pfile = g_Engine.COM_ParseFile(pfile, token);
			if (!pfile)
				break;
			while (strcmp(token, "}"))
			{
				if (!stricmp(token, "zoom"))
				{
					pfile = g_Engine.COM_ParseFile(pfile, token);
					m_OverviewData.zoom = (float)atof(token);
				}
				else if (!stricmp(token, "origin"))
				{
					pfile = g_Engine.COM_ParseFile(pfile, token);
					m_OverviewData.origin[0] = (float)atof(token);
					pfile = g_Engine.COM_ParseFile(pfile, token);
					m_OverviewData.origin[1] = (float)atof(token);
					pfile = g_Engine.COM_ParseFile(pfile, token);
					m_OverviewData.origin[2] = (float)atof(token);
				}
				else if (!stricmp(token, "rotated"))
				{
					pfile = g_Engine.COM_ParseFile(pfile, token);
					m_OverviewData.rotated = atoi(token);
				}
				pfile = g_Engine.COM_ParseFile(pfile, token);
			}
		}
		else if (!stricmp(token, "layer"))
		{
			pfile = g_Engine.COM_ParseFile(pfile, token);
			if (strcmp(token, "{"))
			{
				MapLoaded = false;
				return false;
			}
			pfile = g_Engine.COM_ParseFile(pfile, token);
			while (strcmp(token, "}"))
			{
				if (!stricmp(token, "image"))
				{
					pfile = g_Engine.COM_ParseFile(pfile, token);
					strcpy(m_OverviewData.layersImages[m_OverviewData.layers], token);
				}
				else if (!stricmp(token, "height"))
				{
					pfile = g_Engine.COM_ParseFile(pfile, token);
					float height = (float)atof(token);
					m_OverviewData.layersHeights[m_OverviewData.layers] = height;
				}
				pfile = g_Engine.COM_ParseFile(pfile, token);
			}
			m_OverviewData.layers++;
		}
	}
	return true;
}

void LoadOverview(char* levelname)
{
	static char last_levelname[256] = "";
	char overview_txt[256];
	if (!strcmp(last_levelname, levelname))
		return;
	if (levelname[0] == 0)
		strcpy(levelname, "cs_miltia");
	sprintf(overview_txt, "overviews/%s.txt", levelname);
	if (!ParseOverview(overview_txt))
	{
		strcpy(last_levelname, levelname);
		MapLoaded = false;
		return;
	}
	m_MapSprites = g_Engine.LoadMapSprite(m_OverviewData.layersImages[0]);
	if (!m_MapSprites)
	{
		strcpy(last_levelname, levelname);
		MapLoaded = false;
		return;
	}
	MapLoaded = true;
	int i = (int)sqrt(m_MapSprites->numframes / (4 * 3));
	xTile = i * 4;
	yTile = i * 3;
}

static int gcd(int a, int b) 
{
	return (b == 0) ? a : gcd(b, a % b);
}

void DrawOverviewLayer()
{
	if (!bInitializeImGui)
		return;
	if (!MapLoaded)
		return;
	if (!cvar.radar || !bAliveLocal())
		return;
	if (!(DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual)))
		return;

	Vector vAngle, vEye = pmove->origin + pmove->view_ofs;
	g_Engine.GetViewAngles(vAngle);

	glViewport(iX, ImGui::GetIO().DisplaySize.y - (iY + iH), iW, iH);
	if (m_MapSprites) 
	{
		float vStepRight[2], vStepUp[2], inner[2], outer[2];
		float z = ((90.0f - vAngle[0]) / 90.0f) * m_OverviewData.layersHeights[0];
		float xStep = (8192.f / cvar.radar_zoom) / xTile;
		float yStep = -(8192.f / (cvar.radar_zoom * (4.f / 3.f))) / yTile;
		float angle = (float)((vAngle[1] + 90.0) * (M_PI / 180));
		if (m_OverviewData.rotated)
			angle -= float(M_PI / 2);
		vStepRight[0] = (float)cos(angle) * xStep;
		vStepRight[1] = (float)sin(angle) * xStep;
		vStepUp[0] = (float)cos(angle + (M_PI / 2)) * yStep;
		vStepUp[1] = (float)sin(angle + (M_PI / 2)) * yStep;
		float tile_x, tile_y;
		if (m_OverviewData.rotated) 
		{
			float origin_tilex = (float)(-4.f + m_OverviewData.zoom * (1.0 / 1024.0) * m_OverviewData.origin[0]);
			float origin_tiley = (float)(3.f + m_OverviewData.zoom * (1.0 / 1024.0) * m_OverviewData.origin[1]);
			tile_y = -(float)(origin_tilex - (1.0 / 1024) * m_OverviewData.zoom * vEye[0]);
			tile_x = (float)(origin_tiley - (1.0 / 1024) * m_OverviewData.zoom * vEye[1]);
		}
		else 
		{ 
			float origin_tilex = (float)(3.f + m_OverviewData.zoom * (1.0 / 1024.0) * m_OverviewData.origin[0]);
			float origin_tiley = (float)(4.f + m_OverviewData.zoom * (1.0 / 1024.0) * m_OverviewData.origin[1]);
			tile_x = (float)(origin_tilex - (1.0 / 1024) * m_OverviewData.zoom * vEye[0]);
			tile_y = (float)(origin_tiley - (1.0 / 1024) * m_OverviewData.zoom * vEye[1]);
		}
		outer[0] = (ImGui::GetIO().DisplaySize.x / 2) - tile_x * vStepRight[0] - tile_y * vStepUp[0];
		outer[1] = (ImGui::GetIO().DisplaySize.y / 2) - tile_x * vStepRight[1] - tile_y * vStepUp[1];
		g_Engine.pTriAPI->RenderMode(kRenderTransTexture);
		g_Engine.pTriAPI->CullFace(TRI_NONE);
		glEnable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
		for (int ix = 0, frame = 0; ix < yTile; ix++) 
		{
			inner[0] = outer[0];
			inner[1] = outer[1];
			for (int iy = 0; iy < xTile; iy++) 
			{
				g_Engine.pTriAPI->SpriteTexture(m_MapSprites, frame);
				g_Engine.pTriAPI->Begin(TRI_QUADS);
				g_Engine.pTriAPI->TexCoord2f(0, 0);
				g_Engine.pTriAPI->Vertex3f(inner[0], inner[1], z);
				g_Engine.pTriAPI->TexCoord2f(0, 1);
				g_Engine.pTriAPI->Vertex3f(inner[0] + vStepRight[0], inner[1] + vStepRight[1], z);
				g_Engine.pTriAPI->TexCoord2f(1, 1);
				g_Engine.pTriAPI->Vertex3f(inner[0] + vStepRight[0] + vStepUp[0], inner[1] + vStepRight[1] + vStepUp[1], z);
				g_Engine.pTriAPI->TexCoord2f(1, 0);
				g_Engine.pTriAPI->Vertex3f(inner[0] + vStepUp[0], inner[1] + vStepUp[1], z);
				g_Engine.pTriAPI->End();
				frame++;
				inner[0] += vStepUp[0];
				inner[1] += vStepUp[1];
			}
			outer[0] += vStepRight[0];
			outer[1] += vStepRight[1];
		}
		glDisable(GL_BLEND);
	}
	glViewport(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
}

void VectorRotateZ(const float* in, float angle, float* out)
{
	float a, c, s;
	a = (angle * (M_PI / 180));
	c = cos(a);
	s = sin(a);
	out[0] = c * in[0] - s * in[1];
	out[1] = s * in[0] + c * in[1];
	out[2] = in[2];
}

void DrawOverviewEntities()
{
	if (!cvar.radar || !bAliveLocal())
		return;
	if (!(DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual)))
		return;

	static model_s* hSpriteRed = (struct model_s*)g_Engine.GetSpritePointer(g_Engine.pfnSPR_Load("sprites/iplayerred.spr"));
	static model_s* hSpriteBlue = (struct model_s*)g_Engine.GetSpritePointer(g_Engine.pfnSPR_Load("sprites/iplayerblue.spr"));

	Vector vAngle, vEye = pmove->origin + pmove->view_ofs;
	g_Engine.GetViewAngles(vAngle);

	g_Engine.pTriAPI->CullFace(TRI_NONE);
	for (unsigned int i = 1; i <= g_Engine.GetMaxClients(); i++)
	{
		cl_entity_s* ent = g_Engine.GetEntityByIndex(i);
		if (!ent || !ent->player)
			continue;
		if (ent->curstate.messagenum < g_Engine.GetEntityByIndex(pmove->player_index + 1)->curstate.messagenum)
			continue;
		if (ent->index == pmove->player_index + 1)
			continue;
		if (!g_Player[ent->index].bAliveInScoreTab)
			continue;
		if (ent->curstate.mins.IsZero())
			continue;
		if (ent->curstate.maxs.IsZero())
			continue;
		if (g_Engine.GetEntityByIndex(pmove->player_index + 1)->curstate.iuser1 == OBS_IN_EYE && g_Engine.GetEntityByIndex(pmove->player_index + 1)->curstate.iuser2 == ent->index)
			continue;
		if (cvar.visual_idhook_only && idhook.FirstKillPlayer[ent->index] != 1)
			continue;
		if (!cvar.visual_visual_team && g_Player[ent->index].iTeam == g_Local.iTeam)
			continue;
		if (g_Player[ent->index].iTeam == 0)
			continue;

		struct model_s* hSprite = 0;
		if (g_Player[i].iTeam == 1)
			hSprite = hSpriteRed;
		if (g_Player[i].iTeam == 2)
			hSprite = hSpriteBlue;

		int screenx, screeny;
		int boxsize = (cvar.radar_point_size / 2) * 1.4;
		float aim[3], newaim[3];
		aim[0] = ent->origin[0] - vEye[0];
		aim[1] = ent->origin[1] - vEye[1];
		aim[2] = ent->origin[2] - vEye[2];
		VectorRotateZ(aim, -vAngle[1], newaim);
		screenx = iX + iW / 2 - int(newaim[1] / cvar.radar_zoom * m_OverviewData.zoom * 0.3f * iW / 2 / (ImGui::GetIO().DisplaySize.x / 6.7));
		screeny = iY + iH / 2 - int(newaim[0] / cvar.radar_zoom * m_OverviewData.zoom * 0.4f * iH / 2 / (ImGui::GetIO().DisplaySize.y / 5.1));
		if (screenx > iX + iW / 2 + iW / 2 - boxsize - 2)
			screenx = iX + iW / 2 + iW / 2 - boxsize - 2;
		if (screenx < iX + iW / 2 - iW / 2 + boxsize + 3)
			screenx = iX + iW / 2 - iW / 2 + boxsize + 3;
		if (screeny > iY + iH / 2 + iH / 2 - boxsize - 2)
			screeny = iY + iH / 2 + iH / 2 - boxsize - 2;
		if (screeny < iY + iH / 2 - iH / 2 + boxsize + 3)
			screeny = iY + iH / 2 - iH / 2 + boxsize + 3;

		g_Engine.pTriAPI->SpriteTexture(hSprite, 0);
		g_Engine.pTriAPI->RenderMode(kRenderTransTexture);

		Vector vforward, vRight, vAngles = Vector(0, -ent->angles[1] + vAngle[1] - 90, 0);
		g_Engine.pfnAngleVectors(vAngles, vforward, vRight, NULL);

		g_Engine.pTriAPI->Begin(TRI_QUADS);
		g_Engine.pTriAPI->Color4f(1.0, 1.0, 1.0, 1.0);

		float flX, flY;

		g_Engine.pTriAPI->TexCoord2f(1, 0);
		flX = screenx + cvar.radar_point_size * vRight[0];
		flX = flX + cvar.radar_point_size * vforward[0];
		flY = screeny + cvar.radar_point_size * vRight[1];
		flY = flY + cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(0, 0);
		flX = screenx + cvar.radar_point_size * vRight[0];
		flX = flX - cvar.radar_point_size * vforward[0];
		flY = screeny + cvar.radar_point_size * vRight[1];
		flY = flY - cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(0, 1);
		flX = screenx - cvar.radar_point_size * vRight[0];
		flX = flX - cvar.radar_point_size * vforward[0];
		flY = screeny - cvar.radar_point_size * vRight[1];
		flY = flY - cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(1, 1);
		flX = screenx - cvar.radar_point_size * vRight[0];
		flX = flX + cvar.radar_point_size * vforward[0];
		flY = screeny - cvar.radar_point_size * vRight[1];
		flY = flY + cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->End();
	}
}

void DrawOverviewEntitiesSoundIndex()
{
	if (!cvar.radar || !bAliveLocal())
		return;
	if (!(DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual)))
		return;

	static model_s* hSpriteRed = (struct model_s*)g_Engine.GetSpritePointer(g_Engine.pfnSPR_Load("sprites/iplayerred.spr"));
	static model_s* hSpriteBlue = (struct model_s*)g_Engine.GetSpritePointer(g_Engine.pfnSPR_Load("sprites/iplayerblue.spr"));

	Vector vAngle, vEye = pmove->origin + pmove->view_ofs;
	g_Engine.GetViewAngles(vAngle);

	g_Engine.pTriAPI->CullFace(TRI_NONE);
	for (player_sound_index_t sound_index : Sound_Index)
	{
		cl_entity_s* ent = g_Engine.GetEntityByIndex(sound_index.index);
		if (!ent)
			continue;
		if (cvar.visual_idhook_only && idhook.FirstKillPlayer[sound_index.index] != 1)
			continue;
		if (ent->curstate.messagenum == g_Engine.GetEntityByIndex(pmove->player_index + 1)->curstate.messagenum)
			continue;
		if (!cvar.visual_visual_team && g_Player[sound_index.index].iTeam == g_Local.iTeam)
			continue;
		if (g_Player[sound_index.index].iTeam == 0)
			continue;
		if (!g_Player[sound_index.index].bAliveInScoreTab)
			continue;

		struct model_s* hSprite = 0;
		if (g_Player[sound_index.index].iTeam == 1)
			hSprite = hSpriteRed;
		if (g_Player[sound_index.index].iTeam == 2)
			hSprite = hSpriteBlue;

		int screenx, screeny;
		int boxsize = (cvar.radar_point_size / 2) * 1.4;
		float aim[3], newaim[3];
		aim[0] = sound_index.origin[0] - vEye[0];
		aim[1] = sound_index.origin[1] - vEye[1];
		aim[2] = sound_index.origin[2] - vEye[2];
		VectorRotateZ(aim, -vAngle[1], newaim);
		screenx = iX + iW / 2 - int(newaim[1] / cvar.radar_zoom * m_OverviewData.zoom * 0.3f * iW / 2 / 160);
		screeny = iY + iH / 2 - int(newaim[0] / cvar.radar_zoom * m_OverviewData.zoom * 0.4f * iH / 2 / 160);
		if (screenx > iX + iW / 2 + iW / 2 - boxsize - 2)
			screenx = iX + iW / 2 + iW / 2 - boxsize - 2;
		if (screenx < iX + iW / 2 - iW / 2 + boxsize + 3)
			screenx = iX + iW / 2 - iW / 2 + boxsize + 3;
		if (screeny > iY + iH / 2 + iH / 2 - boxsize - 2)
			screeny = iY + iH / 2 + iH / 2 - boxsize - 2;
		if (screeny < iY + iH / 2 - iH / 2 + boxsize + 3)
			screeny = iY + iH / 2 - iH / 2 + boxsize + 3;

		g_Engine.pTriAPI->SpriteTexture(hSprite, 0);
		g_Engine.pTriAPI->RenderMode(kRenderTransTexture);

		QAngle QAimAngles;
		VectorAngles(sound_index.origin - vEye, QAimAngles);
		Vector vforward, vRight, vAngles = Vector(0, -QAimAngles[1] + vAngle[1] + 90, 0);
		g_Engine.pfnAngleVectors(vAngles, vforward, vRight, NULL);

		g_Engine.pTriAPI->Begin(TRI_QUADS);
		g_Engine.pTriAPI->Color4f(1.0, 1.0, 1.0, 1.0);

		float flX, flY;

		g_Engine.pTriAPI->TexCoord2f(1, 0);
		flX = screenx + cvar.radar_point_size * vRight[0];
		flX = flX + cvar.radar_point_size * vforward[0];
		flY = screeny + cvar.radar_point_size * vRight[1];
		flY = flY + cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(0, 0);
		flX = screenx + cvar.radar_point_size * vRight[0];
		flX = flX - cvar.radar_point_size * vforward[0];
		flY = screeny + cvar.radar_point_size * vRight[1];
		flY = flY - cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(0, 1);
		flX = screenx - cvar.radar_point_size * vRight[0];
		flX = flX - cvar.radar_point_size * vforward[0];
		flY = screeny - cvar.radar_point_size * vRight[1];
		flY = flY - cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(1, 1);
		flX = screenx - cvar.radar_point_size * vRight[0];
		flX = flX + cvar.radar_point_size * vforward[0];
		flY = screeny - cvar.radar_point_size * vRight[1];
		flY = flY + cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->End();
	}
}

void DrawOverviewEntitiesSoundNoIndex()
{
	if (!cvar.radar || !bAliveLocal())
		return;
	if (!(DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual)))
		return;

	static struct model_s* hSprite = (struct model_s*)g_Engine.GetSpritePointer(g_Engine.pfnSPR_Load("sprites/iplayer.spr"));

	Vector vAngle, vEye = pmove->origin + pmove->view_ofs;
	g_Engine.GetViewAngles(vAngle);

	g_Engine.pTriAPI->CullFace(TRI_NONE);
	for (player_sound_no_index_t sound_no_index : Sound_No_Index)
	{
		int screenx, screeny;
		int boxsize = (cvar.radar_point_size / 2) * 1.4;
		float aim[3], newaim[3];
		aim[0] = sound_no_index.origin[0] - vEye[0];
		aim[1] = sound_no_index.origin[1] - vEye[1];
		aim[2] = sound_no_index.origin[2] - vEye[2];
		VectorRotateZ(aim, -vAngle[1], newaim);
		screenx = iX + iW / 2 - int(newaim[1] / cvar.radar_zoom * m_OverviewData.zoom * 0.3f * iW / 2 / 160);
		screeny = iY + iH / 2 - int(newaim[0] / cvar.radar_zoom * m_OverviewData.zoom * 0.4f * iH / 2 / 160);
		if (screenx > iX + iW / 2 + iW / 2 - boxsize - 2)
			screenx = iX + iW / 2 + iW / 2 - boxsize - 2;
		if (screenx < iX + iW / 2 - iW / 2 + boxsize + 3)
			screenx = iX + iW / 2 - iW / 2 + boxsize + 3;
		if (screeny > iY + iH / 2 + iH / 2 - boxsize - 2)
			screeny = iY + iH / 2 + iH / 2 - boxsize - 2;
		if (screeny < iY + iH / 2 - iH / 2 + boxsize + 3)
			screeny = iY + iH / 2 - iH / 2 + boxsize + 3;

		g_Engine.pTriAPI->SpriteTexture(hSprite, 0);
		g_Engine.pTriAPI->RenderMode(kRenderTransTexture);

		QAngle QAimAngles;
		VectorAngles(sound_no_index.origin - vEye, QAimAngles);
		Vector vforward, vRight, vAngles = Vector(0, -QAimAngles[1] + vAngle[1] + 90, 0);
		g_Engine.pfnAngleVectors(vAngles, vforward, vRight, NULL);

		g_Engine.pTriAPI->Begin(TRI_QUADS);
		g_Engine.pTriAPI->Color4f(0.0, 1.0, 0.0, 1.0);

		float flX, flY;

		g_Engine.pTriAPI->TexCoord2f(1, 0);
		flX = screenx + cvar.radar_point_size * vRight[0];
		flX = flX + cvar.radar_point_size * vforward[0];
		flY = screeny + cvar.radar_point_size * vRight[1];
		flY = flY + cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(0, 0);
		flX = screenx + cvar.radar_point_size * vRight[0];
		flX = flX - cvar.radar_point_size * vforward[0];
		flY = screeny + cvar.radar_point_size * vRight[1];
		flY = flY - cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(0, 1);
		flX = screenx - cvar.radar_point_size * vRight[0];
		flX = flX - cvar.radar_point_size * vforward[0];
		flY = screeny - cvar.radar_point_size * vRight[1];
		flY = flY - cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->TexCoord2f(1, 1);
		flX = screenx - cvar.radar_point_size * vRight[0];
		flX = flX + cvar.radar_point_size * vforward[0];
		flY = screeny - cvar.radar_point_size * vRight[1];
		flY = flY + cvar.radar_point_size * vforward[1];
		glVertex2f(flX, flY);

		g_Engine.pTriAPI->End();
	}
}

void DrawOverview()
{
	if (!cvar.radar || !bAliveLocal())
		return;

	ImVec2 WindowMinSize = ImGui::GetStyle().WindowMinSize;
	ImGui::GetStyle().WindowMinSize = ImVec2(75.0f, 75.0f);
	float WindowBorderSize = ImGui::GetStyle().WindowBorderSize;
	ImGui::GetStyle().WindowBorderSize = 1.0f;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(150, 150), ImGuiCond_Once);
	ImGui::Begin("overview", NULL, ImGuiWindowFlags_NoTitleBar);
	{
		iX = ImGui::GetCursorScreenPos().x;
		iY = ImGui::GetCursorScreenPos().y;
		iW = ImGui::GetContentRegionAvail().x;
		iH = ImGui::GetContentRegionAvail().y;

		ImU32 color;
		if (g_Local.iTeam == 1)
			color = Red();
		else if (g_Local.iTeam == 2)
			color = Blue();
		else
			color = White();
		ImGui::GetCurrentWindow()->DrawList->AddTriangleFilled({ IM_ROUND(iX + iW / 2 - cvar.radar_point_size) , IM_ROUND(iY + iH / 2) }, { IM_ROUND(iX + iW / 2), IM_ROUND(iY + iH / 2) - cvar.radar_point_size / 2 }, { IM_ROUND(iX + iW / 2) , IM_ROUND(iY + iH / 2) - cvar.radar_point_size }, color);
		ImGui::GetCurrentWindow()->DrawList->AddTriangleFilled({ IM_ROUND(iX + iW / 2 + cvar.radar_point_size) , IM_ROUND(iY + iH / 2) }, { IM_ROUND(iX + iW / 2), IM_ROUND(iY + iH / 2) - cvar.radar_point_size / 2 }, { IM_ROUND(iX + iW / 2) , IM_ROUND(iY + iH / 2) - cvar.radar_point_size }, color);
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::GetStyle().WindowMinSize = WindowMinSize;
	ImGui::GetStyle().WindowBorderSize = WindowBorderSize;
}