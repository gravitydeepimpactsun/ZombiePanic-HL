/***
*
*	Copyright (c) 1996-2004, Shambler Team. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Shambler Team.  All other use, distribution, or modification is prohibited
*   without written permission from Shambler Team.
*
****/
/*
====== weather.h ========================================================
*/
#ifndef __RAIN_H__
#define __RAIN_H__

#define DRIPSPEED		900		// speed of raindrips (pixel per secs)
#define SNOWSPEED		200		// speed of snowflakes
#define SNOWFADEDIST	80

#define MAXDRIPS_LOW			150
#define MAXDRIPS_MEDIUM			800
#define MAXDRIPS_HIGH			2000
#define MAXDRIPS_VERYHIGH		3000
#define MAXFX_LOW				150
#define MAXFX_MEDIUM			500
#define MAXFX_HIGH				1000
#define MAXFX_VERYHIGH			2000

#define DRIP_SPRITE_HALFHEIGHT	46
#define DRIP_SPRITE_HALFWIDTH	8

#define SNOW_SPRITE_HALFSIZE	3
#define DUST_SPRITE_HALFSIZE	256

// radius water rings
#define MAXRINGHALFSIZE		25	

// Johan: Updated the rain code to be "weather" instead.
// Makes it easier for us, since we have multiple choices, instead of just rain.
// Snow variant is something I created for Condition Zero rebuild.
enum WeatherMode_t
{
	WEATHER_NONE = -1,
	WEATHER_SNOW = 0,
	WEATHER_RAIN,
	WEATHER_DUST
};

typedef struct
{
	int					dripsPerSecond;
	float				distFromPlayer;
	float				windX, windY;
	float				randX, randY;
	WeatherMode_t		weatherMode;
	float				globalHeight;
} weather_properties;


typedef struct cl_drip
{
	float		birthTime;
	float		minHeight;	// minimal height to kill raindrop
	Vector		origin;
	float		alpha;

	float		xDelta;		// side speed
	float		yDelta;
	int			landInWater;
	
	cl_drip*		p_Next;		// next drip in chain
	cl_drip*		p_Prev;		// previous drip in chain
} cl_drip_t;

typedef struct cl_rainfx
{
	float		birthTime;
	float		life;
	Vector		origin;
	float		alpha;
	bool		inWater;
	
	cl_rainfx*		p_Next;		// next fx in chain
	cl_rainfx*		p_Prev;		// previous fx in chain
} cl_rainfx_t;

typedef enum
{
	WEATHER_OFF = 0,
	WEATHER_LOW,
	WEATHER_MEDIUM,
	WEATHER_HIGH,
	WEATHER_VERYHIGH
} WEATHER_QUALITY;

namespace Weather
{
	void Init( void );
	void Reset( void );
	void Parse( void );
	void Process( void );
	void ProcessFXObjects( void );
	void WaterLandingEffect( cl_drip *drip, bool inWater );

	void SetPoint( float x, float y, float z, float (*matrix)[4] );
	void Draw();
	void DrawFXObjects();
}

#endif