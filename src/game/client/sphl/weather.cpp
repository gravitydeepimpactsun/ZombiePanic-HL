/***
*
*	Copyright (c) 2005, BUzer.
*	
*	Used with permission for Spirit of Half-Life 1.5
*
****/
/*
====== weather.cpp ========================================================
*/

//Johan: (2025-09-12)
//  -Changed and tweaked this for Zombie Panic! (We use a different base, so I had to change a few things)
//  -Changed the old "cl_weather" to "cl_weather_quality" ConVar
//  -Removed .pcs files, changed to KeyValues format instead
// - Added convar "cl_weather_rainsplash" to enable/disable rain splashes. It's disabled by default.

//SysOp: 
//	-Added new Dust Mode (customizable in .pcs file) Use rain mode 2 for turn it on.
//	-Added a new cvar "cl_weather". If the user don't want those effects, he can turn it off whenever.

// Good work BUzer :)

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include <memory.h>
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_types.h"
#include "cdll_int.h"
#include "pm_defs.h"
#include "event_api.h"

#include "weather.h"

#include <tier2/tier2.h>
#include <KeyValues.h>

// For our draw
#include "mathlib/mathlib.h"
#include "engfuncs.h"
#include "tri.h"
#include "triangleapi.h"

// Normalize
#include "zp/zp_shared.h"

weather_properties	WeatherData;

cl_drip		FirstChainDrip;
cl_rainfx	FirstChainFX;

// Mathlib doesn't want to work for client, so let's use these instead.
extern void AngleMatrix( const float *angles, float (*matrix)[4] );
extern void VectorTransform( const float *in1, float in2[3][4], float *out );

double rain_curtime;    // current time
double rain_oldtime;    // last time we have updated drips
double rain_timedelta;  // difference between old time and current time
double rain_nextspawntime;  // when the next drip should be spawned

int dripcounter = 0;
int fxcounter = 0;

// 3 is the default, which is "High".
static ConVar cl_weather_quality( "cl_weather_quality", "3", FCVAR_BHL_ARCHIVE );
static ConVar cl_weather_rainsplash( "cl_weather_rainsplash", "0", FCVAR_BHL_ARCHIVE );

