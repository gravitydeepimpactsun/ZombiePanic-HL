// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "CMenuItem.h"
#include "client_vgui.h"
#include "zp/ui/workshop/WorkshopItemList.h"

#define COLOR_ACTIVE		Color( 255, 253, 161, 255 )
#define COLOR_NOT_ACTIVE	Color( 255, 255, 255, 255 )

CMenuItem::CMenuItem( vgui2::Panel *pParent, const char *szImage, const char *szText, const char *szHelp, const char *szCommand )
    : BaseClass( pParent, "CMenuItem" )
{
	SetSize( 100, 80 );
	SetPos( 0, 0 );
	SetPaintBackgroundEnabled( false );
	SetProportional( true );
	SetMouseInputEnabled( true );

	SetScheme( vgui2::scheme()->LoadSchemeFromFile( VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme") );

	m_szCommand[0] = 0;
	if ( m_szCommand )
		Q_strcpy( m_szCommand, szCommand );

	m_pPanel = new vgui2::ImagePanel( this, "Image" );
	m_pPanel->SetFillColor( Color( 0, 0, 0, 100 ) );
	m_pPanel->SetSize( GetWide(), GetTall() );
	m_pPanel->SetPos( 0, 0 );
	m_pPanel->SetShouldScaleImage( true );
	m_pPanel->SetImage( vgui2::VarArgs( "ui/mainmenu/icons/%s", szImage ) );
	m_pPanel->SetMouseInputEnabled( false );
	m_pPanel->SetKeyBoardInputEnabled( false );

	m_pText = new vgui2::Label( this, "Text", szText );
	m_pText->SetContentAlignment( vgui2::Label::Alignment::a_west );
	m_pText->SetPaintBackgroundEnabled( false );
	m_pText->SetMouseInputEnabled( false );
	m_pText->SetKeyBoardInputEnabled( false );

	m_pHelpText = new vgui2::Label( this, "HelpMe", szHelp );
	m_pHelpText->SetContentAlignment( vgui2::Label::Alignment::a_west );
	m_pHelpText->SetPaintBackgroundEnabled( false );
	m_pHelpText->SetMouseInputEnabled( false );
	m_pHelpText->SetKeyBoardInputEnabled( false );
}

void CMenuItem::SetContent( const int &x, const int &y, const int &w, const int &h )
{
	SetSize( w, h );
	SetPos( x, y );
	m_pPanel->SetSize( h, h );

	m_pText->SetBounds( h, 10, w, 35 );
	m_pHelpText->SetBounds( h, 25, w, h );

	InvalidateLayout( true );
	Repaint();
}

void CMenuItem::OnMousePressed( vgui2::MouseCode code )
{
	if ( code == vgui2::MouseCode::MOUSE_LEFT )
	{
		GetParent()->OnCommand( m_szCommand );
		return;
	}
	BaseClass::OnMousePressed( code );
}

void CMenuItem::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	vgui2::HFont hTextFont = pScheme->GetFont( "MenuItem" );
	if ( hTextFont != vgui2::INVALID_FONT )
		m_pText->SetFont( hTextFont );

	hTextFont = pScheme->GetFont( "MenuItemHelp" );
	if ( hTextFont != vgui2::INVALID_FONT )
		m_pHelpText->SetFont( hTextFont );
}

void CMenuItem::OnCursorEntered()
{
	m_pText->SetFgColor( COLOR_ACTIVE );
	m_pHelpText->SetFgColor( COLOR_ACTIVE );
	vgui2::surface()->PlaySound( "sound/ui/launch_select1.wav" );
}

void CMenuItem::OnCursorExited()
{
	m_pText->SetFgColor( COLOR_NOT_ACTIVE );
	m_pHelpText->SetFgColor( COLOR_NOT_ACTIVE );
}
