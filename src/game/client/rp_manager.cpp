// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "hud.h"
#include "rp_manager.h"

#include "steam/steam_api.h"

struct StringTable_s
{
	const char *Key;
	const char *Value;
};

StringTable_s GameModes[] = {
	{ "zp_", "Survival" },
	{ "zpo", "Objective" },
	{ "zph", "Hardcore" }
};

StringTable_s OfficialMaps[] = {
	{ "clubzombo", "Club Zombo" },
	{ "contingency", "Contingency" },
	{ "eastside", "East Side" },
	{ "haunted", "Haunted" },
	{ "industry", "Industry" },
	{ "santeria", "Santeria" },
	{ "thelabs", "The Labs" }
};

const char *GetGameMode( const char *szMap )
{
	// Only read the first 3 characters.
	char buf[4];
	buf[0] = 0;
	V_strcpy_safe( buf, szMap );

	// Loop through our array
	for ( size_t i = 0; i < ARRAYSIZE( GameModes ); i++ )
	{
		StringTable_s table = GameModes[ i ];
		if ( FStrEq( buf, table.Key ) )
			return table.Value;
	}

	// We found nothing, return dev mode.
	return "Dev";
}

const char *GetMapName( const char *szMap )
{
	// Strip away the first 3 characters.
	char buf[32];
	buf[0] = 0;
	V_strcpy_safe( buf, szMap + 3 );
	// If we find _, just increase it by 1 more character.
	if ( buf[0] == '_' )
		V_strcpy_safe( buf, szMap + 4 );

	// Loop through our array
	for ( size_t i = 0; i < ARRAYSIZE( OfficialMaps ); i++ )
	{
		StringTable_s table = OfficialMaps[ i ];
		if ( FStrEq( buf, table.Key ) )
			return table.Value;
	}

	// We found nothing, return the map
	return szMap;
}

CRichPresenceManager *g_RPManagerInstance = nullptr;

CRichPresenceManager::CRichPresenceManager()
{
	g_RPManagerInstance = this;
	Reset();
}

CRichPresenceManager *CRichPresenceManager::GetInstance()
{
	return g_RPManagerInstance;
}

void CRichPresenceManager::Reset()
{
	m_szLevelName[0] = 0;
	m_iCurrentRounds = -1;
}

void CRichPresenceManager::UpdatePresence()
{
	char buf[64];
	buf[0] = 0;
	V_FileBase( gEngfuncs.pfnGetLevelName(), buf, sizeof(buf) );

	// Are we connected or not?
	bool bNotConnected = false;
	if ( FStrEq( buf, "" ) )
		bNotConnected = true;

	// Are we allowed to update?
	bool bShouldUpdate = false;

	// We have a map name, but we aren't connected no more.
	if ( ( m_szLevelName && m_szLevelName[0] ) && bNotConnected )
		bShouldUpdate = true;
	// Check if the map isn't the same
	else if ( !FStrEq( buf, m_szLevelName ) )
		bShouldUpdate = true;
	// Check our current rounds.
	else if ( gHUD.m_MapRounds != m_iCurrentRounds )
		bShouldUpdate = true;

	// Only update if we require one.
	if ( !bShouldUpdate ) return;

	// Save our data
	m_iCurrentRounds = gHUD.m_MapRounds;
	V_strcpy_safe( m_szLevelName, buf );

	// Steam Rich Presence
	if ( GetSteamAPI() )
	{
		// Set our round state
		char szBuffer[32];
		Q_snprintf( szBuffer, sizeof( szBuffer ), "Round %i", gHUD.m_MapRounds );
		GetSteamAPI()->SteamFriends()->SetRichPresence( "zp:rounds", szBuffer );

		const char *szGameMode = GetGameMode( m_szLevelName );
		const char *szMap = GetMapName( m_szLevelName );

		//V_strcpy_safe( szBuffer, "%game:map_name%" );
		//V_strcpy_safe( szBuffer, "%game:raw_map_name%" );
		Q_snprintf( szBuffer, sizeof( szBuffer ), "%s: %s", szGameMode, szMap );
		GetSteamAPI()->SteamFriends()->SetRichPresence( "zp:info", szBuffer );

		// Apply our presence
		GetSteamAPI()->SteamFriends()->SetRichPresence( "steam_display", bNotConnected ? "#HL_RP_MainMenu" : "#HL_RP_TeamplayUnknown" );

		// Update it
		gEngfuncs.pfnClientCmd( "richpresence_update\n" );
	}

	// Discord Rich Presence
	// NOTE: Should we add it?
}