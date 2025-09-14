#include "hud.h"
#include "mathlib/vector.h"
#include "fog.h"
#if WIN32
	#include "windows.h"
	#include "gl/gl.h"
#endif
#include "triangleapi.h"

CFog gFog;

// Fog overrides
static ConVar cl_fog_override( "cl_fog_override", "0", FCVAR_CHEATS, "Override the fog" );
static ConVar cl_fog_density( "cl_fog_density", "0.5", FCVAR_CHEATS, "Density of the fog when overridden" );
static ConVar cl_fog_start( "cl_fog_start", "1500", FCVAR_CHEATS, "Start distance of the fog when overridden" );
static ConVar cl_fog_end( "cl_fog_end", "2000", FCVAR_CHEATS, "End distance of the fog when overridden" );
static ConVar cl_fog_color( "cl_fog_color", "128 128 128", FCVAR_CHEATS, "Color of the fog when overridden (r g b)" );

CFog::CFog()
{
	ClearFog();
}

void CFog::SetWaterLevel( int waterLevel )
{
	m_iWaterLevel = waterLevel;
}

void CFog::SetFogParameters( const FogParams &params )
{
	// If blending, copy current fog params to the blend state if we had any active
	if (params.flBlendTime > 0 && params.fogEnd > 0 && params.fogStart > 0)
	{
		memcpy(&m_fogBlend1, &params, sizeof(FogParams));

		VectorCopy(params.color, m_fogBlend2.color);
		m_fogBlend2.fogStart = params.fogStart;
		m_fogBlend2.fogEnd = params.fogEnd;
		m_fogBlend2.density = params.density;

		// Set times
		m_fogChangeTime = gEngfuncs.GetClientTime();
		m_fogBlendTime = params.density;
	}
	else
	{
		// No blending, just set
		memset(&m_fogBlend1, 0, sizeof(FogParams));
		memset(&m_fogBlend2, 0, sizeof(FogParams));
		m_FogParams = params;
		m_fogChangeTime = 0;
		m_fogBlendTime = 0;
	}
}

FogParams CFog::GetFogParameters() const
{
	return m_FogParams;
}

void CFog::RenderFog( Vector& color )
{
	bool bFog;

	if ( m_iWaterLevel <= 2 ) // checking if player is not underwater
		bFog = (m_FogParams.density > 0.0f) ? true : false;
	else
		bFog = false;

	float flDensity = m_FogParams.density;
	float flStart = m_FogParams.fogStart;
	float flEnd = m_FogParams.fogEnd;
	float flColor[3];
	VectorCopy( color, flColor );

	bool bIsCheatsEnabled = CVAR_GET_FLOAT( "sv_cheats" ) >= 1 ? true : false;
	if ( cl_fog_override.GetBool() && bIsCheatsEnabled && m_bAllowOverride )
	{
		// Override the fog parameters
		bFog = true;
		flDensity = cl_fog_density.GetFloat();
		flStart = cl_fog_start.GetInt();
		flEnd = cl_fog_end.GetInt();
		sscanf( cl_fog_color.GetString(), "%f %f %f", &flColor[0], &flColor[1], &flColor[2] );
		flColor[0] = clamp( flColor[0], 0.0f, 255.0f ) / 255.0f;
		flColor[1] = clamp( flColor[1], 0.0f, 255.0f ) / 255.0f;
		flColor[2] = clamp( flColor[2], 0.0f, 255.0f ) / 255.0f;
	}

	// If start distance and end distance isn't properly set
	if ( !(flEnd > 0) && !(flStart > 0) )
		bFog = false;

	// If FOG is disabled.
	if ( !bFog )
	{
		gEngfuncs.pTriAPI->Fog( Vector(0, 0, 0), 0, 0, FALSE );
#if WIN32
		glDisable( GL_FOG );
#endif
		return;
	}

#if WIN32
	glEnable( GL_FOG );
	glFogi( GL_FOG_MODE, GL_LINEAR );
	glFogf( GL_FOG_DENSITY, flDensity );

	glFogfv( GL_FOG_COLOR, flColor );
	glFogf( GL_FOG_START, flStart );
	glFogf( GL_FOG_END, flEnd );
#else
	gEngfuncs.pTriAPI->FogParams( flDensity, m_FogParams.fogSkybox );
#endif

	// Finally, set the fog.
	gEngfuncs.pTriAPI->Fog( flColor, flStart, flEnd, bFog );
}

void CFog::ClearFog()
{
	memset(&m_fogBlend1, 0, sizeof(FogParams));
	memset(&m_fogBlend2, 0, sizeof(FogParams));

	m_FogParams.fogSkybox = false;
	m_FogParams.color[0] = m_FogParams.color[1] = m_FogParams.color[2] = 0.0f;
	m_FogParams.density = 0.0f;
	m_FogParams.flBlendTime = 0.0f;
}

void CFog::BlendFog()
{
	if (!m_fogChangeTime && !m_fogBlendTime)
		return;

	// Clear if blend is over
	if ((m_fogChangeTime + m_fogBlendTime) < gEngfuncs.GetClientTime())
	{
		memcpy(&m_FogParams, &m_fogBlend2, sizeof(FogParams));
		memset(&m_fogBlend1, 0, sizeof(FogParams));
		memset(&m_fogBlend2, 0, sizeof(FogParams));

		m_fogChangeTime = 0;
		m_fogBlendTime = 0;
		return;
	}

	// Perform linear blending
	float interp = (gEngfuncs.GetClientTime() - m_fogChangeTime) / m_fogBlendTime;
	VectorScale(m_fogBlend1.color, (1.0 - interp), m_FogParams.color);
	VectorMA(m_FogParams.color, interp, m_fogBlend2.color, m_FogParams.color);
	m_FogParams.fogEnd = (m_fogBlend1.fogEnd * (1.0 - interp)) + (m_fogBlend2.fogEnd * interp);
	m_FogParams.fogStart = (m_fogBlend1.fogStart * (1.0 - interp)) + (m_fogBlend2.fogStart * interp);
	m_FogParams.density = (m_fogBlend1.density * (1.0 - interp)) + (m_fogBlend2.density * interp);
}

void CFog::DrawFog( FogDrawType nType )
{
	switch ( nType )
	{
		case FOG_DRAW_STANDARD:
		case FOG_DRAW_VIEW:
		{
		    m_bAllowOverride = true;
			RenderFog( m_FogParams.color );
		}
		break;
		case FOG_DRAW_TRANSPARENT:
		{
		    m_bAllowOverride = false;
			RenderFog( Vector(0, 0, 0) );
		    m_bAllowOverride = true;
		}
		break;
	}
}
