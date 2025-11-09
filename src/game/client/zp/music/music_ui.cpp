// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "../../hud/ammohistory.h"
#include "music_ui.h"
#include "music_manager.h"
#include "client_vgui.h"
#include "vgui/client_viewport.h"

// vgui2::VarArgs
#include "zp/ui/workshop/WorkshopItemList.h"

DEFINE_HUD_ELEM( CMusicUI );

CMusicUI::CMusicUI()
    : vgui2::Panel( NULL, "MusicUI" )
{
	SetParent( g_pViewport );

	vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile( VGUI2_ROOT_DIR "resource/ChatScheme.res", "ChatScheme" );
	SetScheme( scheme );

	SetPaintBackgroundEnabled( true );
	SetProportional( true );

	m_pTextTitle = new vgui2::Label( this, "text", "Unknown Track" );
	m_pTextTitle->SetContentAlignment( vgui2::Label::a_center );

	InvalidateLayout();
}

void CMusicUI::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Setup the background painting style
	SetPaintBackgroundType( 2 );
	SetPaintBorderEnabled( true );
	SetPaintBackgroundEnabled( true );

	// Set the background color
	m_bgColor = pScheme->GetColor("ChatBgColor", GetBgColor());
	SetBgColor( Color( m_bgColor.r(), m_bgColor.g(), m_bgColor.b(), 0 ) );

	vgui2::HFont fontNormal = pScheme->GetFont( "Default" );
	m_pTextTitle->SetFont( fontNormal );
	m_pTextTitle->SetPaintBackgroundEnabled( false );
	m_pTextTitle->SetFgColor( Color( 255, 255, 255, 0 ) );

	SetSize( 300, 30 );
}

bool CMusicUI::IsAllowedToDraw()
{
	return hud_draw.GetFloat() > 0.0f;
}

void CMusicUI::NewTrackPlaying()
{
	wchar_t *pString = g_pVGuiLocalize->Find( "ZP_UI_NowPlaying" );
	if ( !pString ) pString = L"Now Playing:";
	char szNowPlaying[128];
	g_pVGuiLocalize->ConvertUnicodeToANSI( pString, szNowPlaying, sizeof( szNowPlaying ) );
	m_pTextTitle->SetColorCodedText( vgui2::VarArgs( "%s %s", szNowPlaying, CMusicManager::GetInstance()->GetTrackName() ) );
	m_flDrawTime = gHUD.m_flTime + 4.0;
}

void CMusicUI::Paint()
{
	if ( CMusicManager::GetInstance() )
		CMusicManager::GetInstance()->OnThink();

	if ( !IsAllowedToDraw() )
	{
		m_pTextTitle->SetFgColor( Color( 255, 255, 255, 0 ) );
		return;
	}

	m_pTextTitle->SetSize( GetWide(), GetTall() );

	int middle_pos = ScreenWidth / 2;
	int half_width = GetWide() / 2;

	int x = middle_pos - half_width;
	int a1 = m_pTextTitle->GetFgColor().a();
	int a2 = m_bgColor.a();

	if ( m_flDrawTime - gHUD.m_flTime > 0 )
	{
		a1 += 5;
		a2 += 5;
	}
	else
	{
		a1 -= 3;
		a2 -= 3;
	}

	m_pTextTitle->SetFgColor( Color( 255, 255, 255, clamp( a1, 0, 255 ) ) );
	SetBgColor( Color( m_bgColor.r(), m_bgColor.g(), m_bgColor.b(), clamp( a2, 0, 64 ) ) );
	SetAlpha( clamp( a2, 0, 64 ) );
	SetPos( x, 0 );
}