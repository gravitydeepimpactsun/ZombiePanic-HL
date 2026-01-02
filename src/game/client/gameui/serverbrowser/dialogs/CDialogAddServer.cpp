// Ported from Source, but modified for GoldSrc

// For DNS stuff
#if defined( _WIN32 )
#include "winsani_in.h"
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <winsock.h>
#include "winsani_out.h"
#else
// Our include file
#include <netdb.h>		// gethostbyname()
#endif

#include <vgui/ILocalize.h>
#include <vgui/IInputInternal.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/ImageList.h>
#include "tier1/KeyValues.h"
#include "gameui/serverbrowser/CServerContextMenu.h"
#include "gameui/serverbrowser/CServerBrowser.h"
#include "gameui/serverbrowser/CBaseTab.h"
#include "gameui/serverbrowser/ServerListSorter.h"
#include "gameui/gameui_viewport.h"
#include "CDialogAddServer.h"
#include "client_vgui.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  : *gameList - game list to add specified server to
//-----------------------------------------------------------------------------
CDialogAddServer::CDialogAddServer( vgui2::Panel *parent, CBaseTab *gameList) : Frame(parent, "DialogAddServer" )
{
	SetProportional(true);
	SetDeleteSelfOnClose(true);

	m_pGameList = gameList;

	SetTitle("#ServerBrowser_AddServersTitle", true);
	SetSizeable( false );

	m_pTabPanel = new vgui2::PropertySheet(this, "GameTabs");
	m_pTabPanel->SetProportional(true);
	m_pTabPanel->SetTabWidth( GetScaledValue(72));

	m_pDiscoveredGames = new vgui2::ListPanel( this, "Servers" );
	m_pDiscoveredGames->SetAllowUserModificationOfColumns( true );
	// Add the column headers
	m_pDiscoveredGames->AddColumnHeader(0, "Name", "#ServerBrowser_Servers", GetScaledValue( 200 ), ListPanel::COLUMN_RESIZEWITHWINDOW | ListPanel::COLUMN_UNHIDABLE);
	m_pDiscoveredGames->AddColumnHeader(1, "IPAddr", "#ServerBrowser_IPAddress", GetScaledValue( 100 ) );
	m_pDiscoveredGames->AddColumnHeader(2, "Players", "#ServerBrowser_Players", GetScaledValue( 60 ));
	m_pDiscoveredGames->AddColumnHeader(3, "Map", "#ServerBrowser_Map", GetScaledValue( 100 ));
	m_pDiscoveredGames->AddColumnHeader(4, "Ping", "#ServerBrowser_Latency", GetScaledValue( 60 ));

	// setup fast sort functions
	m_pDiscoveredGames->SetSortFunc(0, ServerNameCompare);
	m_pDiscoveredGames->SetSortFunc(1, IPAddressCompare);
	m_pDiscoveredGames->SetSortFunc(2, PlayersCompare);
	m_pDiscoveredGames->SetSortFunc(3, MapCompare);
	m_pDiscoveredGames->SetSortFunc(4, PingCompare);

	m_pDiscoveredGames->SetSortColumn(4); // sort on ping

	m_pTextEntry = new vgui2::TextEntry( this, "ServerNameText" );
	m_pTextEntry->AddActionSignalTarget( this );
	
	m_pTestServersButton = new vgui2::Button( this, "TestServersButton", "" );
	m_pAddServerButton = new vgui2::Button( this, "OKButton", "" );
	m_pAddSelectedServerButton = new vgui2::Button( this, "SelectedOKButton", "", this, "addselected" );

	m_pTabPanel->AddPage( m_pDiscoveredGames, "#ServerBrowser_Servers" );

	LoadControlSettings("Servers/DialogAddServer.res");

	// Setup the buttons. We leave them disabled until there is text in the textbox.
	m_pAddServerButton->SetEnabled( false );
	m_pTestServersButton->SetEnabled( false );
	m_pAddSelectedServerButton->SetEnabled( false );
	m_pAddSelectedServerButton->SetVisible( false );
	m_pTabPanel->SetVisible( false );

	m_pTextEntry->RequestFocus();

	// Initially, we aren't high enough to show the tab panel.
	int x, y;
	m_pTabPanel->GetPos( x, y );
	m_OriginalHeight = m_pTabPanel->GetTall() + y + 50;
	SetTall( y );

	InvalidateLayout();
	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogAddServer::~CDialogAddServer()
{
	FOR_EACH_VEC( m_Queries, i )
	{
		if ( GetSteamAPI()->SteamMatchmakingServers() )
			GetSteamAPI()->SteamMatchmakingServers()->CancelServerQuery( m_Queries[ i ] );
	}
}

//-----------------------------------------------------------------------------
// Lets us know when the text entry has changed.
//-----------------------------------------------------------------------------
void CDialogAddServer::OnTextChanged()
{
	bool bAnyText = (m_pTextEntry->GetTextLength() > 0);
	m_pAddServerButton->SetEnabled( bAnyText );
	m_pTestServersButton->SetEnabled( bAnyText );
}

//-----------------------------------------------------------------------------
// Purpose: button command handler
//-----------------------------------------------------------------------------
void CDialogAddServer::OnCommand(const char *command)
{
	if ( Q_stricmp(command, "OK") == 0 )
	{
		OnOK();
	}
	else if ( Q_stricmp( command, "TestServers" ) == 0 )
	{
		SetTall( m_OriginalHeight );
		m_pTabPanel->SetVisible( true );
		m_pAddSelectedServerButton->SetVisible( true );
	
		TestServers();
	}
	else if ( !Q_stricmp( command, "addselected" ) )
	{
		if ( m_pDiscoveredGames->GetSelectedItemsCount() )
		{
			// get the server
			int serverID = m_pDiscoveredGames->GetItemUserData( m_pDiscoveredGames->GetSelectedItem(0) );
			CGameUIViewport::Get()->GetServerBrowser()->AddServerToFavorites( m_Servers[ serverID ] );
			m_pDiscoveredGames->RemoveItem( m_pDiscoveredGames->GetSelectedItem(0) ); // as we add to favs remove from the list
			m_pDiscoveredGames->SetEmptyListText( "" );
		}
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}


void CDialogAddServer::GrabIPAddress( const char *szAddress, unsigned int *nIP, uint16 *nQuery, uint16 *nPort )
{
	*nIP = 0;
	*nQuery = 0;
	*nPort = 0;

	char szHostName[ 256 ];
	Q_strncpy( szHostName, szAddress, sizeof(szHostName) );
	char *pchColon = strchr( szHostName, ':' );
	if ( pchColon )
		*pchColon = 0;

	// DNS it
	struct hostent *host = gethostbyname( szHostName );
	if ( !host ) return;

	*nIP = ntohl( *(int *)host->h_addr_list[0] );
	if ( pchColon )
		*nPort = atoi( ++pchColon );
}

void CDialogAddServer::GetMostCommonQueryPorts( CUtlVector<uint16> &ports )
{
	for ( int i=0; i <= 5; i++ )
	{
		ports.AddToTail( 27015 + i );
		ports.AddToTail( 26900 + i );
	}

	ports.AddToTail(4242);
	ports.AddToTail(27215);
}

//-----------------------------------------------------------------------------
// Purpose: Handles the OK button being pressed; adds the server to the game list
//-----------------------------------------------------------------------------
void CDialogAddServer::OnOK()
{
	// try and parse out IP address
	const char *address = GetControlString( "ServerNameText", "" );
	servernetadr_t netaddr;
	unsigned int nIP;
	uint16 nQuery, nConnection;
	GrabIPAddress( address, &nIP, &nQuery, &nConnection );
	netaddr.Init( nIP, nQuery, nConnection );
	if ( !netaddr.GetConnectionPort() )
	{
		// use the default port since it was not entered
		netaddr.SetConnectionPort( 27015 );
	}

	if ( netaddr.GetIP() != 0 )
	{
		gameserveritem_t server;
		memset(&server, 0, sizeof(server));
		server.SetName( address );

		// We assume here that the query and connection ports are the same. This is why it's much
		// better if they click "Servers" and choose a server in there.
		server.m_NetAdr = netaddr;

		server.m_nAppID = 0;
		CGameUIViewport::Get()->GetServerBrowser()->AddServerToFavorites( server );
	}
	else
	{
		// could not parse the ip address, popup an error
		vgui2::MessageBox *dlg = new vgui2::MessageBox("#ServerBrowser_AddServerErrorTitle", "#ServerBrowser_AddServerError");
		dlg->SetProportional(true);
		dlg->DoModal();
	}

	// mark ourselves to be closed
	PostMessage(this, new KeyValues("Close"));
}


//-----------------------------------------------------------------------------
// Purpose: Ping a particular IP for server presence
//-----------------------------------------------------------------------------
void CDialogAddServer::TestServers()
{
	if ( !GetSteamAPI()->SteamMatchmakingServers() )
		return;

	m_pDiscoveredGames->SetEmptyListText( "" );
	m_pDiscoveredGames->RemoveAll();

	// If they specified a port, then send a query to that port.
	const char *address = GetControlString( "ServerNameText", "" );
	servernetadr_t netaddr;
	unsigned int nIP;
	uint16 nQuery, nConnection;
	GrabIPAddress( address, &nIP, &nQuery, &nConnection );
	netaddr.Init( nIP, nQuery, nConnection );
	
	m_Servers.RemoveAll();
	CUtlVector<servernetadr_t> vecAdress;

	if ( netaddr.GetConnectionPort() == 0 )
	{
		// No port specified. Go to town on the ports.
		CUtlVector<uint16> portsToTry;
		GetMostCommonQueryPorts( portsToTry );
		
		for ( int i=0; i < portsToTry.Count(); i++ )
		{
			servernetadr_t newAddr = netaddr;
			newAddr.SetConnectionPort( portsToTry[i] );
			vecAdress.AddToTail( newAddr );
		} 
	}
	else
	{
		vecAdress.AddToTail( netaddr );
	}

	// Change the text on the tab panel..
	m_pTabPanel->RemoveAllPages();
		
	wchar_t wstr[512];
	if ( address[0] == 0 )
	{
		Q_wcsncpy( wstr, g_pVGuiLocalize->Find( "#ServerBrowser_ServersRespondingLocal"), sizeof( wstr ) );
	}
	else
	{
		wchar_t waddress[512];
		Q_UTF8ToUnicode( address, waddress, sizeof( waddress ) );
		g_pVGuiLocalize->ConstructString( wstr, sizeof( wstr ), g_pVGuiLocalize->Find( "#ServerBrowser_ServersResponding"), 1, waddress );
	}

	char str[512];
	Q_UnicodeToUTF8( wstr, str, sizeof( str ) );
	m_pTabPanel->AddPage( m_pDiscoveredGames, str );
	m_pTabPanel->InvalidateLayout( true, true );

	FOR_EACH_VEC( vecAdress, iAddress )
	{
		m_Queries.AddToTail( GetSteamAPI()->SteamMatchmakingServers()->PingServer( vecAdress[ iAddress ].GetIP(), vecAdress[ iAddress ].GetConnectionPort(), this ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: A server answered our ping
//-----------------------------------------------------------------------------
void CDialogAddServer::ServerResponded( gameserveritem_t &server )
{
	KeyValues *kv = new KeyValues( "Server" );

	kv->SetString( "name", server.GetName() );
	kv->SetString( "map", server.m_szMap );
	kv->SetString( "IPAddr", server.m_NetAdr.GetConnectionAddressString() );

	char buf[32];
	Q_snprintf(buf, sizeof(buf), "%d / %d", server.m_nPlayers, server.m_nMaxPlayers);
	kv->SetString("Players", buf);

	kv->SetInt("Ping", server.m_nPing);

	// new server, add to list
	int iServer = m_Servers.AddToTail( server );
	int iListID = m_pDiscoveredGames->AddItem(kv, iServer, false, false);
	if ( m_pDiscoveredGames->GetItemCount() == 1 )
	{
		m_pDiscoveredGames->AddSelectedItem( iListID );
	}
	kv->deleteThis();

	m_pDiscoveredGames->InvalidateLayout();
}

void CDialogAddServer::ServerFailedToRespond()
{
	m_pDiscoveredGames->SetEmptyListText( "#ServerBrowser_ServerNotResponding" );
}

void CDialogAddServer::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	vgui2::HFont hFont = pScheme->GetFont( "ListSmall", IsProportional() );
	if ( !hFont )
		hFont = pScheme->GetFont( "DefaultSmall", IsProportional() );

	m_pDiscoveredGames->SetFont( hFont );

	m_pDiscoveredGames->InvalidateLayout( true, true );
}


//-----------------------------------------------------------------------------
// Purpose: A server on the listed IP responded
//-----------------------------------------------------------------------------
void CDialogAddServer::OnItemSelected()
{
	int nSelectedItem = m_pDiscoveredGames->GetSelectedItem(0);
	if( nSelectedItem != -1 ) 
	{
		m_pAddSelectedServerButton->SetEnabled( true );
	}
	else
	{
		m_pAddSelectedServerButton->SetEnabled( false );
	}
}
