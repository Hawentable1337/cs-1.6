#include "client.h"

typedef void (APIENTRY* glBegin_t)(GLenum);
typedef BOOL (APIENTRY* wglSwapBuffers_t)(HDC  hdc);
typedef void (APIENTRY* glClear_t)(GLbitfield mask); 
typedef void (__stdcall* glReadPixels_t)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*);

glBegin_t pglBegin = NULL;
wglSwapBuffers_t pwglSwapBuffers = NULL;
glClear_t pglClear = NULL; 
glReadPixels_t pglReadPixels = NULL;

void APIENTRY Hooked_glBegin(GLenum mode)
{
	cl_entity_s* ent = g_Studio.GetCurrentEntity();
	if (ent && ent->curstate.messagenum == -1337)
	{
		if (mode == GL_TRIANGLE_STRIP)
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
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
		pglReadPixels = (glReadPixels_t)DetourFunction((PBYTE)GetProcAddress(hmOpenGL, "glReadPixels"), (PBYTE)Hooked_glReadPixels);
	}
}