/*
=================================
Must think every frame.
=================================
*/
void Weather::Process()
{
	rain_oldtime = rain_curtime; // save old time
	rain_curtime = gEngfuncs.GetClientTime();
	rain_timedelta = rain_curtime - rain_oldtime;

	// first frame
	if (rain_oldtime == 0)
	{
		// fix first frame bug with nextspawntime
		rain_nextspawntime = gEngfuncs.GetClientTime();
		Parse();
		return;
	}

	if ( WeatherData.weatherMode == WEATHER_NONE )
		return;

	if ( WeatherData.dripsPerSecond == 0 && FirstChainDrip.p_Next == NULL )
	{
		// keep nextspawntime correct
		rain_nextspawntime = rain_curtime;
		return;
	}

	if (rain_timedelta == 0)
		return; // not in pause

	double timeBetweenDrips = 1 / (double)WeatherData.dripsPerSecond;

	cl_drip* curDrip = FirstChainDrip.p_Next;
	cl_drip* nextDrip = NULL;

	cl_entity_t *player = gEngfuncs.GetLocalPlayer();

	while (curDrip != NULL) // go through list
	{
		nextDrip = curDrip->p_Next; // save pointer to next drip

		if ( WeatherData.weatherMode == WEATHER_RAIN )
			curDrip->origin.z -= rain_timedelta * DRIPSPEED; // rain
		else
			curDrip->origin.z -= rain_timedelta * SNOWSPEED; // snow

		curDrip->origin.x += rain_timedelta * curDrip->xDelta;
		curDrip->origin.y += rain_timedelta * curDrip->yDelta;

		// remove drip if its origin lower than minHeight
		if (curDrip->origin.z < curDrip->minHeight) 
		{
			if ( WeatherData.weatherMode == WEATHER_RAIN )
				WaterLandingEffect( curDrip, curDrip->landInWater ); // create water rings

			curDrip->p_Prev->p_Next = curDrip->p_Next; // link chain
			if (nextDrip != NULL)
				nextDrip->p_Prev = curDrip->p_Prev; 
			delete curDrip;
					
			dripcounter--;
		}

		curDrip = nextDrip; // restore pointer, so we can continue moving through chain
	}

	int maxDelta; // maximum height randomize distance
	float falltime;
	if ( WeatherData.weatherMode == WEATHER_RAIN )
	{
		maxDelta = DRIPSPEED * rain_timedelta; // for rain
		falltime = (WeatherData.globalHeight + 4096) / DRIPSPEED;
	}
	else
	{
		maxDelta = SNOWSPEED * rain_timedelta; // for snow
		falltime = (WeatherData.globalHeight + 4096) / SNOWSPEED;
	}

	int max_drawamount = 0;
	switch ( cl_weather_quality.GetInt() )
	{
		case WEATHER_QUALITY::WEATHER_OFF: max_drawamount = 0; break;
		case WEATHER_QUALITY::WEATHER_LOW: max_drawamount = MAXDRIPS_LOW; break;
		case WEATHER_QUALITY::WEATHER_MEDIUM: max_drawamount = MAXDRIPS_MEDIUM; break;
		case WEATHER_QUALITY::WEATHER_HIGH: max_drawamount = MAXDRIPS_HIGH; break;
		case WEATHER_QUALITY::WEATHER_VERYHIGH: max_drawamount = MAXDRIPS_VERYHIGH; break;
	}

	while (rain_nextspawntime < rain_curtime)
	{
		rain_nextspawntime += timeBetweenDrips;
		if (dripcounter < max_drawamount) // check for overflow
		{
			float deathHeight;
			Vector vecStart, vecEnd;

			vecStart[0] = gEngfuncs.pfnRandomFloat(player->origin.x - WeatherData.distFromPlayer, player->origin.x + WeatherData.distFromPlayer);
			vecStart[1] = gEngfuncs.pfnRandomFloat(player->origin.y - WeatherData.distFromPlayer, player->origin.y + WeatherData.distFromPlayer);
			vecStart[2] = WeatherData.globalHeight;

			float xDelta = WeatherData.windX + gEngfuncs.pfnRandomFloat(WeatherData.randX * -1, WeatherData.randX);
			float yDelta = WeatherData.windY + gEngfuncs.pfnRandomFloat(WeatherData.randY * -1, WeatherData.randY);

			// find a point at bottom of map
			vecEnd[0] = falltime * xDelta;
			vecEnd[1] = falltime * yDelta;
			vecEnd[2] = -4096;

			pmtrace_t pmtrace;
			gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
			gEngfuncs.pEventAPI->EV_PlayerTrace( vecStart, vecEnd, PM_STUDIO_IGNORE, -1, &pmtrace );

			if (pmtrace.startsolid)
				continue;

			// falling to water?
			int contents = gEngfuncs.PM_PointContents( pmtrace.endpos, NULL );
			if (contents == CONTENTS_WATER)
			{
				int waterEntity = gEngfuncs.PM_WaterEntity( pmtrace.endpos );
				if ( waterEntity > 0 )
				{
					cl_entity_t *pwater = gEngfuncs.GetEntityByIndex( waterEntity );
					if ( pwater && ( pwater->model != NULL ) )
					{
						deathHeight = pwater->curstate.maxs[2];
					}
					else
					{
						//Msg( "Weather error: can't get water entity\n" );
						continue;
					}
				}
				else
				{
					//Msg( "Weather error: water is not func_water entity\n" );
					continue;
				}
			}
			else
			{
				deathHeight = pmtrace.endpos[2];
			}

			// just in case..
			if (deathHeight > vecStart[2])
			{
				Msg( "Weather error: can't create drip in water\n" );
				continue;
			}


			cl_drip *newClDrip = new cl_drip;
			if (!newClDrip)
			{
				WeatherData.dripsPerSecond = 0; // disable rain
				Msg( "Weather error: failed to allocate object!\n" );
				return;
			}
			
			vecStart[2] -= gEngfuncs.pfnRandomFloat(0, maxDelta); // randomize a bit
			
			newClDrip->alpha = gEngfuncs.pfnRandomFloat(0.12, 0.2);
			VectorCopy(vecStart, newClDrip->origin);
			
			newClDrip->xDelta = xDelta;
			newClDrip->yDelta = yDelta;
	
			newClDrip->birthTime = rain_curtime; // store time when it was spawned
			newClDrip->minHeight = deathHeight;

			if (contents == CONTENTS_WATER)
				newClDrip->landInWater = 1;
			else
				newClDrip->landInWater = 0;

			// add to first place in chain
			newClDrip->p_Next = FirstChainDrip.p_Next;
			newClDrip->p_Prev = &FirstChainDrip;
			if (newClDrip->p_Next != NULL)
				newClDrip->p_Next->p_Prev = newClDrip;
			FirstChainDrip.p_Next = newClDrip;

			dripcounter++;
		}
		else
			return;
	}
}


