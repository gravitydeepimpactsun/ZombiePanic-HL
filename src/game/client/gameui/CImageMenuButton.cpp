// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui/IVGui.h>
#include "CImageMenuButton.h"
#include "steam/steam_api.h"
#include "client_vgui.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>

#define COLOR_ACTIVE			Color( 255, 253, 161, 255 )
#define COLOR_ACTIVE_HELP		Color( 168, 166, 104, 255 )
#define COLOR_NOT_ACTIVE		Color( 255, 255, 255, 0 )
#define COLOR_NOT_ACTIVE_HELP	Color( 133, 133, 133, 0 )

CImageMenuButton::CImageMenuButton( vgui2::Panel *pParent, const char *szImage, const char *szURL )
    : BaseClass( pParent, "CImageMenuButton" )
{
	SetSize( 100, 80 );
	SetPos( 0, 0 );
	SetMouseInputEnabled( true );

	SetScheme( vgui2::scheme()->LoadSchemeFromFile( VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme") );

	m_szURL[0] = 0;
	m_szImage[0] = 0;
	m_szImageHover[0] = 0;

	if ( szURL )
		Q_strcpy( m_szURL, szURL );

	if ( szImage )
	{
		Q_strcpy( m_szImage, szImage );
		Q_strcpy( m_szImageHover, szImage );
	}

	m_Text = new vgui2::Label( this, "text", "" );
	m_Text->SetPos( 0, 0 );
	m_Text->SetPaintBackgroundEnabled( false );
	m_Text->SetMouseInputEnabled( false );
	m_Text->SetKeyBoardInputEnabled( false );
	m_Text->SetFgColor( COLOR_NOT_ACTIVE );
	m_Text->SetZPos( 1 );

	m_TextHelp = new vgui2::Label( this, "helptext", "" );
	m_TextHelp->SetPos( 0, 0 );
	m_TextHelp->SetPaintBackgroundEnabled( false );
	m_TextHelp->SetMouseInputEnabled( false );
	m_TextHelp->SetKeyBoardInputEnabled( false );
	m_TextHelp->SetFgColor( COLOR_NOT_ACTIVE_HELP );
	m_TextHelp->SetZPos( 0 );

	m_TextSize[0] = 0;
	m_TextSize[1] = 0;

	m_pPanel = new vgui2::ImagePanel( this, "Image" );
	m_pPanel->SetFillColor( Color( 0, 0, 0, 100 ) );
	m_pPanel->SetSize( GetWide(), GetTall() );
	m_pPanel->SetPos( 0, 0 );
	m_pPanel->SetShouldScaleImage( true );
	m_pPanel->SetImage( m_szImage );
	m_pPanel->SetMouseInputEnabled( false );
	m_pPanel->SetKeyBoardInputEnabled( false );
	m_pPanel->SetZPos( 2 );

	m_bIsHovering = false;

	vgui2::ivgui()->AddTickSignal( GetVPanel() );
}

void CImageMenuButton::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	vgui2::HFont hTextFont = pScheme->GetFont( "MenuItem" );
	if ( hTextFont != vgui2::INVALID_FONT )
		m_Text->SetFont( hTextFont );

	hTextFont = pScheme->GetFont( "MenuItemHelp" );
	if ( hTextFont != vgui2::INVALID_FONT )
		m_TextHelp->SetFont( hTextFont );

	m_Text->SetFgColor( COLOR_NOT_ACTIVE );
	m_TextHelp->SetFgColor( COLOR_NOT_ACTIVE_HELP );
}

void CImageMenuButton::SetImageHover( const char *szImage )
{
	if ( szImage )
		Q_strcpy( m_szImageHover, szImage );
}

void CImageMenuButton::OnTick()
{
	BaseClass::OnTick();

	if ( m_bIsHovering )
	{
		UpdateTextColor( m_Text, COLOR_ACTIVE, 0 );
		UpdateTextColor( m_TextHelp, COLOR_ACTIVE_HELP, 2 );
	}
	else
	{
		UpdateTextColor( m_Text, COLOR_NOT_ACTIVE, 15 );
		UpdateTextColor( m_TextHelp, COLOR_NOT_ACTIVE_HELP, 17 );
	}
}

void CImageMenuButton::OnCursorEntered()
{
	m_pPanel->SetImage( m_szImageHover );
	m_bIsHovering = true;
}

void CImageMenuButton::OnCursorExited()
{
	m_pPanel->SetImage( m_szImage );
	m_bIsHovering = false;
}

void CImageMenuButton::UpdateTextColor( vgui2::Label *pLabel, const Color vColorTarget, int xpos )
{
	Color TextColor = pLabel->GetFgColor();
	int rgb[4];
	rgb[0] = TextColor.r();
	rgb[1] = TextColor.g();
	rgb[2] = TextColor.b();
	rgb[3] = TextColor.a();

#define UPDATE_COLOR( ID, CLR ) \
if ( rgb[ID] > vColorTarget.CLR() ) rgb[ID] -= 8; \
else if ( rgb[ID] < vColorTarget.CLR() ) rgb[ID] += 8; \
rgb[ID] = clamp( rgb[ID], 0, 255 )

	// Set the new color
	UPDATE_COLOR( 0, r );
	UPDATE_COLOR( 1, g );
	UPDATE_COLOR( 2, b );
	UPDATE_COLOR( 3, a );
	pLabel->SetFgColor( Color( rgb[0], rgb[1], rgb[2], rgb[3] ) );

	// Set the new xPos
	int xPosCurrent = pLabel->GetXPos();
	if ( xPosCurrent > xpos )
		xPosCurrent--;
	else if ( xPosCurrent < xpos )
		xPosCurrent++;
	xPosCurrent = clamp( xPosCurrent, 0, m_TextSize[0] );
	pLabel->SetPos( xPosCurrent, pLabel->GetYPos() );
}

void CImageMenuButton::SetContent( const int &x, const int &y, const int &w, const int &h )
{
	if ( m_Text || m_TextHelp )
	{
		// We have text, so let's add some extra space.
		// We also need to nudge the X pos a little too.
		SetSize( w + m_TextSize[0] + 5, h + 5 );
		SetPos( x - m_TextSize[0] + 5, y );
		m_pPanel->SetSize( w, h );
		m_pPanel->SetPos( m_TextSize[0], 0 );
		m_pPanel->MoveToFront();
	}
	else
	{
		SetSize( w, h );
		SetPos( x, y );
		m_pPanel->SetSize( w, h );
	}
}

void CImageMenuButton::OnMousePressed( vgui2::MouseCode code )
{
	if ( code == vgui2::MouseCode::MOUSE_LEFT )
	{
		if ( !GetSteamAPI() ) return;
		if ( m_szURL && m_szURL[0] )
			GetSteamAPI()->SteamFriends()->ActivateGameOverlayToWebPage( m_szURL );
		return;
	}
	BaseClass::OnMousePressed( code );
}

void CImageMenuButton::SetText( const char *szText )
{
	SetText( szText, nullptr );
}

void CImageMenuButton::SetText( const char *szText, const char *szHelpText )
{
	vgui2::HFont hFont = vgui2::INVALID_FONT;
	vgui2::HFont hFontSmall = vgui2::INVALID_FONT;
	vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme( GetScheme() );
	if ( pScheme )
	{
		vgui2::HFont font = pScheme->GetFont( "MenuItem", IsProportional() );
	    if ( font != vgui2::INVALID_FONT )
			hFont = font;
		font = pScheme->GetFont( "MenuItemHelp", IsProportional() );
	    if ( font != vgui2::INVALID_FONT )
			hFontSmall = font;
	}

	if ( szText )
	{
		m_Text->SetText( szText );
		if ( hFont != vgui2::INVALID_FONT )
			m_Text->SetFont( hFont );
		m_Text->SetPos( 5, 0 );
		wchar_t wsText[256];
		g_pVGuiLocalize->ConvertANSIToUnicode( szText, wsText, sizeof( wsText ) );
		const wchar_t *pTranslated = g_pVGuiLocalize->Find( szText );
		if ( pTranslated )
			vgui2::surface()->GetTextSize( hFont, pTranslated, m_TextSize[0], m_TextSize[1] );
		else
			vgui2::surface()->GetTextSize( hFont, wsText, m_TextSize[0], m_TextSize[1] );
		m_Text->SetSize( m_TextSize[0], m_TextSize[1] );
	}

	if ( szHelpText )
	{
		m_TextHelp->SetText( szHelpText );
		if ( hFontSmall != vgui2::INVALID_FONT )
			m_TextHelp->SetFont( hFontSmall );
		if ( szText )
			m_TextHelp->SetPos( m_Text->GetXPos() + 5, m_Text->GetYPos() + m_TextSize[1] );
		else
			m_TextHelp->SetPos( 5, 0 );

		wchar_t wsText[256];
		int wide, tall;
		g_pVGuiLocalize->ConvertANSIToUnicode( szHelpText, wsText, sizeof( wsText ) );
		const wchar_t *pTranslated = g_pVGuiLocalize->Find( szHelpText );
		if ( pTranslated )
			vgui2::surface()->GetTextSize( hFontSmall, pTranslated, wide, tall );
		else
			vgui2::surface()->GetTextSize( hFontSmall, wsText, wide, tall );
		m_TextHelp->SetSize( wide, tall );
		if ( m_TextSize[0] == 0 && m_TextSize[1] == 0 )
		{
			m_TextSize[0] = wide;
			m_TextSize[1] = tall;
		}
	}

	m_Text->SetFgColor( COLOR_NOT_ACTIVE );
	m_TextHelp->SetFgColor( COLOR_NOT_ACTIVE_HELP );
}
