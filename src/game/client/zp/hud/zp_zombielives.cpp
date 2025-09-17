// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "cl_voice_status.h"
#include "zp_zombielives.h"
#include "vgui/client_viewport.h"

DEFINE_HUD_ELEM(CHudZombieLives);

CHudZombieLives::CHudZombieLives()
    : vgui2::Panel(NULL, "HudZombieLives")
{
	SetParent(g_pViewport);

	m_clrIcon = Color(255, 255, 255, 254);

	m_pLives = new vgui2::Label( this, "Lives", "0" );

	HookMessage<&CHudZombieLives::MsgFunc_ZombieLives>("ZombieLives");
}

void CHudZombieLives::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	vgui2::HFont font = pScheme->GetFont( m_szLivesText, IsProportional() );
	if ( font != vgui2::INVALID_FONT )
		m_pLives->SetFont( font );

	SetBgColor( Color(0, 0, 0, 0) );
}

bool CHudZombieLives::IsAllowedToDraw()
{
	if ( gHUD.m_RoundState < ZP::RoundState::RoundState_RoundHasBegunPost ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_TEAM ) ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_MOTD ) ) return false;
	// Only draw in survival or hardcore modes
	switch ( gHUD.m_GameMode )
	{
		case ZP::GAMEMODE_SURVIVAL:
		case ZP::GAMEMODE_HARDCORE: break;
		default: return false;
	}
	if ( gEngfuncs.GetLocalPlayer()->index <= 0 ) return false;
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer->IsConnected() ) return false;
	return hud_draw.GetBool();
}

void CHudZombieLives::Paint()
{
	if ( !IsAllowedToDraw() )
	{
		if ( m_pLives->IsVisible() )
			m_pLives->SetVisible( false );
		return;
	}

	int x, y, w, h;
	GetBounds( x, y, w, h );

	// 0 - Wide
	// 1 - Tall
	int imagesize[3];
	switch ( gHUD.m_iRes )
	{
		default:
		case 320: imagesize[0] = m_iIconWide_0; imagesize[1] = m_iIconTall_0; imagesize[2] = m_iTextYAdd_0; break;
		case 640: imagesize[0] = m_iIconWide_1; imagesize[1] = m_iIconTall_1; imagesize[2] = m_iTextYAdd_1; break;
		case 1280: imagesize[0] = m_iIconWide_2; imagesize[1] = m_iIconTall_2; imagesize[2] = m_iTextYAdd_2; break;
		case 2560: imagesize[0] = m_iIconWide_3; imagesize[1] = m_iIconTall_3; imagesize[2] = m_iTextYAdd_3; break;
	}

	vgui2::surface()->DrawSetTexture( m_iIconTexture );
	vgui2::surface()->DrawSetColor( m_clrIcon );
	vgui2::surface()->DrawTexturedRect( 0, 0, imagesize[0], imagesize[1] );

	// Let's move our text to the right a bit
	// but not too far, or too close.
	int xPos = imagesize[0] + m_iTextXAdd;

	// Now draw our lives
	m_pLives->SetBounds( xPos, imagesize[2], m_iTextSizeWide, m_iTextSizeTall );
}

int CHudZombieLives::MsgFunc_ZombieLives( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int lives = READ_SHORT();
	char text[12];
	sprintf( text, "%i", lives );
	m_pLives->SetText( text );
	m_pLives->SetVisible( true );
	return 1;
}