void Weather::WaterLandingEffect( cl_drip *drip, bool inWater )
{
	if ( !cl_weather_rainsplash.GetBool() ) return;
	int max_drawamount = 0;
	switch ( cl_weather_quality.GetInt() )
	{
		case WEATHER_QUALITY::WEATHER_OFF: max_drawamount = 0; break;
		case WEATHER_QUALITY::WEATHER_LOW: max_drawamount = MAXFX_LOW; break;
		case WEATHER_QUALITY::WEATHER_MEDIUM: max_drawamount = MAXFX_MEDIUM; break;
		case WEATHER_QUALITY::WEATHER_HIGH: max_drawamount = MAXFX_HIGH; break;
		case WEATHER_QUALITY::WEATHER_VERYHIGH: max_drawamount = MAXFX_VERYHIGH; break;
	}

	if (fxcounter >= max_drawamount)
	{
		//gEngfuncs.Con_Printf( "Weather error: FX limit overflow!\n" );
		return;
	}

	cl_rainfx *newFX = new cl_rainfx;
	if (!newFX)
	{
		gEngfuncs.Con_Printf( "Weather error: failed to allocate FX object!\n");
		return;
	}

	newFX->alpha = gEngfuncs.pfnRandomFloat(0.6, 0.9);
	VectorCopy(drip->origin, newFX->origin);
	newFX->origin[2] = drip->minHeight; // correct position

	newFX->birthTime = gEngfuncs.GetClientTime();
	newFX->life = gEngfuncs.pfnRandomFloat(0.7, 1);
	newFX->inWater = inWater;

	// add to first place in chain
	newFX->p_Next = FirstChainFX.p_Next;
	newFX->p_Prev = &FirstChainFX;
	if (newFX->p_Next != NULL)
		newFX->p_Next->p_Prev = newFX;
	FirstChainFX.p_Next = newFX;

	fxcounter++;
}


void Weather::ProcessFXObjects()
{
	float curtime = gEngfuncs.GetClientTime();
	
	cl_rainfx* curFX = FirstChainFX.p_Next;
	cl_rainfx* nextFX = NULL;	

	while (curFX != NULL) // go through FX objects list
	{
		nextFX = curFX->p_Next; // save pointer to next
		
		// delete current?
		if ((curFX->birthTime + curFX->life) < curtime)
		{
			curFX->p_Prev->p_Next = curFX->p_Next; // link chain
			if (nextFX != NULL)
				nextFX->p_Prev = curFX->p_Prev; 
			delete curFX;					
			fxcounter--;
		}
		curFX = nextFX; // restore pointer
	}
}


void Weather::Reset()
{
	// delete all drips
	cl_drip* delDrip = FirstChainDrip.p_Next;
	FirstChainDrip.p_Next = NULL;
	
	while (delDrip != NULL)
	{
		cl_drip* nextDrip = delDrip->p_Next; // save pointer to next drip in chain
		delete delDrip;
		delDrip = nextDrip; // restore pointer
		dripcounter--;
	}

	// delete all FX objects
	cl_rainfx* delFX = FirstChainFX.p_Next;
	FirstChainFX.p_Next = NULL;
	
	while (delFX != NULL)
	{
		cl_rainfx* nextFX = delFX->p_Next;
		delete delFX;
		delFX = nextFX;
		fxcounter--;
	}

	Init();
}


