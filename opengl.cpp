#include "client.h"

typedef void (APIENTRY* glBegin_t)(GLenum);
typedef BOOL (APIENTRY* wglSwapBuffers_t)(HDC  hdc);
typedef void (APIENTRY* glClear_t)(GLbitfield mask);
typedef void (APIENTRY* glColor4f_t)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (__stdcall* glReadPixels_t)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*);

glBegin_t pglBegin = NULL;
wglSwapBuffers_t pwglSwapBuffers = NULL;
glClear_t pglClear = NULL;
glColor4f_t pglColor4f = NULL;
glReadPixels_t pglReadPixels = NULL;

void APIENTRY Hooked_glBegin(GLenum mode)
{
	Playerdummy();
	cl_entity_s* ent = g_Studio.GetCurrentEntity();
	if (ent && ent == &playerdummy)
	{
		if (cvar.chams_player > 1 && chams_playerdummy)
		{
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			glEnable(GL_TEXTURE_2D);
		}
	}
	pglBegin(mode);
}

BOOL APIENTRY Hooked_wglSwapBuffers(HDC hdc)
{
	if(hdc)HookImGui(hdc);
	return pwglSwapBuffers(hdc);
}

void APIENTRY Hooked_glClear(GLbitfield mask)
{
	if (mask == GL_DEPTH_BUFFER_BIT)
		pglClear(GL_COLOR_BUFFER_BIT), glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	pglClear(mask);
}

void __stdcall Hooked_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
	if (ScreenFirst || !cvar.snapshot_memory)
	{
		dwSize = (width * height) * 3;
		BufferScreen = (PBYTE)malloc(dwSize);
		pglReadPixels(x, y, width, height, format, type, pixels);
		memcpy(BufferScreen, pixels, dwSize);
		DrawVisuals = true;
		ScreenFirst = false;
		return;
	}
	memcpy(pixels, BufferScreen, dwSize);
}

void APIENTRY Hooked_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	if (chams_world)
	{
		if (cvar.chams_world == 1 || cvar.chams_world == 3)
			red = chams_world_r, green = chams_world_g, blue = chams_world_b;
		if (cvar.chams_world == 2)
			red = chams_world_r * red, green = chams_world_g * green, blue = chams_world_b * blue;
	}
	if (chams_viewmodel)
	{
		if (cvar.chams_view_model == 1 || cvar.chams_view_model == 3)
			red = chams_viewmodel_r, green = chams_viewmodel_g, blue = chams_viewmodel_b;
		if (cvar.chams_view_model == 2)
			red = chams_viewmodel_r * red, green = chams_viewmodel_g * green, blue = chams_viewmodel_b * blue;
	}
	if (chams_player)
	{
		if (cvar.chams_player == 1 || cvar.chams_player == 3)
			red = chams_player_r, green = chams_player_g, blue = chams_player_b;
		if (cvar.chams_player == 2)
			red = chams_player_r * red, green = chams_player_g * green, blue = chams_player_b * blue;
	}
	if (chams_playerdummy)
	{
		if (cvar.chams_player == 1 || cvar.chams_player == 3)
			red = chams_playerdummy_r, green = chams_playerdummy_g, blue = chams_playerdummy_b;
		if (cvar.chams_player == 2)
			red = chams_playerdummy_r * red, green = chams_playerdummy_g * green, blue = chams_playerdummy_b * blue;
	}
	pglColor4f(red, green, blue, alpha);
}

void HookOpenGL()
{
	if (g_Studio.IsHardware() != 1)
		c_Offset.Error("Please run game in OpenGL renderer mode");

	HMODULE hmOpenGL = GetModuleHandle("opengl32.dll"); 
	if (hmOpenGL)
	{
		pglBegin = (glBegin_t)DetourFunction((LPBYTE)GetProcAddress(hmOpenGL, "glBegin"), (LPBYTE)& Hooked_glBegin);
		pwglSwapBuffers = (wglSwapBuffers_t)DetourFunction((LPBYTE)GetProcAddress(hmOpenGL, "wglSwapBuffers"), (LPBYTE)& Hooked_wglSwapBuffers);
		pglClear = (glClear_t)DetourFunction((LPBYTE)GetProcAddress(hmOpenGL, "glClear"), (LPBYTE)& Hooked_glClear);
		pglColor4f = (glColor4f_t)DetourFunction((LPBYTE)GetProcAddress(hmOpenGL, "glColor4f"), (LPBYTE)&Hooked_glColor4f);
		pglReadPixels = (glReadPixels_t)DetourFunction((PBYTE)GetProcAddress(hmOpenGL, "glReadPixels"), (PBYTE)Hooked_glReadPixels);
	}
}
