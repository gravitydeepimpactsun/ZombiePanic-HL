// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "cl_dll.h"
#include "event_api.h"
#include "cl_voice_status.h"
#include "zp_roundstate.h"
#include "vgui/client_viewport.h"

DEFINE_HUD_ELEM(CHudRoundState);

extern void Input_StopAllMovements( bool bForce );

ConVar cl_zpssound("cl_zpssound", "0", FCVAR_BHL_ARCHIVE, "Play ZPS themed sounds");

CHudRoundState::CHudRoundState()
    : vgui2::Panel(NULL, "HudRoundState")
{
	SetParent( g_pViewport );

	SetScheme( "ClientScheme" );

	m_pText = new vgui2::Label( this, "Text", "Apple!" );
	m_pText->SetVisible( false );
	m_pText->SetContentAlignment( vgui2::Label::Alignment::a_center );
	gHUD.m_RoundState = ZP::RoundState::RoundState_Invalid;

	HookMessage<&CHudRoundState::MsgFunc_RoundState>("RoundState");
}

void CHudRoundState::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor( Color(0, 0, 0, 0) );
}

bool CHudRoundState::IsAllowedToDraw()
{
	if ( gHUD.m_RoundState == ZP::RoundState::RoundState_Invalid ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_TEAM ) ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_MOTD ) ) return false;
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer->IsConnected() ) return false;
	return hud_draw.GetBool();
}

void CHudRoundState::Paint()
{
	if ( !IsAllowedToDraw() )
	{
		if ( m_pText->IsVisible() )
			m_pText->SetVisible( false );
		return;
	}

	int x, y, w, h;
	GetBounds( x, y, w, h );

	switch ( gHUD.m_RoundState )
	{
		case ZP::RoundState::RoundState_WaitingForPlayers:
		{
			vgui2::surface()->DrawSetColor( Color( 0, 0, 0, 150 ) );
			vgui2::surface()->DrawFilledRect( 0, m_iWaitingForPlayersYPos, w, m_iWaitingForPlayersYPos + m_iWaitingForPlayersTall );
			m_pText->SetBounds( 0, m_iWaitingForPlayersYPos, w, m_iTextWaitingForPlayersTall );
			if ( !m_pText->IsVisible() )
				m_pText->SetVisible( true );
		}
		break;

		case ZP::RoundState::RoundState_RoundIsOver:
		{
		    int iAlpha = m_pText->GetFgColor().a();
		    iAlpha += 2;
			m_pText->SetFgColor( Color( 255, 255, 255, (int)clamp( iAlpha, 0, 255 ) ) );
			m_pText->SetBounds( 0, m_iRoundIsOverYPos, w, m_iRoundIsOverTall );
			if ( !m_pText->IsVisible() )
				m_pText->SetVisible( true );
			// Make sure every movement input is stopped
			// when the round is over every frame to prevent
			// the player from moving around.
			Input_StopAllMovements( true );
		}
		break;
	}
}

int CHudRoundState::MsgFunc_RoundState(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	int state = READ_SHORT();
	int winner = READ_SHORT();
	SetRoundState( (ZP::RoundState)state, winner );

	return 1;
}

void CHudRoundState::SetRoundState( ZP::RoundState iState, int iWinner )
{
	gHUD.m_RoundState = iState;

	switch ( gHUD.m_RoundState )
	{
		case ZP::RoundState_WaitingForPlayers:
		{
			vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme( GetScheme() );
			if ( pScheme )
			{
				vgui2::HFont font = pScheme->GetFont( m_szWaitingForPlayersFont, IsProportional() );
			    if ( font != vgui2::INVALID_FONT )
					m_pText->SetFont( font );
			}
			m_pText->SetFgColor( Color( 255, 255, 255, 255 ) );
			m_pText->SetText( "#ZP_WaitingForPlayers" );
			m_pText->SetVisible( true );
			Input_StopAllMovements( true );
		}
		break;
		// Use these or???
		case ZP::RoundState_RoundIsStarting:
		{
			m_pText->SetVisible( false );
			PlayAudio( "modes/round_ready.wav" );
			Input_StopAllMovements( true );
		}
	    break;
		//case ZP::RoundState_PickVolunteers: break;
		//case ZP::RoundState_RoundHasBegunPost: break;
		//case ZP::RoundState_RoundHasBegun: break;
		case ZP::RoundState_RoundIsOver:
		{
			// winner is only used here under RoundIsOver state.
			vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme( GetScheme() );
			if ( pScheme )
			{
				vgui2::HFont font = pScheme->GetFont( m_szRoundIsOverFont, IsProportional() );
				if ( font != vgui2::INVALID_FONT )
					m_pText->SetFont( font );
		    }
			// Show our win text & play the audio.
			// ---------------------------------------
		    // Zombie Panic! never had any "round win" audio,
		    // so we will use a modified one from Zombie Panic! Source instead.
		    const char *szSoundToPlay = nullptr;
			switch ( iWinner )
			{
				case 1:
				{
					m_pText->SetText( "#ZP_Win_Draw" );
			        szSoundToPlay = "modes/win_draw.wav";
				}
				break;
				case 2:
				{
					m_pText->SetText( "#ZP_Win_Zombie" );
			        szSoundToPlay = "modes/win_zombie.wav";
				}
				break;
				case 3:
				{
					m_pText->SetText( "#ZP_Win_Survivor" );
			        szSoundToPlay = "modes/win_human.wav";
				}
				break;
			}
			PlayAudio( szSoundToPlay );
			m_pText->SetFgColor( Color( 255, 255, 255, 0 ) );
			m_pText->SetVisible( true );
			Input_StopAllMovements( true );
		}
	    break;
	    default: m_pText->SetVisible( false ); break;
	}
}

void CHudRoundState::PlayAudio( const char *szAudio )
{
	if ( !cl_zpssound.GetBool() ) return;
	if ( szAudio && szAudio[0] )
		gEngfuncs.pEventAPI->EV_PlaySound(
			gEngfuncs.GetLocalPlayer()->index,
			gEngfuncs.GetLocalPlayer()->origin,
		    CHAN_BOT, szAudio,
			1, ATTN_NORM,
			0, PITCH_NORM
		);
}
