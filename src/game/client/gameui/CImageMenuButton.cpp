// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui/IVGui.h>
#include "CImageMenuButton.h"
#include "steam/steam_api.h"

CImageMenuButton::CImageMenuButton( vgui2::Panel *pParent, const char *szImage, const char *szURL )
    : BaseClass( pParent, "CImageMenuButton" )
{
	SetSize( 100, 80 );
	SetPos( 0, 0 );
	SetMouseInputEnabled( true );

	m_szURL[0] = 0;
	if ( szURL )
		Q_strcpy( m_szURL, szURL );

	m_pPanel = new vgui2::ImagePanel( this, "Image" );
	m_pPanel->SetFillColor( Color( 0, 0, 0, 100 ) );
	m_pPanel->SetSize( GetWide(), GetTall() );
	m_pPanel->SetPos( 0, 0 );
	m_pPanel->SetShouldScaleImage( true );
	m_pPanel->SetImage( szImage );
	m_pPanel->SetMouseInputEnabled( false );
	m_pPanel->SetKeyBoardInputEnabled( false );
}

void CImageMenuButton::SetContent( const int &x, const int &y, const int &w, const int &h )
{
	SetSize( w, h );
	SetPos( x, y );
	m_pPanel->SetSize( GetWide(), GetTall() );
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

