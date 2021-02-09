#ifndef _WINDOW_
#define _WINDOW_

void DrawFullScreenWindow();
bool WorldToScreen(float* pflOrigin, float* pflVecScreen);
bool ScreenToWorld(float* screen, float* world);

#endif