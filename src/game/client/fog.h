#ifndef FOG_H
#define FOG_H

constexpr float FOG_START_DISTANCE = 1500.0f;
constexpr float FOG_END_DISTANCE = 2000.0f;

#include "mathlib/vector.h"

struct FogParams
{
	bool fogSkybox = false;
	Vector color;
	float density = 0.0f;
	float flBlendTime = 0.0f;
	int fogStart = FOG_START_DISTANCE;
	int fogEnd = FOG_END_DISTANCE;
};

enum FogDrawType
{
	FOG_DRAW_STANDARD = 0,
	FOG_DRAW_TRANSPARENT,
	FOG_DRAW_VIEW
};

class CFog
{
public:
	CFog();
	void SetWaterLevel(int waterLevel);
	void SetFogParameters(const FogParams &params);
	FogParams GetFogParameters() const;
	void RenderFog( Vector& color );
	void ClearFog();
	void BlendFog();

	// To make sure fog works properly in OpenGL
	void DrawFog( FogDrawType nType );

private:
	FogParams m_FogParams;
	FogParams m_fogBlend1;
	FogParams m_fogBlend2;
	int m_iWaterLevel;

	float m_fogChangeTime;
	float m_fogBlendTime;

	bool m_bAllowOverride = true;
};

extern CFog gFog;

#endif // FOG_H