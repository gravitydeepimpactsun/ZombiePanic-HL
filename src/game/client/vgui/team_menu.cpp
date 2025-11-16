#include <vgui/ILocalize.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/MessageBox.h>
#include "gameui/gameui_viewport.h"
#include "client_vgui.h"
#include "client_viewport.h"
#include "team_menu.h"
#include "viewport_panel_names.h"
#include "hud.h"
#include "cl_util.h"
#include "event_api.h"
#include "zp/zp_shared.h"

extern void Input_StopAllMovements();

CTeamMenu::CTeamMenu()
    : BaseClass(nullptr, VIEWPORT_PANEL_TEAM_MENU)
{
	char buf[64];

	SetTitle("Zombie Panic!", true);
	SetMoveable(false);
	SetProportional(true);
	SetSizeable(false);
	SetCloseButtonVisible(false);

	m_pVolunteerForZombie = new vgui2::CheckButton(this, "VolunteerForZombie", "#ZP_TeamMenu_VolunteerForZombie");
	m_pJoinGameButton = new vgui2::Button(this, "JoinGameButton", "#ZP_TeamMenu_JoinGame", this, "JoinGame");
	m_pSpectateButton = new vgui2::Button(this, "SpectateButton", "#ZP_TeamMenu_Spectate", this, "Spectate");
	m_pBriefingText = new vgui2::RichText(this, "BriefingText");
	m_pGameModeInfo = new vgui2::RichText(this, "GameModeInfo");
	m_pLabelCurrentMapValue = new vgui2::Label(this, "LabelCurrentMapValue", "");
	m_pLabelGameModeName = new vgui2::Label(this, "LabelGameModeName", "");
	m_pLabelRounds = new vgui2::Label(this, "LabelRounds", "Rounds: 1");

	ReloadLayout();
	SetVisible(false);
}

CTeamMenu::~CTeamMenu()
{
}

void CTeamMenu::OnCommand(const char *pCommand)
{
	if (!V_stricmp(pCommand, "JoinGame") || !V_stricmp(pCommand, "Close"))
	{
		if (m_pVolunteerForZombie->IsSelected())
			gEngfuncs.pfnServerCmd("joingame volunteer");
		else
			gEngfuncs.pfnServerCmd("joingame");
		ShowPanel(false);
	}
	else if (!V_stricmp(pCommand, "Spectate"))
	{
#if 0
		if ( gHUD.m_RoundState >= ZP::RoundState_RoundHasBegun )
		{
			gEngfuncs.pfnServerCmd("spectate");
			ShowPanel(false);
		}
		else
		{
			vgui2::MessageBox *pBox = new vgui2::MessageBox( "#ZP_GameTitle", "#ZP_MessageBox_Error_EarlySpec", this );
			pBox->ShowWindow( this );
		}
#else
		// Allow spectating at any time, since we now
		// allow spectators to open up this menu if they press +duck button.
		gEngfuncs.pfnServerCmd( "spectate" );
		ShowPanel( false );
#endif
	}
}

void CTeamMenu::OnKeyCodeTyped(vgui2::KeyCode code)
{
	if ( code == vgui2::KEY_ENTER )
		m_pJoinGameButton->FireActionSignal();
	else
		BaseClass::OnKeyCodeTyped( code );
}

