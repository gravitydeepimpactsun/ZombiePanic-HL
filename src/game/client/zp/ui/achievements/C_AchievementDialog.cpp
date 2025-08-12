//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include "C_AchievementDialog.h"
#include "CAchievementRequirements.h"

#include "steam/steam_api.h"
#include "client_vgui.h"
#include "gameui/gameui_viewport.h"
#include "IBaseUI.h"
#include "zp/zp_achievements.h"

using namespace vgui2;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ===================================
// SETUP
// ===================================

enum
{
	CATEGORY_SHOWALL = 0,
	CATEGORY_GENERAL,
	CATEGORY_MAPS,
	CATEGORY_KILLS,

	MAX_CATEGORIES
};

// TODO: Add the actual steps once all the maps are done.
std::vector<RequiredStepsTable> m_MarathonSteps = {
	RequiredStepsTable( ZP_KILLS_CROWBAR, 1 ),
	RequiredStepsTable( ZP_KILLS_SHOTGUN )
};

// We only need to check for one single kill for each
std::vector<RequiredStepsTable> m_JackOfTradesSteps = {
	RequiredStepsTable( ZP_KILLS_CROWBAR, 1 ),
	RequiredStepsTable( ZP_KILLS_PISTOL, 1 ),
	RequiredStepsTable( ZP_KILLS_REVOLVER, 1 ),
	RequiredStepsTable( ZP_KILLS_RIFLE, 1 ),
	RequiredStepsTable( ZP_KILLS_MP5, 1 ),
	RequiredStepsTable( ZP_KILLS_SHOTGUN, 1 ),
	RequiredStepsTable( ZP_KILLS_SATCHEL, 1 ),
	RequiredStepsTable( ZP_KILLS_TNT, 1 )
};

