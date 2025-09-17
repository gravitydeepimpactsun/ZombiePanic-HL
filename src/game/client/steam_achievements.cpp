// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "steam_achievements.h"
#include "gameui/gameui_viewport.h"

// ================================================================= \\

// Let's construct our MapStatCheck_t array here.

#define _MAP_STAT(map, mapid, statid) { mapid, #map, statid }
static MapStatCheck_t g_MapStatChecks[] = {
	// Survival maps
	_MAP_STAT(zp_santeria, MAPSTAT_SURVIVAL, MAP_ZP_SANTERIA),
	_MAP_STAT(zp_clubzombo, MAPSTAT_SURVIVAL, MAP_ZP_CLUBZOMBO),
	_MAP_STAT(zp_thelabs, MAPSTAT_SURVIVAL, MAP_ZP_THELABS),
	_MAP_STAT(zp_eastside, MAPSTAT_SURVIVAL, MAP_ZP_EASTSIDE),
	_MAP_STAT(zp_industry, MAPSTAT_SURVIVAL, MAP_ZP_INDUSTRY),
	_MAP_STAT(zp_contingency, MAPSTAT_SURVIVAL, MAP_ZP_CONTINGENCY),
	_MAP_STAT(zp_haunted, MAPSTAT_SURVIVAL, MAP_ZP_HAUNTED),

	// Objective maps
	_MAP_STAT(zpo_contingency, MAPSTAT_OBJECTIVE, MAP_ZPO_CONTINGENCY),

	// Hardcore maps
	_MAP_STAT(zph_haunted, MAPSTAT_HARDCORE, MAP_ZPH_HAUNTED),
	_MAP_STAT(zph_santeria, MAPSTAT_HARDCORE, MAP_ZPH_SANTERIA),
	_MAP_STAT(zph_contingency, MAPSTAT_HARDCORE, MAP_ZPH_CONTINGENCY),
};

// ================================================================= \\

bool CLIENT_UTIL_IsSpecialAchievement( int iAchievement )
{
	if ( iAchievement == EAchievements::JACKOFTRADES ) return true;
	if ( iAchievement == EAchievements::PARTNERINCRIME ) return true;
	if ( iAchievement == EAchievements::MARATHON ) return true;
	if ( iAchievement == EAchievements::PLAY_ALL_SURVIVAL ) return true;
	if ( iAchievement == EAchievements::PLAY_ALL_OBJECTIVE ) return true;
	return false;
}

bool CLIENT_UTIL_HasEarnedAchievement( int iAchievement )
{
	bool bEarned = false;
	GetSteamAPI()->SteamUserStats()->GetAchievement( GetAchievementByID( iAchievement ).GetAchievementName(), &bEarned );
	return bEarned;
}

int32 CLIENT_UTIL_GetStatAchievement( int iAchievement )
{
	int32 value;
	GetSteamAPI()->SteamUserStats()->GetStat( GetAchievementByID( iAchievement ).GetData().Name, &value );
	return value;
}

static bool PlayingWithAFriend()
{
	for (int i = 1; i <= MAX_PLAYERS; i++)
	{
		CPlayerInfo *pi = GetPlayerInfo(i)->Update();
		if ( !pi ) continue;
		if ( !pi->IsConnected() ) continue;
		CSteamID steamfriend( pi->GetValidSteamID64() );
		if ( !steamfriend.IsValid() ) continue;
		if ( GetSteamAPI()->SteamFriends()->GetFriendRelationship( steamfriend ) == k_EFriendRelationshipFriend )
		{
			// Check our local player with this player, and make sure we are on the same team!
			CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
			if ( !localplayer ) continue;
			if ( !localplayer->IsConnected() ) continue;
			if ( localplayer->GetTeamNumber() != pi->GetTeamNumber() ) continue;
			return true;
		}
	}
	return false;
}

