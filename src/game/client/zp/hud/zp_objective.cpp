// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "cl_voice_status.h"
#include "zp_objective.h"
#include "vgui/client_viewport.h"

DEFINE_HUD_ELEM( CObjectiveText );

bool CObjectiveText::m_bObjectiveChanged;
ObjectiveState CObjectiveText::m_State;
std::string CObjectiveText::m_strObjectiveString[2];
std::string CObjectiveText::m_strObjectiveString_Changed[2];

CObjectiveText::CObjectiveText()
    : vgui2::Panel(NULL, "HudObjectiveText")
{
	SetParent(g_pViewport);

	SetProportional( true );

	m_pText = new vgui2::Label( this, "Text", "" );

	m_State = ObjectiveState::State_Normal;

	HookMessage<&CObjectiveText::MsgFunc_ObjMsg>("ObjMsg");
}

void CObjectiveText::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pText->SetFont( pScheme->GetFont( "ObjectiveText" ) );
	m_pText->SetContentAlignment( vgui2::Label::a_northwest );
	m_pText->SetFgColor( Color( 255, 255, 255, 255 ) );

	SetBgColor( Color(0, 0, 0, 0) );
}

void CObjectiveText::DrawText( bool bChanged )
{
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer ) return;
	if ( !localplayer->IsConnected() ) return;
	bool bIsZombo = ( localplayer->GetTeamNumber() == ZP::TEAM_ZOMBIE );
	if ( bChanged )
		m_pText->SetText( m_strObjectiveString_Changed[ bIsZombo ? 1 : 0 ].c_str() );
	else
		m_pText->SetText( m_strObjectiveString[ bIsZombo ? 1 : 0 ].c_str() );
}

bool CObjectiveText::IsAllowedToDraw()
{
	if ( gHUD.m_RoundState < ZP::RoundState::RoundState_RoundHasBegunPost ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_TEAM ) ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_MOTD ) ) return false;
	if ( gHUD.m_GameMode != ZP::GAMEMODE_OBJECTIVE ) return false;
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer->IsConnected() ) return false;
	return hud_draw.GetBool();
}

void CObjectiveText::Paint()
{
	if ( !IsAllowedToDraw() )
	{
		if ( m_pText->IsVisible() )
			m_pText->SetVisible( false );
		return;
	}

	// Make sure we're visible
	if ( !m_pText->IsVisible() )
		m_pText->SetVisible( true );

	// Now draw our lives
	m_pText->SetBounds( m_iTextX, m_iTextY, m_iTextWide, m_iTextTall );

	int clrFlash = abs( sin( gHUD.m_flTime * 2 ) * 255 );
	int clrR,
		clrG,
		clrB,
		clrA;

	// Default to white
	clrR = clrG = clrB = 255;
	clrA = 200;

	switch( m_State )
	{
		case ObjectiveState::State_Normal:
			clrA = 255;
		break;

		case ObjectiveState::State_InProgress:
			clrB = clrFlash;
			clrA = 255;
		break;

		case ObjectiveState::State_Failed:
			clrB = clrG = clrFlash;
			clrA = 255;
		break;
	}

	// Override the values, if this is set
	if ( m_bObjectiveChanged || m_flChangedRecently > 0 )
	{
		if ( m_State == ObjectiveState::State_Completed )
			m_pText->SetFgColor( Color( 0, 255, 0, 255 ) );
		else
			m_pText->SetFgColor( Color( 255, 0, 0, 255 ) );

		if ( m_bObjectiveChanged )
		{
			// How long should we be on screen?
			m_flChangedRecently = 2.0f;
			// Set it back to false
			m_bObjectiveChanged = false;
		}
		else
			m_flChangedRecently -= 0.025f;

		// Set the new text
		DrawText( true );
	}
	else
	{
		// Set the new color
		m_pText->SetFgColor( Color( clrR, clrG, clrB, clrA ) );

		// Set the new text
		DrawText( false );
	}
}

int CObjectiveText::MsgFunc_ObjMsg(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	int obj_state = READ_SHORT();
	static char szText[512];
	strncpy( szText, READ_STRING(), 512 );
	static char szTextZombo[512];
	strncpy( szTextZombo, READ_STRING(), 512 );

	// Mark if we just completed or failed an objective
	if ( obj_state == ObjectiveState::State_Completed || obj_state == ObjectiveState::State_Failed )
	{
		CObjectiveText::m_strObjectiveString_Changed[0] = szText;
		CObjectiveText::m_strObjectiveString[0] = szText;
		CObjectiveText::m_strObjectiveString_Changed[1] = szTextZombo;
		CObjectiveText::m_strObjectiveString[1] = szTextZombo;
		// Set our obj ID
		CObjectiveText::m_bObjectiveChanged = true;
	}
	else
	{
		CObjectiveText::m_strObjectiveString_Changed[0] = "";
		CObjectiveText::m_strObjectiveString_Changed[1] = "";
		CObjectiveText::m_strObjectiveString[0] = szText;
		CObjectiveText::m_strObjectiveString[1] = szTextZombo;
	}

	m_pText->SetVisible( true );

	// Update our state
	CObjectiveText::m_State = (ObjectiveState)obj_state;

	return 1;
}