void Weather::Init()
{
	WeatherData.dripsPerSecond = 0;
	WeatherData.distFromPlayer = 0;
	WeatherData.windX = 0;
	WeatherData.windY = 0;
	WeatherData.randX = 0;
	WeatherData.randY = 0;
	WeatherData.weatherMode = WEATHER_NONE;
	WeatherData.globalHeight = 0;

	FirstChainDrip.birthTime = 0;
	FirstChainDrip.minHeight = 0;
	FirstChainDrip.origin[0]=0;
	FirstChainDrip.origin[1]=0;
	FirstChainDrip.origin[2]=0;
	FirstChainDrip.alpha = 0;
	FirstChainDrip.xDelta = 0;
	FirstChainDrip.yDelta = 0;
	FirstChainDrip.landInWater = 0;
	FirstChainDrip.p_Next = NULL;
	FirstChainDrip.p_Prev = NULL;

	FirstChainFX.alpha = 0;
	FirstChainFX.birthTime = 0;
	FirstChainFX.life = 0;
	FirstChainFX.origin[0] = 0;
	FirstChainFX.origin[1] = 0;
	FirstChainFX.origin[2] = 0;
	FirstChainFX.p_Next = NULL;
	FirstChainFX.p_Prev = NULL;
	
	rain_oldtime = 0;
	rain_curtime = 0;
	rain_nextspawntime = 0;

	return;
}


void Weather::Parse( void )
{
	if ( WeatherData.distFromPlayer != 0 || WeatherData.dripsPerSecond != 0 || WeatherData.globalHeight != 0 ) return;

	char mapname[64];
	strcpy( mapname, gEngfuncs.pfnGetLevelName() );
	mapname[strlen(mapname) - 4] = 0; // cut .bsp

	std::string szFile( "scripts/" + std::string( mapname ) + ".txt" );
	KeyValuesAD kvData( "Weather" );
	if ( kvData->LoadFromFile( g_pFullFileSystem, szFile.c_str(), "GAME" ) )
	{
		WeatherData.dripsPerSecond = kvData->GetInt( "DripsPerSecond" );
		WeatherData.distFromPlayer = kvData->GetFloat( "DistFromPlayer" );
		WeatherData.windX = kvData->GetFloat( "WindSpeed_X" );
		WeatherData.windY = kvData->GetFloat( "WindSpeed_Y" );
		WeatherData.randX = kvData->GetFloat( "Random_X" );
		WeatherData.randY = kvData->GetFloat( "Random_Y" );
		WeatherData.globalHeight = kvData->GetFloat( "Height" );

		const char *szWeatherType = kvData->GetString( "Type", NULL );
		if ( szWeatherType && szWeatherType[0] )
		{
			if ( FStrEq( szWeatherType, "Snow" ) )
				WeatherData.weatherMode = WEATHER_SNOW;
			else if ( FStrEq( szWeatherType, "Rain" ) )
				WeatherData.weatherMode = WEATHER_RAIN;
			else if ( FStrEq( szWeatherType, "Dust" ) )
				WeatherData.weatherMode = WEATHER_DUST;
			else
				WeatherData.weatherMode = WEATHER_NONE;
		}
	}
}

void Weather::SetPoint( float x, float y, float z, float (*matrix)[4] )
{
	Vector point, result;
	point[0] = x;
	point[1] = y;
	point[2] = z;

	VectorTransform( point, matrix, result );

	gEngfuncs.pTriAPI->Vertex3f( result[0], result[1], result[2] );
}