DialogAchievementData g_DAchievements[] =
{
	_ACH_ID(KILLS_CROWBAR,					CATEGORY_KILLS,			ZP_KILLS_CROWBAR),
	_ACH_ID(KILLS_PISTOL,					CATEGORY_KILLS,			ZP_KILLS_PISTOL),
	_ACH_ID(KILLS_REVOLVER,					CATEGORY_KILLS,			ZP_KILLS_REVOLVER),
	_ACH_ID(KILLS_RIFLE,					CATEGORY_KILLS,			ZP_KILLS_RIFLE),
	_ACH_ID(KILLS_MP5,						CATEGORY_KILLS,			ZP_KILLS_MP5),
	_ACH_ID(KILLS_SHOTGUN,					CATEGORY_KILLS,			ZP_KILLS_SHOTGUN),
	_ACH_ID(KILLS_SATCHEL,					CATEGORY_KILLS,			ZP_KILLS_SATCHEL),
	_ACH_ID(KILLS_TNT,						CATEGORY_KILLS,			ZP_KILLS_TNT),
	_ACH_ID(KILLS_ZOMBIE,					CATEGORY_KILLS,			ZP_KILLS_ZOMBIE),
	_ACH_ID(YOU_WILL_DIE_WITH_ME,			CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ID(UNSAFE_HANDLING,				CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ID_LIST(JACKOFTRADES,				CATEGORY_KILLS,			INVALID_STAT, m_JackOfTradesSteps),
	_ACH_ID(PANICRUSH,						CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ID(FLEEESH,						CATEGORY_KILLS,			ZP_FLEEESH),
	_ACH_ID(ZOMBIEDESSERT,					CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ID(SCREAM4ME,						CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ID(INLINEP2,						CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ID(CUTYOUDOWN,						CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ID(RABBITBEAST,					CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ID(ITS_A_MASSACRE,					CATEGORY_KILLS,			ZP_ITS_A_MASSACRE),

	_ACH_ID(FIRST_SURVIVAL,			CATEGORY_MAPS, INVALID_STAT),
	_ACH_ID(FIRST_OBJECTIVE,		CATEGORY_MAPS, INVALID_STAT),
	_ACH_ID(PARTOFHORDE,			CATEGORY_MAPS, INVALID_STAT),
	_ACH_ID(CLOCKOUT,				CATEGORY_MAPS, INVALID_STAT),
	_ACH_ID(THE_ATEAM,				CATEGORY_MAPS, INVALID_STAT),
	_ACH_ID(PARTNERINCRIME,			CATEGORY_MAPS, INVALID_STAT),
	_ACH_ID(LASTMANSTAND,			CATEGORY_MAPS, INVALID_STAT),

	_ACH_ID_LIST(MARATHON,			CATEGORY_MAPS, INVALID_STAT, m_MarathonSteps ),
	_ACH_ID(PLAY_ALL_SURVIVAL,		CATEGORY_MAPS, INVALID_STAT),
	_ACH_ID(PLAY_ALL_OBJECTIVE,		CATEGORY_MAPS, INVALID_STAT),

	_ACH_ID(PANIC_ATTACK,			CATEGORY_GENERAL, INVALID_STAT),
	_ACH_ID(PANIC_100,				CATEGORY_GENERAL, ZP_PANIC_100),
	_ACH_ID(I_FELL,					CATEGORY_GENERAL, INVALID_STAT),
	_ACH_ID(ONE_OF_US,				CATEGORY_GENERAL, INVALID_STAT),
	_ACH_ID(FIRST_TO_DIE,			CATEGORY_GENERAL, INVALID_STAT),
	_ACH_ID(TABLE_FLIP,				CATEGORY_GENERAL, INVALID_STAT),
	_ACH_ID(ZMASH,					CATEGORY_GENERAL, INVALID_STAT),
	_ACH_ID(DIE_BY_DOOR,			CATEGORY_GENERAL, INVALID_STAT),
	_ACH_ID(PUMPUPSHOTGUN,			CATEGORY_GENERAL, ZP_PUMPUPSHOTGUN),
	_ACH_ID(CHILDOFGRAVE,			CATEGORY_GENERAL, ZP_CHILDOFGRAVE),
};

extern StatData_t GrabStat( EStats nID );
extern void SetStat( EStats nID, int32 value );

// ===================================
// Achievements
// ===================================

class CSteamAchievementsDialog
{
public:
	CSteamAchievementsDialog(DialogAchievementData *Achievements, int NumAchievements);

	DialogAchievementData *m_pAchievements;		// Achievements data
	int m_iNumAchievements;				// The number of Achievements
	bool RequestStats();
	void GetStat( UserStatsReceived_t *pCallback, EStats id );

	STEAM_CALLBACK(CSteamAchievementsDialog, OnUserStatsReceived, UserStatsReceived_t,
		m_CallbackUserStatsReceived);
};

CSteamAchievementsDialog::CSteamAchievementsDialog(DialogAchievementData *Achievements, int NumAchievements) :
	m_CallbackUserStatsReceived(this, &CSteamAchievementsDialog::OnUserStatsReceived)
{
	m_pAchievements = Achievements;
	m_iNumAchievements = NumAchievements;
	RequestStats();
}

bool CSteamAchievementsDialog::RequestStats()
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == GetSteamAPI()->SteamUserStats() || NULL == GetSteamAPI()->SteamUser())
		return false;

	// Is the user logged on?  If not we can't get stats.
	if (!GetSteamAPI()->SteamUser()->BLoggedOn())
		return false;

	// Request user stats.
	return GetSteamAPI()->SteamUserStats()->RequestCurrentStats();
}

void CSteamAchievementsDialog::OnUserStatsReceived(UserStatsReceived_t *pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (GetSteamAPI()->SteamUtils()->GetAppID() == pCallback->m_nGameID)
	{
		if (k_EResultOK != pCallback->m_eResult) return;
		// Go through our stats
		for ( int i = 0; i < STAT_MAX; i++ )
			GetStat( pCallback, (EStats)i );
	}
}

void CSteamAchievementsDialog::GetStat( UserStatsReceived_t *pCallback, EStats id )
{
	StatData_t stat = GrabStat( id );
	int32 iData = 0;
	GetSteamAPI()->SteamUserStats()->GetUserStat( pCallback->m_steamIDUser, stat.Name, &iData );
	SetStat( id, iData );
}

CSteamAchievementsDialog*	g_DSteamAchievements = NULL;

C_AchievementDialog::C_AchievementDialog(vgui2::Panel *pParent)
    : BaseClass(pParent, "AchievementDialog")
{
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	MoveToCenterOfScreen();

	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(true);
	SetSizeable(false);
	SetMoveable(true);
	SetVisible(true);
	SetDeleteSelfOnClose(true);

	bool bRet = GetSteamAPI()->Init();
	if ( bRet && !g_DSteamAchievements )
		g_DSteamAchievements = new CSteamAchievementsDialog(g_DAchievements, ACHV_MAX);

	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));

	LoadControlSettings(VGUI2_ROOT_DIR "resource/zps/zp_achievementsdialog.res");

	vgui2::ivgui()->AddTickSignal( GetVPanel(), 25 );

	iAchievement = 0;
	CurrentCategory = -1;

	HideAchieved = false;
	miProgressBar = 0;
	miTotalAchievements = 0;
	miCompletedAchievements = 0;

	// KeyValues
	KeyValues *kv = new KeyValues("Achievement_Categories", "Key", "Value");

	// Fonts
	vgui2::HFont	hTextFont;
	vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));

	// Set Achievements categories
	ui_AchvList = new vgui2::ComboBox(this, "achievement_pack_combo", MAX_CATEGORIES, false);
	ui_AchvList->SetPos(25, 410);
	ui_AchvList->SetSize(235, 24);
	ui_AchvList->AddItem( "#ZP_UI_Achievements_Show_All_Achievements", kv);
	ui_AchvList->AddItem( "#ZP_UI_Achievements_General", kv);
	ui_AchvList->AddItem( "#ZP_UI_Achievements_Map", kv);
	ui_AchvList->AddItem( "#ZP_UI_Achievements_Kills", kv);
	// Auto select "Show All Achievements"
	ui_AchvList->ActivateItem(0);

	// Should we hide achieved ones?
	ui_AchvTaken = new vgui2::CheckButton(this, "HideAchieved", "#ZP_UI_Achievement_Hide_Achieved" );
	ui_AchvTaken->SetPos(260, 410);
	ui_AchvTaken->SetSize(150, 24);
	ui_AchvTaken->SetCommand("hide_achieved");
	ui_AchvTaken->SetSelected(HideAchieved);

	// Our achievements listing
	ui_AchvPList = new AchievementList(this, "listpanel_achievements");
	ui_AchvPList->SetPos(15, 100);
	ui_AchvPList->SetSize(600, 302);

	// Setup achievement progress
	ui_CurrentCompleted = new vgui2::Label(this, "PercentageText", "0%");
	hTextFont = pScheme->GetFont("AchievementItemDescription");
	if ( hTextFont != vgui2::INVALID_FONT )
		ui_CurrentCompleted->SetFont(hTextFont);
	ui_CurrentCompleted->SetPos(407, 46);
	ui_CurrentCompleted->SetSize(200, 20);
	ui_CurrentCompleted->SetContentAlignment( vgui2::Label::a_east );

	ui_TotalProgress = new vgui2::ImagePanel(this, "PercentageBar");
	ui_TotalProgress->SetPos(23, 67);
	ui_TotalProgress->SetSize(0, 16);
	ui_TotalProgress->SetFillColor(Color(142, 20, 48, 255));
}

void C_AchievementDialog::OnTick()
{
	BaseClass::OnTick();

	// Setup our completed achievements
	char buffer[40];

	miTotalAchievements = 0;
	miCompletedAchievements = 0;

	for (int iAch = 0; iAch < g_DSteamAchievements->m_iNumAchievements; ++iAch)
	{
		DialogAchievementData &ach = g_DSteamAchievements->m_pAchievements[iAch];

		// Total achievements
		miTotalAchievements++;

		// Completed achievements
		if ( ach.IsAchieved() )
			miCompletedAchievements++;
	}

	// Get propper ratio of the bar
	float ratio = miCompletedAchievements / (float)miTotalAchievements;
	int   realpos = ratio * 584;

	// Make our position bigger!
	for (int i_pos = 0; i_pos < realpos; i_pos++)
		miProgressBar = i_pos;

	if (miProgressBar < 584)
		miProgressBar = miProgressBar;
	else
		miProgressBar = 584;

	Q_snprintf(buffer, sizeof(buffer), "%d%%", (int)(ratio * 100));
	ui_CurrentCompleted->SetText(buffer);
	ui_TotalProgress->SetSize(miProgressBar, 16);

	LoadAchievements();

	if ( ui_AchvList->GetActiveItem() != CurrentCategory )
	{
		CurrentCategory = ui_AchvList->GetActiveItem();
		iAchievement = 0;
	}
}

void C_AchievementDialog::LoadAchievements()
{
ReadAchievement:
	if ( iAchievement >= g_DSteamAchievements->m_iNumAchievements )
		return;
	
	// Fonts
	vgui2::HFont hTextFont;
	vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));

	// Reset the value, if our iAchievement is 0
	if ( iAchievement == 0 )
		ui_AchvPList->DeleteAllItems();

	DialogAchievementData &ach = g_DSteamAchievements->m_pAchievements[ iAchievement ];

	// Increase
	iAchievement++;

	// Grab achievement
	bool bIsAchieved = ach.IsAchieved();
	GetSteamAPI()->SteamUserStats()->GetAchievement( ach.GetAchievementName(), &bIsAchieved );
	ach.SetAchieved( bIsAchieved );

	// Wrong category!
	// If its not on (Show All)
	if ( CATEGORY_SHOWALL != ui_AchvList->GetActiveItem() )
	{
		if ( ach.GetCategoryID() != ui_AchvList->GetActiveItem() )
			goto ReadAchievement;
	}

	// We won't show hidden achievements
	// Unless we have achieved em
	if ( !Q_stricmp( GetSteamAPI()->SteamUserStats()->GetAchievementDisplayAttribute( ach.GetAchievementName(), "hidden" ), "1")
		&& !ach.IsAchieved() )
		goto ReadAchievement;

	// hide achievements we already got
	if ( HideAchieved && HideAchieved == ach.IsAchieved() )
		goto ReadAchievement;

	// Create Image
	vgui2::ImagePanel *imagePanel = new vgui2::ImagePanel(this, "AchievementIcon");

	char buffer[158];
	Q_snprintf( buffer, sizeof( buffer ), "ui/achievements/%s%s", ach.GetAchievementName(), !ach.IsAchieved() ? "_L" : "" );

	imagePanel->SetImage( vgui2::scheme()->GetImage(buffer, false) );
	imagePanel->SetSize(56, 56);
	imagePanel->SetPos(4, 4);

	// Font Text
	Q_snprintf( buffer, sizeof( buffer ), "#ZP_ACH_%s_NAME", ach.GetAchievementName() );
	vgui2::Label *label_title = new vgui2::Label(this, "AchievementTitle", buffer );
	label_title->SetSize(400, 20);
	label_title->SetPos(70, 5);
	label_title->SetPaintBackgroundEnabled(false);
	hTextFont = pScheme->GetFont("AchievementItemTitle");
	if ( hTextFont != vgui2::INVALID_FONT )
		label_title->SetFont(hTextFont);

	Q_snprintf( buffer, sizeof( buffer ), "#ZP_ACH_%s_DESC", ach.GetAchievementName() );
	vgui2::Label *label_desc = new vgui2::Label(this, "AchievementDescription", buffer );
	label_desc->SetSize(490, 40);
	label_desc->SetPos(71, 22);
	label_desc->SetPaintBackgroundEnabled(false);
	hTextFont = pScheme->GetFont("AchievementItemDescription");
	if ( hTextFont != vgui2::INVALID_FONT )
		label_desc->SetFont(hTextFont);

	vgui2::Label *label_achievement_progress_num = NULL;
	vgui2::ImagePanel *label_achievement_progress_bg = NULL;
	vgui2::ImagePanel *label_achievement_progress = NULL;
	vgui2::CAchievementRequirementsHolder *required_steps = NULL;

	int iValue = 0;
	int imValue = 0;

	StatData_t steamstats = ach.GetData();
	int32 nStatValue = steamstats.Value;
	int32 nStatMaxValue = steamstats.MaxValue;

	// Has required steps?
	// If we have steps, it will override the progress bar.
	if ( ach.HasRequiredSteps() )
	{
		nStatValue = 0;
		required_steps = new vgui2::CAchievementRequirementsHolder( this, "RequiredSteps" );
		for ( size_t i = 0; i < ach.GetRequiredStepCount(); i++ )
		{
			RequiredStepsTable SteamStatID = ach.GetRequiredStepID( i );
			StatData_t itemStat = GrabStat( SteamStatID.Stat );
			int32 nItemMax = itemStat.MaxValue;
			if ( SteamStatID.MaxValue > 0 )
				nItemMax = SteamStatID.MaxValue;
			bool bObtained = ( itemStat.Value >= nItemMax ) ? true : false;
			Q_snprintf( buffer, sizeof( buffer ), "#ZP_STAT_%s", itemStat.Name );
			required_steps->AddItem( bObtained, buffer );
			if ( bObtained ) nStatValue++;
		}
		nStatMaxValue = ach.GetRequiredStepCount();
	}

	if ( ( ach.HasStatID() || ach.HasRequiredSteps() )
	    && nStatValue < nStatMaxValue
		&& !ach.IsAchieved() )
	{
		Q_snprintf( buffer, sizeof(buffer), "%d / %d", nStatValue, nStatMaxValue );
		label_achievement_progress_num = new vgui2::Label(this, "AchievementProgress", buffer);
		if ( hTextFont != vgui2::INVALID_FONT )
			label_achievement_progress_num->SetFont(hTextFont);
		label_achievement_progress_num->SetPaintBackgroundEnabled(false);

		label_achievement_progress_bg = new vgui2::ImagePanel(this, "AchievementProgressBarBG");
		label_achievement_progress_bg->SetSize(475, 12);
		label_achievement_progress_bg->SetFillColor(Color(32, 32, 32, 255));

		label_achievement_progress = new vgui2::ImagePanel(this, "AchievementProgressBar");
		label_achievement_progress->SetSize(475, 12);
		label_achievement_progress->SetFillColor(Color(142, 20, 48, 255));

		// Achievement progress
		iValue = nStatValue;
		imValue = nStatMaxValue;
	}
	else
	{
		iValue = 0;
		imValue = 0;
	}

	// Setup the obtained achievement bg texture (only shows if achieved)
	vgui2::ImagePanel *AchievedBG = new vgui2::ImagePanel(this, "AchievementIcon");
	if ( ach.IsAchieved() )
		AchievedBG->SetImage( vgui2::scheme()->GetImage("ui/gfx/ach_obtained", false) );
	AchievedBG->SetSize(50, 56);
	AchievedBG->SetPos(4, 4);

	// Add Label and Image to PanelListPanel
	ui_AchvPList->AddItem(
		imagePanel, label_title,
		label_desc, label_achievement_progress_num,
		label_achievement_progress, label_achievement_progress_bg,
		iValue, imValue, AchievedBG, required_steps );
}


