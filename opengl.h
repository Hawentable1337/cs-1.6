#ifndef _OPENGL_
#define _OPENGL_

void APIENTRY Hooked_glBegin(GLenum mode);
void APIENTRY Hooked_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
BOOL APIENTRY Hooked_wglSwapBuffers(HDC hdc);
void APIENTRY Hooked_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void APIENTRY Hooked_glClear(GLbitfield mask);
void APIENTRY Hooked_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);

void HookOpenGL();

#endif