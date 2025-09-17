// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef STEAM_ACHIEVEMENTS_H
#define STEAM_ACHIEVEMENTS_H

#include "zp/ui/achievements/C_AchievementDialog.h"
#include "zp/zp_achievements.h"
#include "steam/steam_api.h"

StatData_t GrabStat( EStats nID );
void SetStat( EStats nID, int32 value );
bool CLIENT_UTIL_HasEarnedAchievement( int iAchievement );
int32 CLIENT_UTIL_GetStatAchievement( int iAchievement );
bool CLIENT_UTIL_IsSpecialAchievement( int iAchievement );
void CLIENT_UTIL_GiveAchievement( int iAchievement );

enum MapStatCheck
{
	MAPSTAT_NONE = 0,
	MAPSTAT_SURVIVAL,
	MAPSTAT_OBJECTIVE,
	MAPSTAT_HARDCORE
};

struct MapStatCheck_t
{
	MapStatCheck Check;
	const char *MapName;
	EStats StatID;
};

StatData_t GrabMapStat( MapStatCheck nCheck );

#endif // STEAM_ACHIEVEMENTS_H