StatData_t GrabMapStat( MapStatCheck nCheck )
{
	// Get the map name
	char mapname[64];
	strcpy( mapname, gEngfuncs.pfnGetLevelName() );

	// Remove the "maps/" part if it exists.
	if ( V_stristr( mapname, "maps/" ) == mapname )
	{
		// Move the string 5 characters forward.
		memmove( mapname, mapname + 5, strlen(mapname) - 4 );
	}

	// Remove the extension if it exists.
	char *dot = V_stristr( mapname, ".bsp" );
	if ( dot ) *dot = '\0';

	// Make sure our mapname is lowercase.
	V_strlower( mapname );

	// mapname should now return something like "zp_santeria"
	// instead of "maps/ZP_santeria.bsp" (official maps does not use uppercase, but it's just an example.)

	// Now, check our array for a match.
	for ( size_t i = 0; i < ARRAYSIZE(g_MapStatChecks); i++ )
	{
		if ( g_MapStatChecks[i].Check != nCheck ) continue;
		if ( !V_stricmp( g_MapStatChecks[i].MapName, mapname ) )
		{
			return GrabStat( g_MapStatChecks[i].StatID );
		}
	}

	// If we reach here, return invalid.
	return GrabStat( INVALID_STAT );
}

// Similar to the IndicateAchievementProgress below,
// but this is used for map progress and does not update
// the actual achievement itself (or the stat).
static bool NotifyMapProgress( int iAchievement )
{
	if ( !GetSteamAPI() ) return false;
	if ( !GetSteamAPI()->SteamUserStats() ) return false;

	DialogAchievementData ach = GetAchievementByID( iAchievement );
	int32 value = 0;
	int32 maxvalue = ach.GetRequiredStepCount();
	for ( size_t i = 0; i < maxvalue; i++ )
	{
		// Check if this step is completed.
		if ( GrabStat( ach.GetRequiredStepID( i ).Stat ).Value == 1 )
			value++;
	}

	// Check if we have enough.
	if ( value < maxvalue )
	{
		GetSteamAPI()->SteamUserStats()->IndicateAchievementProgress(
			ach.GetAchievementName(),
			value,
			maxvalue
		);
		return true;
	}

	return false;
}

// Increase the map stat by one.
// This is used for achievements like "Play all survival maps", "Play all objective maps" etc.
// It's also used by marathon achievement, which is why this is a simple static function.
static bool SetMapStat( MapStatCheck nCheck )
{
	StatData_t stat = GrabMapStat( nCheck );
	if ( stat.ID == INVALID_STAT ) return false;
	if ( stat.Value == 1 ) return false; // Already completed.
	stat.Value++;
	SetStat( stat.ID, stat.Value );
	GetSteamAPI()->SteamUserStats()->SetStat( stat.Name, stat.Value );
	return true;
}

static bool CheckIfAllMapsAreCompleted( MapStatCheck nCheck )
{
	// Check if all maps of this type are completed.
	bool bAllCompleted = true;

	// Go trough our array, and check if all maps of this type are completed.
	// They need to have a value of 1 or higher.
	for ( size_t i = 0; i < ARRAYSIZE( g_MapStatChecks ); i++ )
	{
		if ( g_MapStatChecks[i].Check != nCheck ) continue;
		StatData_t stat = GrabStat( g_MapStatChecks[i].StatID );
		if ( stat.ID == INVALID_STAT || stat.Value < 1 )
		{
			bAllCompleted = false;
			break;
		}
	}

	return bAllCompleted;
}

static bool CheckIfMarathonIsComplete()
{
	// Check if all maps of this type are completed.
	bool bAllCompleted = true;

	// Same as CheckIfAllMapsAreCompleted, but check for all map types.
	for ( size_t i = 0; i < ARRAYSIZE( g_MapStatChecks ); i++ )
	{
		StatData_t stat = GrabStat( g_MapStatChecks[i].StatID );
		if ( stat.ID == INVALID_STAT || stat.Value < 1 )
		{
			bAllCompleted = false;
			break;
		}
	}

	return bAllCompleted;
}

