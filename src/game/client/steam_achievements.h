// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef STEAM_ACHIEVEMENTS_H
#define STEAM_ACHIEVEMENTS_H

#include "zp/ui/achievements/C_AchievementDialog.h"
#include "zp/zp_achievements.h"
#include "steam/steam_api.h"

// Used by our custom achievements notification system.
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "hud/base.h"

StatData_t GrabStat( EStats nID );
void SetStat( EStats nID, int32 value );
void STAT_OnUserStatsReceived( UserStatsReceived_t *pCallback );
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

// Let's construct our class for our HUD element for our achievements notification.
class CHudAchievementNotification : public CHudElemBase<CHudAchievementNotification>, public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHudAchievementNotification, vgui2::EditablePanel );

public:
	CHudAchievementNotification();
	void ApplySchemeSettings( vgui2::IScheme *pScheme ) override;
	void Init( void ) override;
	void VidInit( void ) override;
	void Think( void ) override;
	void Paint( void ) override;
	void ClearData();
	void PrecacheImages();
	void ShowAchievement( int iAchievement );

private:
	float m_flStartTime;
	vgui2::ImagePanel *m_pIcon;
	vgui2::Label *m_pLabel;
	vgui2::Label *m_pLabelProgress;
	int32 m_iValue;
	int32 m_iMaxValue;
	bool m_bShowingAchievement;
	std::vector<int> m_Queue;
};

#endif // STEAM_ACHIEVEMENTS_H