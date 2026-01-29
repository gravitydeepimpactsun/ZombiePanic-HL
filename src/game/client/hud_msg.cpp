/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  hud_msg.cpp
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "hud/ammo.h"
#include "hud/status_icons.h"
#include "hud/chat.h"
#include "engine_builds.h"
#include "zp/zp_shared.h"
#include "zp/hud/zp_beacons.h"
#include "vgui/team_menu.h"
#include "vgui/client_viewport.h"

#include <vgui/ILocalize.h>

#include "particleman.h"
extern IParticleMan *g_pParticleMan;

#include "fog.h"
#include "sphl/weather.h"
#include "zp/zp_apicallback.h"
#include "zp/hud/zp_progressbar.h"

extern weather_properties WeatherData;

#define MAX_CLIENTS 32

#if !defined(_TFC)
extern BEAM *pBeam;
extern BEAM *pBeam2;
#endif

#if defined(_TFC)
void ClearEventList(void);
#endif

extern ConVar zoom_sensitivity_ratio;
extern float g_lastFOV;

#if defined(CLIENT_WEAPONS)
cvar_t *cl_lw = nullptr;
#endif

ClientAPIData_t g_ClientAPIDataArray[ MAX_PLAYERS ] = {};
ClientAPIData_t GetClientAPIData( int iClient )
{
	if ( iClient >= 0 && iClient < MAX_PLAYERS )
		return g_ClientAPIDataArray[iClient];
	return {};
}

void CAM_ToFirstPerson(void);
float IN_GetMouseSensitivity();

/// USER-DEFINED SERVER MESSAGE HANDLERS

int CHud::MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	// prepare all hud data
	for (CHudElem *i : m_HudList)
		i->InitHudData();

#if defined(_TFC)
	ClearEventList();

	// catch up on any building events that are going on
	gEngfuncs.pfnServerCmd("sendevents");
#endif

	if (g_pParticleMan)
		g_pParticleMan->ResetParticles();

#if !defined(_TFC)
	//Probably not a good place to put this.
	pBeam = pBeam2 = NULL;
#endif

	return 1;
}

int CHud::MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	// clear all hud data
	for (CHudElem *i : m_HudList)
		i->Reset();

	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;

	return 1;
}

int CHud::MsgFunc_Logo(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

int CHud::MsgFunc_Damage(const char *pszName, int iSize, void *pbuf)
{
	int armor, blood;
	Vector from;
	int i;
	float count;

	BEGIN_READ(pbuf, iSize);
	armor = READ_BYTE();
	blood = READ_BYTE();

	for (i = 0; i < 3; i++)
		from[i] = READ_COORD();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return 1;
}

int CHud::MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_GameMode = (ZP::GameModeType_e)READ_BYTE();

	if (GetEngineBuild() >= ENGINE_BUILD_ANNIVERSARY_FIRST)
	{
		if (m_GameMode)
			gEngfuncs.pfnClientCmd("richpresence_gamemode Teamplay\n");
		else
			gEngfuncs.pfnClientCmd("richpresence_gamemode\n"); // reset

		gEngfuncs.pfnClientCmd("richpresence_update\n");
	}

	return 1;
}

int CHud::MsgFunc_Rounds(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_MapRounds = READ_BYTE();
	CTeamMenu *pMenu = (CTeamMenu *)g_pViewport->GetViewPanel( MENU_TEAM );
	if ( pMenu )
		pMenu->SetCurrentRound();
	return 1;
}

int CHud::MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf)
{
	CAM_ToFirstPerson();
	return 1;
}

int CHud::MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int newfov = READ_BYTE();
	int def_fov = default_fov.GetInt();

#if defined(CLIENT_WEAPONS)
	//Weapon prediction already takes care of changing the fog. ( g_lastFOV ).
	if (cl_lw && cl_lw->value)
		return 1;
#endif

	g_lastFOV = newfov;

	if (newfov == 0)
	{
		m_iFOV = def_fov;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iFOV == def_fov)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = IN_GetMouseSensitivity() * ((float)newfov / (float)def_fov) * zoom_sensitivity_ratio.GetFloat();
	}

	return 1;
}