void CLIENT_UTIL_GiveAchievement( int iAchievement )
{
	if ( !GetSteamAPI() ) return;
	if ( !GetSteamAPI()->SteamUserStats() ) return;

	bool bEarned = false;
	GetSteamAPI()->SteamUserStats()->GetAchievement( GetAchievementByID( iAchievement ).GetAchievementName(), &bEarned );

	// Already have it? Skip.
	if ( bEarned ) return;

	// If jack of trades, check
	if ( CLIENT_UTIL_IsSpecialAchievement( iAchievement ) )
	{
		switch ( iAchievement )
		{
			case EAchievements::JACKOFTRADES:
			{
				// Check each of these, and make sure we got them all first!
			    if ( !CLIENT_UTIL_GetStatAchievement( EAchievements::KILLS_CROWBAR ) ) return;
			    if ( !CLIENT_UTIL_GetStatAchievement( EAchievements::KILLS_MP5 ) ) return;
			    if ( !CLIENT_UTIL_GetStatAchievement( EAchievements::KILLS_PISTOL ) ) return;
			    if ( !CLIENT_UTIL_GetStatAchievement( EAchievements::KILLS_REVOLVER ) ) return;
			    if ( !CLIENT_UTIL_GetStatAchievement( EAchievements::KILLS_RIFLE ) ) return;
			    if ( !CLIENT_UTIL_GetStatAchievement( EAchievements::KILLS_SATCHEL ) ) return;
			    if ( !CLIENT_UTIL_GetStatAchievement( EAchievements::KILLS_SHOTGUN ) ) return;
			    if ( !CLIENT_UTIL_GetStatAchievement( EAchievements::KILLS_TNT ) ) return;
			}
			break;

		    case EAchievements::PARTNERINCRIME:
			{
				// Make sure we are playing with a friend.
				// And make sure we are on the same team!
			    if ( !PlayingWithAFriend() ) return;
			}
			break;

		    case EAchievements::MARATHON:
		    {
			    if ( SetMapStat( MAPSTAT_SURVIVAL )
					|| SetMapStat( MAPSTAT_OBJECTIVE )
					|| SetMapStat( MAPSTAT_HARDCORE ) )
				    NotifyMapProgress( iAchievement );
			    if ( !CheckIfMarathonIsComplete() )
					return;
			}
			break;

		    case EAchievements::PLAY_ALL_SURVIVAL:
		    {
			    if ( SetMapStat( MAPSTAT_SURVIVAL ) )
				    NotifyMapProgress( iAchievement );
				if ( !CheckIfAllMapsAreCompleted( MAPSTAT_SURVIVAL ) )
					return;
			}
			break;

		    case EAchievements::PLAY_ALL_OBJECTIVE:
		    {
			    if ( SetMapStat( MAPSTAT_OBJECTIVE ) )
				    NotifyMapProgress( iAchievement );
				if ( !CheckIfAllMapsAreCompleted( MAPSTAT_OBJECTIVE ) )
					return;
			}
			break;

			// TODO: Add PLAY_ALL_HARDCORE
			// and maybe other special achievements.
		}
	}

	// Do we have a stat value?
	if ( GetAchievementByID( iAchievement ).HasStatID() )
	{
		int32 value;
		GetSteamAPI()->SteamUserStats()->GetStat( GetAchievementByID( iAchievement ).GetData().Name, &value );

		// Increase it by one.
		value++;
		GetSteamAPI()->SteamUserStats()->SetStat( GetAchievementByID( iAchievement ).GetData().Name, value );

		// Update it.
		SetStat( GetAchievementByID( iAchievement ).GetData().ID, value );

		// Check if we have enough.
		if ( value < GetAchievementByID( iAchievement ).GetData().MaxValue )
		{
			double dValue = value;
			bool bShouldDraw = false;
			// 25%, 50% or 90% of the way?
			if ( value == int32(dValue * 0.25) ) bShouldDraw = true;
			else if ( !bShouldDraw && value == int32(dValue * 0.5) ) bShouldDraw = true;
			else if ( !bShouldDraw && value == int32(dValue * 0.9) ) bShouldDraw = true;
			if ( bShouldDraw )
				GetSteamAPI()->SteamUserStats()->IndicateAchievementProgress(
					GetAchievementByID( iAchievement ).GetAchievementName(),
					GetAchievementByID( iAchievement ).GetData().Value,
					GetAchievementByID( iAchievement ).GetData().MaxValue
				);
			return;
		}
	}

	// Because GoldSource don't want us to show our progress ingame, let's do some hacky shit instead.
	// We can only do this before we execute SetAchievement function.
	GetSteamAPI()->SteamUserStats()->IndicateAchievementProgress( GetAchievementByID( iAchievement ).GetAchievementName(), 0, 1 );

	// Give the achievement.
	GetSteamAPI()->SteamUserStats()->SetAchievement( GetAchievementByID( iAchievement ).GetAchievementName() );

	// Tell the server, that we earned it!
	gEngfuncs.pfnServerCmd( vgui2::VarArgs( "achearn %i", iAchievement ) );
}