void Weather::Draw()
{
	if ( FirstChainDrip.p_Next == NULL ) return;
	if ( !cl_weather_quality.GetBool() ) return;
	if ( WeatherData.weatherMode == WEATHER_NONE ) return;

	V_HSPRITE hsprTexture;
	const model_s *pTexture;
	float visibleHeight = WeatherData.globalHeight - SNOWFADEDIST;

	// Load our sprite texture
	switch ( WeatherData.weatherMode )
	{
		case WEATHER_RAIN:
		default:
			hsprTexture = LoadSprite( "sprites/effects/rain.spr" );
		break;

		case WEATHER_SNOW:
			hsprTexture = LoadSprite( "sprites/effects/snowflake.spr" );
		break;

		case WEATHER_DUST:
			hsprTexture = LoadSprite( "sprites/effects/fx_dust.spr" );
		break;
	}

	// usual triapi stuff
	pTexture = gEngfuncs.GetSpritePointer( hsprTexture );
	gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)pTexture, 0 );
	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );

	// go through drips list
	cl_drip* Drip = FirstChainDrip.p_Next;
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();

	switch ( WeatherData.weatherMode )
	{
		case WEATHER_RAIN:
		{
			while (Drip != NULL)
			{
				cl_drip* nextdDrip = Drip->p_Next;
					
				Vector2D toPlayer; 
				toPlayer.x = player->origin[0] - Drip->origin[0];
				toPlayer.y = player->origin[1] - Drip->origin[1];
				toPlayer = ZP::Normalize( toPlayer );
	
				toPlayer.x *= DRIP_SPRITE_HALFWIDTH;
				toPlayer.y *= DRIP_SPRITE_HALFWIDTH;

				float shiftX = (Drip->xDelta / DRIPSPEED) * DRIP_SPRITE_HALFHEIGHT;
				float shiftY = (Drip->yDelta / DRIPSPEED) * DRIP_SPRITE_HALFHEIGHT;

			// --- draw triangle --------------------------
				gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, Drip->alpha );
				gEngfuncs.pTriAPI->Begin( TRI_TRIANGLES );

					gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
					gEngfuncs.pTriAPI->Vertex3f( Drip->origin[0]-toPlayer.y - shiftX, Drip->origin[1]+toPlayer.x - shiftY,Drip->origin[2] + DRIP_SPRITE_HALFHEIGHT );

					gEngfuncs.pTriAPI->TexCoord2f( 0.5, 1 );
					gEngfuncs.pTriAPI->Vertex3f( Drip->origin[0] + shiftX, Drip->origin[1] + shiftY, Drip->origin[2]-DRIP_SPRITE_HALFHEIGHT );

					gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
					gEngfuncs.pTriAPI->Vertex3f( Drip->origin[0]+toPlayer.y - shiftX, Drip->origin[1]-toPlayer.x - shiftY, Drip->origin[2]+DRIP_SPRITE_HALFHEIGHT);

				gEngfuncs.pTriAPI->End();
			// --- draw triangle end ----------------------

				Drip = nextdDrip;
			}
		}
		break;

		case WEATHER_SNOW:
		{
			Vector normal;
			gEngfuncs.GetViewAngles((float*)normal);

			float matrix[3][4];
			AngleMatrix( normal, matrix );	// calc view matrix

			while (Drip != NULL)
			{
				cl_drip* nextdDrip = Drip->p_Next;

				matrix[0][3] = Drip->origin[0]; // write origin to matrix
				matrix[1][3] = Drip->origin[1];
				matrix[2][3] = Drip->origin[2];

				// apply start fading effect
				float alpha = (Drip->origin[2] <= visibleHeight) ? Drip->alpha : ((WeatherData.globalHeight - Drip->origin[2]) / (float)SNOWFADEDIST) * Drip->alpha;

			// --- draw quad --------------------------
				gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, alpha );
				gEngfuncs.pTriAPI->Begin( TRI_QUADS );

					gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
					SetPoint(0, SNOW_SPRITE_HALFSIZE ,SNOW_SPRITE_HALFSIZE, matrix);

					gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
					SetPoint(0, SNOW_SPRITE_HALFSIZE ,-SNOW_SPRITE_HALFSIZE, matrix);

					gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
					SetPoint(0, -SNOW_SPRITE_HALFSIZE ,-SNOW_SPRITE_HALFSIZE, matrix);

					gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
					SetPoint(0, -SNOW_SPRITE_HALFSIZE ,SNOW_SPRITE_HALFSIZE, matrix);

				gEngfuncs.pTriAPI->End();
			// --- draw quad end ----------------------

				Drip = nextdDrip;
			}
		}
		break;

		case WEATHER_DUST:
		{
			Vector normal;
			gEngfuncs.GetViewAngles((float*)normal);

			float matrix[3][4];
			AngleMatrix( normal, matrix );	// calc view matrix

			while (Drip != NULL)
			{
				cl_drip* nextdDrip = Drip->p_Next;

				matrix[0][3] = Drip->origin[0]; // write origin to matrix
				matrix[1][3] = Drip->origin[1];
				matrix[2][3] = Drip->origin[2];

				// apply start fading effect
			    float alpha = (Drip->origin[2] <= visibleHeight) ? Drip->alpha : ((WeatherData.globalHeight - Drip->origin[2]) / (float)SNOWFADEDIST) * Drip->alpha;

			// --- draw quad --------------------------
				gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, alpha );
				gEngfuncs.pTriAPI->Begin( TRI_QUADS );

					gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
					SetPoint(0, DUST_SPRITE_HALFSIZE ,DUST_SPRITE_HALFSIZE, matrix);

					gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
					SetPoint(0, DUST_SPRITE_HALFSIZE ,-DUST_SPRITE_HALFSIZE, matrix);

					gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
					SetPoint(0, -DUST_SPRITE_HALFSIZE ,-DUST_SPRITE_HALFSIZE, matrix);

					gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
					SetPoint(0, -DUST_SPRITE_HALFSIZE ,DUST_SPRITE_HALFSIZE, matrix);

				gEngfuncs.pTriAPI->End();
			// --- draw quad end ----------------------

				Drip = nextdDrip;
			}
		}
		break;
	}

	// Draw our effects
	DrawFXObjects();
}