int CHud::MsgFunc_Concuss(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_iConcussionEffect = READ_BYTE();
	if (m_iConcussionEffect)
		CHudStatusIcons::Get()->EnableIcon("dmg_concuss", 255, 160, 0);
	else
		CHudStatusIcons::Get()->DisableIcon("dmg_concuss");
	return 1;
}

int CHud::MsgFunc_Fog(const char *pszName, int iSize, void *pbuf)
{
	FogParams fogParams;

	BEGIN_READ(pbuf, iSize);

	int startdist = READ_COORD();
	int enddist = READ_COORD();

	Vector fogcolor;
	for (int i = 0; i < 3; i++)
		fogcolor[i] = (float)READ_BYTE() / 255.0f;
	VectorCopy( fogcolor, fogParams.color );

	float blendtime = READ_COORD();
	float fogdensity = READ_COORD();

	if (READ_OK())
	{
		fogParams.fogStart = startdist;
		fogParams.fogEnd = enddist;
		fogParams.density = fogdensity;
		fogParams.flBlendTime = blendtime;
		fogParams.fogSkybox = true;
		gFog.SetFogParameters( fogParams );
	}
	else
	{
		gFog.ClearFog();
	}

	return 1;
}

int CHud::MsgFunc_Weather(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ( pbuf, iSize );
		WeatherData.dripsPerSecond =	READ_SHORT();
		WeatherData.distFromPlayer =	READ_COORD();
		WeatherData.windX =				READ_COORD();
		WeatherData.windY =				READ_COORD();
		WeatherData.randX =				READ_COORD();
		WeatherData.randY =				READ_COORD();
		WeatherData.weatherMode =		(WeatherMode_t)READ_SHORT();
		WeatherData.globalHeight =		READ_COORD();

	return 1;
}

int CHud::MsgFunc_Timer(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_GameTimer.RoundTimer = READ_FLOAT();
	m_GameTimer.GameTime = READ_SHORT();
	return 1;
}


int CHud::MsgFunc_BeaconDraw(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	// Setup our beacon data
	CZPBeacons::BeaconData beacon;
	beacon.id = READ_SHORT();
	beacon.active = ( READ_BYTE() == 1 ) ? true : false;
	beacon.important = ( READ_BYTE() == 1 ) ? true : false;
	beacon.type = (BeaconTypes)READ_SHORT();
	beacon.drawtype = (BeaconDrawTypes)READ_SHORT();
	beacon.drawHealth = ( READ_BYTE() == 1 ) ? true : false;
	int iHealth = READ_SHORT();
	beacon.health = clamp( iHealth, 0, 100 );
	beacon.teamfilter = READ_SHORT();
	beacon.position.x = READ_COORD();
	beacon.position.y = READ_COORD();
	beacon.position.z = READ_COORD();
	beacon.range = READ_FLOAT();

	static char szText[512];
	szText[0] = 0;
	strncpy( szText, READ_STRING(), sizeof( szText ) );
	beacon.text = szText;

	strncpy( szText, READ_STRING(), sizeof( szText ) );
	beacon.text_zombie = szText;

	// Add the beacon to our list (or update it if it already exists)
	CZPBeacons::Get()->AddBeacon( beacon );
	return 1;
}


int CHud::MsgFunc_BeaconReset(const char *pszName, int iSize, void *pbuf)
{
	// We don't read anything from the message, just clear the list
	CZPBeacons::Get()->ResetBeacons();
	return 1;
}


int CHud::MsgFunc_Panic(const char *pszName, int iSize, void *pbuf)
{
	// Play the heartbeat, that's it.
	gEngfuncs.pfnClientCmd( "play vo/shared/panic_heartbeat.wav" );
	return 1;
}