void CTeamMenu::Update()
{
	char buf[128];
	// Set the Map Title
	if (!m_bUpdatedMapName)
	{
		const char *level = gEngfuncs.pfnGetLevelName();
		if (level && level[0])
		{
			char sz[256];
			char szTitle[256];
			char *ch;

			// Update the map briefing
			V_strcpy_safe(sz, level);
			ch = strchr(sz, '.');
			*ch = '\0';
			strcat(sz, ".txt");
			char *pfile = (char *)gEngfuncs.COM_LoadFile(sz, 5, NULL);
			if (pfile)
			{
				static char buf[4096];
				static wchar_t wbuf[4096];

				// Replace \r\n with \n
				V_StrSubst(pfile, "\r\n", "\n", buf, sizeof(buf));

				// Convert to Unicode to fix 1024 char limit
				g_pVGuiLocalize->ConvertANSIToUnicode(buf, wbuf, sizeof(wbuf));

				m_pBriefingText->SetText(wbuf);
				gEngfuncs.COM_FreeFile(pfile);
			}
			else
			{
				m_pBriefingText->SetText("#ZP_TeamMenu_NoMapInfo");
			}

			char levelname[64];
			V_FileBase( level, levelname, sizeof(levelname) );
			m_pLabelCurrentMapValue->SetText( levelname );

			m_bUpdatedMapName = true;
		}
	}
	if (!m_bUpdatedGameModeInfo)
	{
		const char *level = gEngfuncs.pfnGetLevelName();
		ZP::GameModeType_e type = ZP::IsValidGameModeMap( level );
		if ( type > ZP::GAMEMODE_INVALID )
		{
			switch ( type )
			{
				case ZP::GAMEMODE_NONE:
				{

					m_pGameModeInfo->SetText( "#ZP_GAMEMODE_DEV_INFO" );
				    m_pLabelGameModeName->SetText( "#ZP_GAMEMODE_DEV" );
				}
				break;
				case ZP::GAMEMODE_SURVIVAL:
				{

					m_pGameModeInfo->SetText( "#ZP_GAMEMODE_SURVIVAL_INFO" );
				    m_pLabelGameModeName->SetText( "#ZP_GAMEMODE_SURVIVAL" );
				}
				break;
				case ZP::GAMEMODE_OBJECTIVE:
				{

					m_pGameModeInfo->SetText( "#ZP_GAMEMODE_OBJECTIVE_INFO" );
				    m_pLabelGameModeName->SetText( "#ZP_GAMEMODE_OBJECTIVE" );
				}
				break;
				case ZP::GAMEMODE_HARDCORE:
				{

					m_pGameModeInfo->SetText( "#ZP_GAMEMODE_HARDCORE_INFO" );
				    m_pLabelGameModeName->SetText( "#ZP_GAMEMODE_HARDCORE" );
				}
				break;
			}
			m_bUpdatedGameModeInfo = true;
		}
	}
}

void CTeamMenu::Activate()
{
	Update();
	ShowPanel(true);
	// Stop player movement, so let's tell our input system about it.
	Input_StopAllMovements();
}

void CTeamMenu::Paint()
{
	BaseClass::Paint();

	if ( m_bHasVolunteered != m_pVolunteerForZombie->IsSelected() )
	{
		m_bHasVolunteered = m_pVolunteerForZombie->IsSelected();
		gEngfuncs.pEventAPI->EV_PlaySound(
			gEngfuncs.GetLocalPlayer()->index,
			gEngfuncs.GetLocalPlayer()->origin,
		    CHAN_BOT, "player/zombieclick.wav",
			1, ATTN_NORM,
			0, PITCH_NORM
		);
	}
}

const char *CTeamMenu::GetName()
{
	return VIEWPORT_PANEL_TEAM_MENU;
}

void CTeamMenu::Reset()
{
	m_bUpdatedMapName = false;
	m_bUpdatedGameModeInfo = false;
	m_iCurrentRounds = -1;
}

void CTeamMenu::ShowPanel(bool state)
{
	if (state != IsVisible())
	{
		SetVisible(state);
	}
}

vgui2::VPANEL CTeamMenu::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CTeamMenu::IsVisible()
{
	return BaseClass::IsVisible();
}

void CTeamMenu::SetParent(vgui2::VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CTeamMenu::ReloadLayout()
{
	LoadControlSettings(VGUI2_ROOT_DIR "resource/TeamMenu.res");
	SetBgColor( Color(0, 0, 0, 150) );
}

void CTeamMenu::SetCurrentRound()
{
	m_iCurrentRounds = gHUD.m_MapRounds;
	char buf[64];
	sprintf( buf, "Round: %i", gHUD.m_MapRounds );
	m_pLabelRounds->SetText( buf );
}
