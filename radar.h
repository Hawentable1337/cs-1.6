#ifndef _RADAR_
#define _RADAR_

typedef struct overviewInfo_s
{
	float	zoom;		// zoom of map images
	int		layers;		// how may layers do we have
	float	origin[3];  //
	float	layersHeights[1];
	char	layersImages[1][255];
	int		rotated;	// are map images rotated (90 degrees) ?
}
overviewInfo_t;

void LoadOverview(char* levelname);
void DrawOverview();
void DrawOverviewLayer();
void DrawOverviewEntities(); 
void DrawOverviewEntitiesSoundIndex(); 
void DrawOverviewEntitiesSoundNoIndex();

#endif