void Weather::DrawFXObjects()
{
	if ( FirstChainFX.p_Next == NULL ) return;

	float curtime = gEngfuncs.GetClientTime();

	// go through objects list
	cl_rainfx* curFX = FirstChainFX.p_Next;
	while (curFX != NULL)
	{
		cl_rainfx* nextFX = curFX->p_Next;

		// usual triapi stuff
		V_HSPRITE hsprTexture;
		const model_s *pTexture;
		if ( curFX->inWater )
			hsprTexture = LoadSprite( RANDOM_LONG(0, 1) == 1 ? "sprites/effects/rain_splash_01.spr" : "sprites/effects/rain_splash_02.spr" );
		else
			hsprTexture = LoadSprite( "sprites/effects/rainsplash.spr" );
		pTexture = gEngfuncs.GetSpritePointer( hsprTexture );
		gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)pTexture, 0 );
		gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
		gEngfuncs.pTriAPI->CullFace( TRI_NONE );

		// fadeout
		float alpha = ((curFX->birthTime + curFX->life - curtime) / curFX->life) * curFX->alpha;
		float size = (curtime - curFX->birthTime) * MAXRINGHALFSIZE;

		// --- draw quad --------------------------
		gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, alpha );
		gEngfuncs.pTriAPI->Begin( TRI_QUADS );

			gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
			gEngfuncs.pTriAPI->Vertex3f(curFX->origin[0] - size, curFX->origin[1] - size, curFX->origin[2]);

			gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
			gEngfuncs.pTriAPI->Vertex3f(curFX->origin[0] - size, curFX->origin[1] + size, curFX->origin[2]);

			gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
			gEngfuncs.pTriAPI->Vertex3f(curFX->origin[0] + size, curFX->origin[1] + size, curFX->origin[2]);

			gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
			gEngfuncs.pTriAPI->Vertex3f(curFX->origin[0] + size, curFX->origin[1] - size, curFX->origin[2]);

		gEngfuncs.pTriAPI->End();
		// --- draw quad end ----------------------

		curFX = nextFX;
	}
}