int CHud::MsgFunc_Voice(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	int iPlayer = READ_SHORT();
	PlayerVocalizeType nType = (PlayerVocalizeType)READ_SHORT();

	// Only allow if the local player is on the human team.
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer ) return 1;
	if ( !localplayer->IsConnected() ) return 1;
	if ( localplayer->GetTeamNumber() != ZP::TEAM_SURVIVIOR ) return 1;

	wchar_t *pVocalizeValue = nullptr;
	switch( nType )
	{
		case PlayerVocalizeType::VOCALIZE_AGREE: pVocalizeValue = g_pVGuiLocalize->Find( "ZP_Vocalize_Agree" ); break;
		case PlayerVocalizeType::VOCALIZE_DECLINE: pVocalizeValue = g_pVGuiLocalize->Find( "ZP_Vocalize_Decline" ); break;
		case PlayerVocalizeType::VOCALIZE_COVER: pVocalizeValue = g_pVGuiLocalize->Find( "ZP_Vocalize_Cover" ); break;
		case PlayerVocalizeType::VOCALIZE_NEED_AMMO: pVocalizeValue = g_pVGuiLocalize->Find( "ZP_Vocalize_INeedAmmo" ); break;
		case PlayerVocalizeType::VOCALIZE_NEED_WEAPON: pVocalizeValue = g_pVGuiLocalize->Find( "ZP_Vocalize_INeedWeapon" ); break;
		case PlayerVocalizeType::VOCALIZE_HOLD_HERE: pVocalizeValue = g_pVGuiLocalize->Find( "ZP_Vocalize_Hold" ); break;
		case PlayerVocalizeType::VOCALIZE_OPEN_FIRE: pVocalizeValue = g_pVGuiLocalize->Find( "ZP_Vocalize_Fire" ); break;
	}
	if ( !pVocalizeValue ) return 1;

	wchar_t *pFormat = g_pVGuiLocalize->Find( "ZP_Vocalize_Format" );
	if ( !pFormat ) return 1;

	wchar_t wcPlayerName[32];
	char szPlayerName[32];
	if ( CPlayerInfo *pi = GetPlayerInfoSafe( iPlayer ) )
		Q_snprintf( szPlayerName, sizeof( szPlayerName ), "%s", pi->Update()->GetDisplayName() );
	else
		Q_snprintf( szPlayerName, sizeof( szPlayerName ), "Unknown" );
	g_pVGuiLocalize->ConvertANSIToUnicode( szPlayerName, wcPlayerName, sizeof( wcPlayerName ) );

	wchar_t wcOutput[512];
	g_pVGuiLocalize->ConstructString(
	    wcOutput, sizeof(wcOutput),
	    pFormat, 2,
	    wcPlayerName,
	    pVocalizeValue
	);

	char szOut[512];
	g_pVGuiLocalize->ConvertUnicodeToANSI( wcOutput, szOut, sizeof(szOut) );

	CHudChat::Get()->ChatPrintf( -1, "%s", szOut );
	return 1;
}


int CHud::MsgFunc_APICheck(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	int iPlayer = READ_SHORT();
	int iGame = READ_SHORT();
	int iTier = READ_SHORT();
	static char szKey[512];
	szKey[0] = 0;
	strncpy( szKey, READ_STRING(), sizeof( szKey ) );

	if ( iPlayer < 0 || iPlayer >= MAX_PLAYERS ) return 1;
	g_ClientAPIDataArray[ iPlayer ].Game = (eGameAPIVersion)iGame;
	g_ClientAPIDataArray[ iPlayer ].Tier = (eSupporterTier)iTier;
	g_ClientAPIDataArray[ iPlayer ].Key = szKey;
	return 1;
}

int CHud::MsgFunc_Barricade(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	float fBuiltTime = READ_FLOAT();
	CZPProgressBar::Get()->DrawText( "Barricading...", fBuiltTime );
	return 1;
}

int CHud::MsgFunc_DebugLine(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	Vector start;
	Vector end;
	for (int i = 0; i < 3; i++)
	{
		start[i] = READ_COORD();
	}
	for (int i = 0; i < 3; i++)
	{
		end[i] = READ_COORD();
	}
	int r = READ_BYTE();
	int g = READ_BYTE();
	int b = READ_BYTE();
	float life = READ_FLOAT();
	// TODO: Create a debug overlay system.
	//DebugOverlay::AddDebugLine( start, end, r, g, b, life, false );
	return 1;
}