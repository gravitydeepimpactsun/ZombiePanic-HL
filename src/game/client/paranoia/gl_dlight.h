#ifndef DYNDLIGHT_H
#define DYNDLIGHT_H

struct DynamicLight
{
	Vector	origin;
	float	radius;
	Vector	color; // ignored for spotlights, they have a texture
	float	die;				// stop lighting after this time
	float	decay;				// drop this each second
	int		key;

	// spotlight specific:
	Vector	angles;	
	float	cone_hor;
	float	cone_ver;
	char	spot_texture[64];
};


DynamicLight* MY_AllocDlight (int key);

int GetDlightsForPoint(Vector point, Vector mins, Vector maxs, DynamicLight **pOutArray, int alwaysFlashlight);
extern int g_flashlight;

#endif // DYNDLIGHT_H