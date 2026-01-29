// ============== Copyright (c) 2026 Monochrome Games ============== \\

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "zp_progressbar.h"
#include "vgui/client_viewport.h"

DEFINE_HUD_ELEM( CZPProgressBar );

CZPProgressBar::CZPProgressBar()
    : vgui2::Panel(NULL, "HudZPProgressBar")
{
	SetParent( g_pViewport );

	SetProportional( true );

	m_pBackground = new vgui2::ImagePanel( this, "Background" );
	m_pBackground->SetShouldScaleImage( true );
	m_pBackground->SetImage( "ui/gfx/hud/progress_bar_bg" );
	m_pProgressBar = new vgui2::ImagePanel( this, "ProgressBar" );
	m_pProgressBar->SetImage( "ui/gfx/hud/progress_bar" );
	m_pText = new vgui2::Label( this, "Text", "" );
	m_flProgressTime = 0.0f;
	m_flStartTime = 0.0f;
}

void CZPProgressBar::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pText->SetFont( pScheme->GetFont( "ObjectiveText" ) );
	m_pText->SetContentAlignment( vgui2::Label::a_northwest );
	m_pText->SetFgColor( Color( 255, 255, 255, 255 ) );

	SetBgColor( Color(0, 0, 0, 0) );

	m_pText->SetVisible( false );
	m_pBackground->SetVisible( false );
	m_pProgressBar->SetVisible( false );
}

void CZPProgressBar::DrawText( const char *szString, float flProgressTime )
{
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer ) return;
	if ( !localplayer->IsConnected() ) return;
	bool bIsZombo = ( localplayer->GetTeamNumber() == ZP::TEAM_ZOMBIE );
	m_pText->SetColorCodedText( szString );
	if ( flProgressTime <= 0 )
		m_flProgressTime = 0;
	else
		m_flProgressTime = gHUD.m_flTime + flProgressTime;
	m_flStartTime = flProgressTime;
}

bool CZPProgressBar::IsAllowedToDraw()
{
	if ( gHUD.m_RoundState < ZP::RoundState::RoundState_RoundHasBegunPost ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_TEAM ) ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_MOTD ) ) return false;
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer->IsConnected() ) return false;
	if ( m_flProgressTime == 0 ) return false;
	if ( m_flProgressTime > 0 && gHUD.m_flTime > m_flProgressTime ) return false;
	return hud_draw.GetBool();
}

void CZPProgressBar::Paint()
{
	if ( !IsAllowedToDraw() )
	{
		m_pText->SetVisible( false );
		m_pBackground->SetVisible( false );
		m_pProgressBar->SetVisible( false );
		m_flProgressTime = 0;
		return;
	}

	// Make sure we're visible
	m_pText->SetVisible( true );
	m_pBackground->SetVisible( true );
	m_pProgressBar->SetVisible( true );

	// Our bounds
	int x, y, w, h;
	GetBounds( x, y, w, h );

	// Grab the center
	int centerX = w / 2;
	int centerY = h / 2;

	// Now draw the text
	m_pText->SetBounds( centerX + m_iTextX, centerY + m_iTextY, m_iTextWide, m_iTextTall );

	int wide = m_iBarWide;
	int tall = m_iBarTall;

	// Background and progress bar use the same size.
	// But progress bar width is clamped to the progress time
	m_pBackground->SetBounds( centerX + m_iBarX, centerY + m_iBarY, wide, tall );
	// Our image size is always the full size.
	m_pProgressBar->SetImageSize( wide, tall );

	// Calculate progress bar width.
	int maxWide = wide;
	if ( m_flProgressTime > 0 )
	{
		float flTimeLeft = m_flProgressTime - gHUD.m_flTime;
		float flTotalTime = m_flStartTime;
		float flProgress = 1.0f - ( flTimeLeft / flTotalTime );
		wide = (int)(maxWide * flProgress);
	}
	else
		wide = 0;
	m_pProgressBar->SetBounds( centerX + m_iBarX, centerY + m_iBarY, wide, tall );
}
