// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "CMenuPage.h"
#include "CMenuItem.h"
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include "client_vgui.h"
#include <FileSystem.h>
#include "tier1/KeyValues.h"
#include "zp/ui/workshop/WorkshopItemList.h"

CMenuPage::CMenuPage( vgui2::Panel *pParent, MenuPagesTable_t nType, const char *szTitle )
    : BaseClass( pParent, "CMenuPage" )
{
	SetSize( 100, 80 );
	SetPos( 0, 0 );
	SetPaintBackgroundEnabled( false );
	SetProportional( true );

	SetScheme( vgui2::scheme()->LoadSchemeFromFile( VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme") );

	m_nType = nType;

	m_pTitle = new vgui2::Label( this, "Title", szTitle );
	m_pTitle->SetPaintBackgroundEnabled( false );

	// Invalid by default.
	for ( int i = 0; i < MAX_PAGE_MENU_ITEMS; i++ )
		m_pMenuItems[i] = nullptr;

	m_IsConnected = false;
}

void CMenuPage::PopulateMenu()
{
	for ( int i = 0; i < MAX_PAGE_MENU_ITEMS; i++ )
	{
		if ( !m_pMenuItems[i] ) continue;
		m_pMenuItems[i]->DeletePanel();
		m_pMenuItems[i] = nullptr;
	}

	SetWide( 700 );
	SetTall( GetParent()->GetTall() );

	int yPos = 350;
	m_pTitle->SetBounds( 0, yPos, GetWide(), 60 );
	m_pTitle->SetContentAlignment( vgui2::Label::Alignment::a_center );

	// ui/mainmenu/%s.res
	// %s being replaced by:
	// MenuMain			- PAGE_MAIN
	// MenuOptions		- PAGE_OPTIONS
	// MenuExtras		- PAGE_EXTRAS
	KeyValuesAD kvData( "GameMenu" );

	const char *szFileName = nullptr;
	switch ( m_nType )
	{
		case MenuPagesTable_t::PAGE_MAIN: szFileName = "MenuMain"; break;
		case MenuPagesTable_t::PAGE_OPTIONS: szFileName = "MenuOptions"; break;
		case MenuPagesTable_t::PAGE_EXTRAS: szFileName = "MenuExtras"; break;
	}

	if ( kvData->LoadFromFile( g_pFullFileSystem, vgui2::VarArgs( "ui/mainmenu/%s.res", szFileName ), "GAME" ) )
	{
		KeyValues *kvSub = kvData->GetFirstSubKey();
		int m_iMenuItem = 0;

		while ( kvSub )
		{
			if ( m_iMenuItem == MAX_PAGE_MENU_ITEMS )
				break;
			bool bIngameItem = kvSub->GetBool( "OnlyInGame", false );
			if ( bIngameItem && !m_IsConnected )
			{
				kvSub = kvSub->GetNextKey();
				continue;
			}
			m_pMenuItems[m_iMenuItem] = new CMenuItem( this,
				kvSub->GetString( "Icon", "back" ),
				kvSub->GetString( "Label", "Example String" ),
				kvSub->GetString( "HelpText" ),
				kvSub->GetString( "Command" )
			);
			m_iMenuItem++;
			kvSub = kvSub->GetNextKey();
		}
	}
	
	yPos += m_pTitle->GetTall() + 25;
	for ( int i = 0; i < MAX_PAGE_MENU_ITEMS; i++ )
	{
		if ( !m_pMenuItems[i] ) continue;
		m_pMenuItems[ i ]->SetContent( 50, yPos, GetWide(), 80 );
		yPos += m_pMenuItems[ i ]->GetTall() + 5;
	}
}

void CMenuPage::DisablePanel( bool state )
{
	SetEnabled( !state );
	SetVisible( !state );
}

void CMenuPage::OnCommand( const char *pcCommand )
{
	GetParent()->OnCommand( pcCommand );
}

void CMenuPage::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	vgui2::HFont hTextFont = pScheme->GetFont( "MenuItemTitle" );
	if ( hTextFont != vgui2::INVALID_FONT )
		m_pTitle->SetFont( hTextFont );
}

void CMenuPage::OnThink()
{
	BaseClass::OnThink();

	// Are we even ingame/connected?
	if ( IsVisible() )
	{
		char buf[64];
		buf[0] = 0;
		V_FileBase( gEngfuncs.pfnGetLevelName(), buf, sizeof(buf) );
		bool bConnected = false;
		if ( buf && buf[0] ) bConnected = true;
		if ( m_IsConnected != bConnected )
		{
			m_IsConnected = bConnected;
			PopulateMenu();
		}
	}
}