CON_COMMAND(gameui_achievements, "Opens Achievements dialog")
{
	// Since this command is called from game menu using "engine gameui_achievements"
	// GameUI will hide itself and show the game.
	// We need to show it again and after that activate C_AchievementDialog
	// Otherwise it may be hidden by the dev console
	gHUD.CallOnNextFrame([]() {
		CGameUIViewport::Get()->GetAchievementDialog()->Activate();
	});
	g_pBaseUI->ActivateGameUI();
}

void C_AchievementDialog::OnCommand(const char* pcCommand)
{
	if ( !Q_stricmp( pcCommand, "hide_achieved" ) )
	{
		iAchievement = 0;
		if ( HideAchieved ) HideAchieved = false;
		else HideAchieved = true;
	}
	else
	{
		BaseClass::OnCommand(pcCommand);
	}
}

DialogAchievementData GetAchievementByID( int eAchievement )
{
	for ( int i = 0; i < ARRAYSIZE( g_DAchievements ); i++ )
	{
		DialogAchievementData item = g_DAchievements[ i ];
		if ( item.GetAchievementID() == eAchievement )
			return item;
	}
	return g_DAchievements[0];
}

DialogAchievementData::DialogAchievementData( EAchievements nID, const char *szName, int nCategory, EStats nStatID )
{
	Q_snprintf( m_pchAchievementID, sizeof(m_pchAchievementID), "%s", szName );
	m_bAchieved = false;
	m_iIconImage = 0;
	m_eCategory = nCategory;
	m_eAchievementID = nID;
	m_nStat = nStatID;
}

DialogAchievementData::DialogAchievementData( EAchievements nID, const char *szName, int nCategory, EStats nStatID, std::vector<RequiredStepsTable> nRequiredSteps )
{
	Q_snprintf( m_pchAchievementID, sizeof(m_pchAchievementID), "%s", szName );
	m_bAchieved = false;
	m_iIconImage = 0;
	m_eCategory = nCategory;
	m_eAchievementID = nID;
	m_nStat = nStatID;
	m_RequiredSteps = nRequiredSteps;
}

StatData_t DialogAchievementData::GetData()
{
	return GrabStat( m_nStat );
}
