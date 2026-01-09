#include <IBaseUI.h>
#include <IEngineVGui.h>
#include <vgui/ISurface.h>
#include <tier2/tier2.h>
#include "hud.h"
#include "cl_util.h"
#include "client_vgui.h"
#include "gameui_viewport.h"
#include "gameui_test_panel.h"
#include "serverbrowser/CServerBrowser.h"
#include "options/adv_options_dialog.h"
#include "zp/ui/achievements/C_AchievementDialog.h"
#include "zp/ui/playerselection/C_PlayerSelection.h"
#include "zp/ui/credits/C_ZPCredits.h"
#include "zp/ui/workshop/CWorkshopDialog.h"
#include "steam_achievements.h"
#include "menu/CBaseMenu.h"
#include <FileSystem.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "zp/music/music_manager.h"
#include "nlohmann/json.hpp"
#include "rp_manager.h"
#include "zp/zp_apicallback.h"

ClientAPIData_t g_ClientAPIData = {};

bool g_bIsConnected = false;
std::vector<PublishedFileId_t> m_InstalledAddons;
bool HasAlreadyDownloadedAddon( const PublishedFileId_t &nAddon )
{
	for ( size_t i = 0; i < m_InstalledAddons.size(); i++ )
	{
		PublishedFileId_t nFind = m_InstalledAddons[i];
		if ( nFind == nAddon ) return true;
	}
	return false;
}

#if defined(_DEBUG)
CON_COMMAND(gameui_cl_open_test_panel, "Opens a test panel for client GameUI")
{
	CGameUIViewport::Get()->OpenTestPanel();
}
#endif

CGameUIViewport::CGameUIViewport()
    : BaseClass(nullptr, "ClientGameUIViewport"),
	m_CallbackUserStatsReceived( this, &CGameUIViewport::OnUserStatsReceived ),
	m_SteamCallbackResultOnDownloadItemResult( this, &CGameUIViewport::OnDownloadItemResult )
{
	Assert(!m_sInstance);
	m_sInstance = this;

	SetParent(g_pEngineVGui->GetPanel(PANEL_GAMEUIDLL));
	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));

	SetSize(0, 0);

	m_bDownloadedItemsReady = false;
	m_bNeedToReconnectAfterDownload = false;
	m_bPrepareForQueryDownload = false;
	m_hWorkshopInfoBox = nullptr;
	SetQueryWait( 1.0f );

	LoadWorkshopItems( false );
	LoadWorkshop();

	RequestStats();

	// API call to grab our donators dynamically. We only do this on game start.
	if ( GetSteamAPI()->SteamHTTP() && GetSteamAPI()->SteamUser() )
	{
		SteamAPICall_t pCall = k_uAPICallInvalid;
		HTTPRequestHandle request = GetSteamAPI()->SteamHTTP()->CreateHTTPRequest(
			k_EHTTPMethodGET,
			vgui2::VarArgs( "https://api.wuffesan.com/donators/check/?steamid=%llu&v=3", GetSteamAPI()->SteamUser()->GetSteamID().ConvertToUint64() )
		);
		GetSteamAPI()->SteamHTTP()->SetHTTPRequestHeaderValue( request, "Cache-Control", "no-cache");
		GetSteamAPI()->SteamHTTP()->SetHTTPRequestContextValue( request, READ_DONATOR_STATUS );
		GetSteamAPI()->SteamHTTP()->SendHTTPRequest( request, &pCall );
		m_callHTTPResult.Set( pCall, this, &CGameUIViewport::UpdateHTTPCallback );
	}

	int w, t;
	vgui2::surface()->GetScreenSize( w, t );
	m_hMenu = new CBaseMenu( this );
	m_hMenu->MakePopup( false, false );
	m_hMenu->SetMenuBounds( 0, 0, w, t );
	m_hMenu->LoadPage( MenuPagesTable_t::PAGE_MAIN );

	// Don't start with maxplayers set to 1.
	// Default should be 16.
	gEngfuncs.pfnClientCmd("maxplayers 16\n");

	// Let's create our music manager after our GameUI is created.
	new CMusicManager();

	new CRichPresenceManager();
}

CGameUIViewport::~CGameUIViewport()
{
	Assert(m_sInstance);
	m_sInstance = nullptr;
}

void CGameUIViewport::OnUserStatsReceived( UserStatsReceived_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (GetSteamAPI()->SteamUtils()->GetAppID() == pCallback->m_nGameID)
	{
		STAT_OnUserStatsReceived( pCallback );
	}
}

void CGameUIViewport::PreventEscapeToShow(bool state)
{
	if (state)
	{
		m_bPreventEscape = true;
		m_iDelayedPreventEscapeFrame = 0;
	}
	else
	{
		// PreventEscapeToShow(false) may be called the same frame that ESC was pressed
		// and CGameUIViewport::OnThink won't hide GameUI
		// So the change is delayed by one frame
		m_bPreventEscape = false;
		m_iDelayedPreventEscapeFrame = gHUD.GetFrameCount();
	}
}

void CGameUIViewport::OpenTestPanel()
{
	GetDialog(m_hTestPanel)->Activate();
}

CAdvOptionsDialog *CGameUIViewport::GetOptionsDialog()
{
	return GetDialog(m_hOptionsDialog);
}

C_AchievementDialog *CGameUIViewport::GetAchievementDialog()
{
	return GetDialog(m_hAchDialog);
}

C_PlayerSelection *CGameUIViewport::GetPlayerSelection()
{
	return GetDialog(m_hPlayerSelection);
}

C_ZPCredits *CGameUIViewport::GetCredits()
{
	return GetDialog(m_hCredits);
}

CWorkshopDialog *CGameUIViewport::GetWorkshopDialog()
{
	return GetDialog(m_hWorkshopDialog);
}

CServerBrowser *CGameUIViewport::GetServerBrowser()
{
	return GetDialog(m_hServerBrowser);
}

CBaseMenu *CGameUIViewport::GetMenu()
{
	return GetDialog(m_hMenu);
}

vgui2::Panel *CGameUIViewport::GetDialog( GameUIDialogs nDialog )
{
	switch ( nDialog )
	{
		case UIDialog_Achievements: return m_hAchDialog;
		case UIDialog_Options: return m_hOptionsDialog;
		case UIDialog_PlayerSelection: return m_hPlayerSelection;
		case UIDialog_Workshop: return m_hWorkshopDialog;
		case UIDialog_Credits: return m_hCredits;
		case UIDialog_ServerBrowser: return m_hServerBrowser;
	}
	return nullptr;
}

void CGameUIViewport::SetQueryWait( const float &flTime )
{
	m_flQueryWait = flTime * 60;
}

void CGameUIViewport::OnThink()
{
	BaseClass::OnThink();

	if ( CMusicManager::GetInstance() )
	{
		char buf[64];
		buf[0] = 0;
		V_FileBase( gEngfuncs.pfnGetLevelName(), buf, sizeof(buf) );
		bool bConnected = false;
		if ( buf && buf[0] ) bConnected = true;
		// Are we even ingame/connected?
		if ( g_bIsConnected != bConnected )
		{
			g_bIsConnected = bConnected;
			CMusicManager::GetInstance()->OnMapShutdown();
			m_hMenu->Repopulate();
			m_hMenu->ToggleBackground( !g_bIsConnected );
		}
	}

	if (m_bPreventEscape || m_iDelayedPreventEscapeFrame == gHUD.GetFrameCount())
	{
		const char *levelName = gEngfuncs.pfnGetLevelName();

		if (levelName && levelName[0])
		{
			g_pBaseUI->HideGameUI();

			// Hiding GameUI doesn't update the mouse cursor
			g_pVGuiSurface->CalculateMouseVisible();
		}
		else
		{
			// Disconnected from the server while prevent escape is enabled
			// Disable it
			m_bPreventEscape = false;
			m_iDelayedPreventEscapeFrame = 0;
		}
	}

	if ( m_bPrepareForQueryDownload )
	{
		// We are currently downloading, check for progress.
		if ( m_CurrentQueryItem.WorkshopID > 0 )
		{
			uint32 eState = GetSteamAPI()->SteamUGC()->GetItemState( m_CurrentQueryItem.WorkshopID );
			bool bCompleted = (( eState & k_EItemStateInstalled ) != 0);

			uint64 progress[2];
			GetSteamAPI()->SteamUGC()->GetItemDownloadInfo( m_CurrentQueryItem.WorkshopID, &progress[0], &progress[1] );
			float flCurrent = (float)progress[0];
			float flTotal = (float)progress[1];
			float flProgress = flCurrent / flTotal;
			SetWorkshopInfoBoxProgress( flProgress );
			if ( flProgress == 1.0f || bCompleted )
			{
				SetQueryWait( 0.35f );
				ShowWorkshopInfoBox( m_CurrentQueryItem.Title, WorkshopInfoBoxState::State_Done );
				bool bHasAddon = HasAlreadyDownloadedAddon( m_CurrentQueryItem.WorkshopID );
				m_CurrentQueryItem.WorkshopID = 0;

				// If already subscribed, ignore.
				//if ( ( eState & k_EItemStateSubscribed ) )
				//	return;

				// We were downloading before? If so, make sure we reconnect even if bHasAddon was true
				if ( ( eState & k_EItemStateDownloading || eState & k_EItemStateNeedsUpdate ) )
					bHasAddon = false;

				if ( m_CurrentQueryItem.Reconnect && !bHasAddon )
				{
					SetWorkshopInfoBoxProgress( 1.0f );
					m_bNeedToReconnectAfterDownload = true;
				}
			}
			return;
		}

		if ( m_flQueryWait > 0.0f )
		{
			m_flQueryWait -= 1.0f;
			return;
		}

		// Prepare the next query.
		if ( PrepareForQueryDownload() )
			return;

		// Clear it
		m_QueryRequests.clear();

		if ( !m_bDownloadedItemsReady ) return;

		// Returned false? then stop the query download.
		m_bPrepareForQueryDownload = false;

		// Now check the client workshop content
		LoadWorkshopItems( true );

		if ( m_bNeedToReconnectAfterDownload )
		{
			m_bNeedToReconnectAfterDownload = false;
			if ( GetWorkshopInfoBoxState() == WorkshopInfoBoxState::State_DownloadingMapContent )
				ShowWorkshopInfoBox( "", WorkshopInfoBoxState::State_DownloadingMapContentDisconnect );
		}

		RebuiltAddonList();
		UpdateWorkshopMapsFile( false );
		UpdateWorkshopMapsFile( true );
	}
	else
		CheckWorkshopSubscriptions();
}

void CGameUIViewport::OnDownloadItemResult( DownloadItemResult_t *pCallback )
{
	// Item either downloaded properly or not.
	// Let's continue, and try reading the directory after 1 second delay.
	SetQueryWait( 1.0f );
	m_bDownloadedItemsReady = true;
	m_bPrepareForQueryDownload = true;

	Color clr = Color( 255, 22, 22, 255 );
	const char *szMsg = nullptr;
	switch ( pCallback->m_eResult )
	{
		case k_EResultOK: szMsg = "Successfully downloaded Workshop Item"; clr = Color( 0, 255, 255, 255 ); break;
		case k_EResultFail: szMsg = "Failed to download Workshop Item"; break;
		case k_EResultNoConnection: szMsg = "Failed to download: No Internet Connection"; break;
		case k_EResultInvalidPassword: szMsg = "Failed to download: Invalid Password"; break;
		case k_EResultLoggedInElsewhere: szMsg = "Failed to download: Already logged in elsewhere"; break;
		case k_EResultInvalidProtocolVer: szMsg = "Failed to download: Invalid Protocol Version"; break;
		case k_EResultInvalidParam: szMsg = "Failed to download: Invalid parameter"; break;
		case k_EResultFileNotFound: szMsg = "Failed to download: File Not Found"; break;
		case k_EResultAccessDenied: szMsg = "Failed to download: Access Denied"; break;
		case k_EResultTimeout: szMsg = "Failed to download: Connection Timed Out"; break;
		case k_EResultAccountNotFound: szMsg = "Failed to download: Account Not Found"; break;
		case k_EResultBanned: szMsg = "Failed to download: VAC Banned"; break;
		case k_EResultInvalidSteamID: szMsg = "Failed to download: Invalid SteamID"; break;
		case k_EResultNotLoggedOn: szMsg = "Failed to download: Not Logged On"; break;
		case k_EResultServiceUnavailable: szMsg = "Failed to download: Service Unavailable"; break;
		default: szMsg = vgui2::VarArgs( "Returned result ID [%i]", pCallback->m_eResult ); break;
	}

	ConPrintf( clr, "[Workshop] %s [%llu]\n", szMsg, pCallback->m_nPublishedFileId );

	// Make sure we add this.
	m_SubscribedItems.push_back( pCallback->m_nPublishedFileId );
}

void CGameUIViewport::GetCurrentItems( std::vector<vgui2::WorkshopItem> &items )
{
	items = m_Items;
}

void CGameUIViewport::UpdateWorkshopMapsFile( const bool &bWorkshopFolder )
{
	// Make sure the file exist
	KeyValuesAD kvFileCheck( new KeyValues( "Workshop" ) );
	if ( !kvFileCheck->LoadFromFile( g_pFullFileSystem, "workshop_maps.kv", "WORKSHOP" ) )
		kvFileCheck->SaveToFile( g_pFullFileSystem, "workshop_maps.kv", "WORKSHOP" );

	for ( int iID = 0; iID < m_Items.size(); iID++ )
	{
		vgui2::WorkshopItem &WorkshopAddon = m_Items[iID];
		if ( WorkshopAddon.iFilterFlag & vgui2::FILTER_MAP && WorkshopAddon.uWorkshopID != 0 )
		{
			FileFindHandle_t mapfh;
			char const *mapfn = g_pFullFileSystem->FindFirst( vgui2::VarArgs( "%llu/maps/*.bsp", WorkshopAddon.uWorkshopID ), &mapfh, bWorkshopFolder ? "WORKSHOPDL" : "WORKSHOP" );
			if ( mapfn )
			{
				do
				{
					KeyValuesAD autoMapData( new KeyValues( "Workshop" ) );
					if ( autoMapData->LoadFromFile( g_pFullFileSystem, "workshop_maps.kv", "WORKSHOP" ) )
					{
						autoMapData->SetString( mapfn, vgui2::VarArgs( "id=%llu", WorkshopAddon.uWorkshopID ) );
						autoMapData->SaveToFile( g_pFullFileSystem, "workshop_maps.kv", "WORKSHOP" );
					}
				}
				while ( ( mapfn = g_pFullFileSystem->FindNext( mapfh ) ) != NULL );
				g_pFullFileSystem->FindClose( mapfh );
			}
		}
	}
}

// ===================================
// Purpose: Load Workshop Content
// ===================================
void CGameUIViewport::LoadWorkshop()
{
	if ( !GetSteamAPI() ) return;
	if ( GetSteamAPI()->SteamUGC() && GetSteamAPI()->SteamUser() )
	{
		handle = GetSteamAPI()->SteamUGC()->CreateQueryUserUGCRequest(
			GetSteamAPI()->SteamUser()->GetSteamID().GetAccountID(),
			k_EUserUGCList_Subscribed,
			k_EUGCMatchingUGCType_Items_ReadyToUse,
			k_EUserUGCListSortOrder_LastUpdatedDesc,
		    (AppId_t)3825360, (AppId_t)3825360, 1
		);
		GetSteamAPI()->SteamUGC()->SetReturnChildren( handle, true );
		SteamAPICall_t apiCall = GetSteamAPI()->SteamUGC()->SendQueryUGCRequest( handle );
		m_SteamCallResultOnSendQueryUGCRequest.Set( apiCall, this, &CGameUIViewport::OnSendQueryUGCRequest );
	}
}

void CGameUIViewport::CheckWorkshopSubscriptions()
{
	if ( !GetSteamAPI() ) return;
	if ( !GetSteamAPI()->SteamUGC() ) return;
	if ( m_flQueryWait > 0.0f )
	{
		m_flQueryWait -= 1.0f;
		return;
	}
	SetQueryWait( 1.55f );

	// Get dynamic count of subscribed items and allocate accordingly.
	uint32 maxItems = GetSteamAPI()->SteamUGC()->GetNumSubscribedItems();

	std::vector<PublishedFileId_t> vWorkshopItems;
	if ( maxItems > 0 )
		vWorkshopItems.resize( maxItems );

	uint32 nItems = 0;
	if ( maxItems > 0 )
		nItems = GetSteamAPI()->SteamUGC()->GetSubscribedItems( vWorkshopItems.data(), maxItems );
	else
		nItems = 0;

	std::vector<PublishedFileId_t> m_SubCheckList;
	for ( uint32 i = 0; i < nItems; ++i )
	{
		// We found a new subscribed item? Download it!
		// This only returns true if we subscribed to an item while in-game.
		if ( !HasSubscribedToItem( vWorkshopItems[i] ) )
			DownloadWorkshopAddon( vWorkshopItems[i], false );
		m_SubCheckList.push_back( vWorkshopItems[i] );
	}

	// Now check if we uninstalled any addons.
	for ( int iID = 0; iID < m_Items.size(); iID++ )
	{
		vgui2::WorkshopItem &WorkshopAddon = m_Items[iID];
		bool bHasSubscribed = false;
		for ( size_t i = 0; i < m_SubCheckList.size(); i++ )
		{
			// We already have this in the list? then skip.
			if ( WorkshopAddon.uWorkshopID == m_SubCheckList[i] )
			{
				bHasSubscribed = true;
				break;
			}
		}
		if ( bHasSubscribed ) continue;

		// We have an item here that we unsubscribed from, Unmount it (if activated) then purge it from the list.
		if ( WorkshopAddon.bMounted )
		{
			CGameUIViewport::Get()->ShowWorkshopInfoBox( WorkshopAddon.szName, WorkshopInfoBoxState::State_Dismounting );
			CGameUIViewport::Get()->MountWorkshopItem( WorkshopAddon, nullptr, nullptr );
		}
		else
			RemoveWorkshopItem( iID );
	}
}

void CGameUIViewport::RemoveWorkshopItem( const int &nID )
{
	// Purge from the list
	m_Items.erase( m_Items.begin() + nID );
	// Remove the item from the addonlist.txt
	RebuiltAddonList();
}

bool CGameUIViewport::HasSubscribedToItem( PublishedFileId_t nWorkshopID )
{
	for ( size_t i = 0; i < m_SubscribedItems.size(); i++ )
	{
		if ( m_SubscribedItems[i] == nWorkshopID )
			return true;
	}
	return false;
}

bool CGameUIViewport::HasLoadedItem( PublishedFileId_t nWorkshopID )
{
	for ( int iID = 0; iID < m_Items.size(); iID++ )
	{
		vgui2::WorkshopItem &WorkshopAddon = m_Items[iID];
		if ( WorkshopAddon.uWorkshopID == nWorkshopID )
			return true;
	}
	return false;
}

void CGameUIViewport::LoadWorkshopItems( bool bWorkshopFolder )
{
#if defined( _DEBUG )
	// Only show on Debug mode
	ConPrintf(
		Color( 255, 255, 0, 255 ),
		"LoadWorkshopItems | IsWorkshopFolder: %s\n",
	    bWorkshopFolder ? "TRUE" : "FALSE"
	);
#endif
	// Load our data from zp_workshop
	FileFindHandle_t fh;
	char const *fn = g_pFullFileSystem->FindFirst( "*.*", &fh, bWorkshopFolder ? "WORKSHOPDL" : "WORKSHOP" );
	if ( !fn )
	{
#if defined( _DEBUG )
		// Only show on Debug mode
		ConPrintf(
			Color( 255, 0, 0, 255 ),
			"Failed to find any files within PathID %s.\n",
		    bWorkshopFolder ? "WORKSHOPDL" : "WORKSHOP"
		);
#endif
		return;
	}
	do
	{
		// Setup the path string, and lowercase it, so we don't need to search for both uppercase, and lowercase files.
		char strFile[ 4028 ];
		bool isSameDir = false;
		strFile[ 0 ] = 0;
		V_strcpy_safe( strFile, fn );
		Q_strlower( strFile );

		// Ignore the same folder
		// And ignore content folder, unless we are loading workshop content trough it.
		if ( vgui2::FStrEq( strFile, "." ) || vgui2::FStrEq( strFile, ".." ) )
			isSameDir = true;

		// Folder found!
		if ( g_pFullFileSystem->FindIsDirectory( fh ) && !isSameDir )
		{
			// Setup the string
			std::string strAddonInfo = strFile;
			strAddonInfo += "/addoninfo.txt";

			// The Workshop file item.
			unsigned long long nFileItem = std::strtoull( strFile, NULL, 0 );

#if defined( _DEBUG )
			// Only show on Debug mode
			ConPrintf(
				Color( 255, 255, 0, 255 ),
				"Checking file [%s] | HasLoadedItem: %s\n",
			    strAddonInfo.c_str(),
				HasLoadedItem( nFileItem ) ? "TRUE": "FALSE"
			);
#endif

			// Check if the file exist
			if ( g_pFullFileSystem->FileExists( strAddonInfo.c_str() ) && !HasLoadedItem( nFileItem ) )
			{

				vgui2::WorkshopItem MountAddon;
				MountAddon.iFilterFlag = 0;
				MountAddon.uWorkshopID = nFileItem;
				MountAddon.bMounted = false;
				MountAddon.bIsWorkshopDownload = bWorkshopFolder;

				// Default title
				Q_snprintf( MountAddon.szName, sizeof( MountAddon.szName ), "%s", strFile );
				Q_snprintf( MountAddon.szDesc, sizeof( MountAddon.szDesc ), "Unknown 3rd party addon" );
				Q_snprintf( MountAddon.szAuthor, sizeof( MountAddon.szAuthor ), "Unknown Author" );

				// Check if the keyvalues exist
				KeyValues *manifest = new KeyValues( "AddonInfo" );
				if ( manifest->LoadFromFile( g_pFullFileSystem, strAddonInfo.c_str(), bWorkshopFolder ? "WORKSHOPDL" : "WORKSHOP" ) )
				{
					// Go trough all keyvalues, and grab the ones we need
					for ( KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
					{
						int length = Q_strlen( sub->GetString() ) + 1;
						char *strValue = (char *)malloc( length );

						Q_strcpy( strValue, sub->GetString() );

						if ( !Q_stricmp( sub->GetName(), "Author" ) )
							Q_snprintf( MountAddon.szAuthor, sizeof( MountAddon.szAuthor ), "%s", strValue );
						else if ( !Q_stricmp( sub->GetName(), "Description" ) )
							Q_snprintf( MountAddon.szDesc, sizeof( MountAddon.szDesc ), "%s", strValue );
						else if ( !Q_stricmp( sub->GetName(), "Title" ) )
							Q_snprintf( MountAddon.szName, sizeof( MountAddon.szName ), "%s", strValue );
					}
					// Go trough our flags
					KeyValues *pAddonFilterFlags = manifest->FindKey( "Tags" );
					if ( pAddonFilterFlags )
					{
						if ( pAddonFilterFlags->GetBool( "Map" ) ) MountAddon.iFilterFlag |= vgui2::FILTER_MAP;
						if ( pAddonFilterFlags->GetBool( "Weapons" ) ) MountAddon.iFilterFlag |= vgui2::FILTER_WEAPONS;
						if ( pAddonFilterFlags->GetBool( "Sounds" ) ) MountAddon.iFilterFlag |= vgui2::FILTER_SOUNDS;
						if ( pAddonFilterFlags->GetBool( "Survivor" ) ) MountAddon.iFilterFlag |= vgui2::FILTER_SURVIVOR;
						if ( pAddonFilterFlags->GetBool( "Zombie" ) ) MountAddon.iFilterFlag |= vgui2::FILTER_ZOMBIE;
						if ( pAddonFilterFlags->GetBool( "Background" ) ) MountAddon.iFilterFlag |= vgui2::FILTER_BACKGROUND;
						if ( pAddonFilterFlags->GetBool( "Sprays" ) ) MountAddon.iFilterFlag |= vgui2::FILTER_SPRAYS;
						if ( pAddonFilterFlags->GetBool( "Music" ) ) MountAddon.iFilterFlag |= vgui2::FILTER_MUSIC;
					}
				}
				manifest->deleteThis();

				// If filter flag has FILTER_MAP, we need to grab the .bsp files and tie them to this addon.
				// This is used if we are hosting a Peer-2-Peer server, to let clients download the map through the workshop.
				// The rest is read by the server, all we do here is grab the files, read them,
				// and then store their path and addon ID into a simple struct.
				if ( MountAddon.iFilterFlag & vgui2::FILTER_MAP && MountAddon.uWorkshopID != 0 )
				{
					FileFindHandle_t mapfh;
					char const *mapfn = g_pFullFileSystem->FindFirst( vgui2::VarArgs( "%s/maps/*.bsp", strFile ), &mapfh, bWorkshopFolder ? "WORKSHOPDL" : "WORKSHOP" );
					if ( mapfn )
					{
						do
						{
							KeyValuesAD autoMapData( new KeyValues( "Workshop" ) );
							if ( autoMapData->LoadFromFile( g_pFullFileSystem, "workshop_maps.kv", "WORKSHOP" ) )
							{
								autoMapData->SetString( mapfn, vgui2::VarArgs( "id=%llu", MountAddon.uWorkshopID ) );
								autoMapData->SaveToFile( g_pFullFileSystem, "workshop_maps.kv", "WORKSHOP" );
							}
						}
						while ( ( mapfn = g_pFullFileSystem->FindNext( mapfh ) ) != NULL );
						g_pFullFileSystem->FindClose( mapfh );
					}
				}

#if defined( _DEBUG )
				// Only show on Debug mode
				ConPrintf(
					Color( 255, 255, 0, 255 ),
					"Workshop Addon [%llu] Added | Flags: %i\n",
					MountAddon.uWorkshopID,
					MountAddon.iFilterFlag
				);
#endif
				// Auto mount, if found.
				AutoMountWorkshopItem( MountAddon );
				m_Items.push_back( MountAddon );
			}
		}

		fn = g_pFullFileSystem->FindNext(fh);
	}
	while(fn);

	g_pFullFileSystem->FindClose(fh);
}

void CGameUIViewport::RebuiltAddonList()
{
	KeyValues *pAddonList = new KeyValues( "AddonList" );
	KeyValuesAD autodel( pAddonList );
	for ( int iID = 0; iID < m_Items.size(); iID++ )
	{
		vgui2::WorkshopItem &WorkshopAddon = m_Items[iID];
		std::string strWorkshopID( std::to_string( WorkshopAddon.uWorkshopID ) );
		pAddonList->SetBool( strWorkshopID.c_str(), WorkshopAddon.bMounted );
	}
	// Save the file
	pAddonList->SaveToFile( g_pFullFileSystem, "addonlist.txt", "WORKSHOP" );
}

void CGameUIViewport::AutoMountWorkshopItem( vgui2::WorkshopItem &WorkshopFile )
{
	if ( ShouldAutoMount( WorkshopFile.uWorkshopID ) )
	{
		// Mark it as not mounted, so we can copy the files again (if they already exist)
		if ( m_bNeedToReconnectAfterDownload )
			WorkshopFile.bMounted = false;

		CGameUIViewport::Get()->ShowWorkshopInfoBox( WorkshopFile.szName, WorkshopInfoBoxState::State_Mounting );
		CGameUIViewport::Get()->MountWorkshopItem( WorkshopFile, nullptr, nullptr );
#if defined( _DEBUG )
		// Only show on Debug mode
		ConPrintf(
			Color( 0, 255, 255, 255 ),
			"ShouldAutoMount [%llu] -- MOUNTED\n",
			WorkshopFile.uWorkshopID
		);
#endif
		WorkshopFile.bMounted = true;
		return;
	}
	if ( !WorkshopIDIsMounted( WorkshopFile.uWorkshopID ) ) return;
	WorkshopFile.bMounted = true;
}

struct CopyPath
{
	std::string from;
	std::string to;
	uint64 item;
};

// From: https://stackoverflow.com/questions/6163611/compare-two-files
bool CompareFiles( const std::string &p1, const std::string &p2 )
{
	std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
	std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

	if (f1.fail() || f2.fail())
	{
		return false; //file problem
	}

	if (f1.tellg() != f2.tellg())
	{
		return false; //size mismatch
	}

	//seek back to beginning and use std::equal to compare contents
	f1.seekg(0, std::ifstream::beg);
	f2.seekg(0, std::ifstream::beg);
	return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
	    std::istreambuf_iterator<char>(),
	    std::istreambuf_iterator<char>(f2.rdbuf()));
}

unsigned CopyFilesToNewDestination( void *Data )
{
	bool bNoChange = CompareFiles( ((CopyPath *)Data)->from, ((CopyPath *)Data)->to );
	if ( std::filesystem::exists( ((CopyPath *)Data)->to ) )
		CGameUIViewport::Get()->SetConflictingFiles(((CopyPath *)Data)->item, !bNoChange);
	std::ifstream src( ((CopyPath *)Data)->from, std::ios::binary );
	std::ofstream dest( ((CopyPath *)Data)->to, std::ios::binary );
	dest << src.rdbuf();
	return 1;
}

struct DeleteFile
{
	std::string file;
};

unsigned RemoveFilesFromAddons( void *Data )
{
	g_pFullFileSystem->RemoveFile( ((DeleteFile *)Data)->file.c_str(), "ADDON" );
	return 1;
}

static bool MountingCheck( const char *szStrFile )
{
	const char *ext = strrchr( szStrFile, '.' );
	if ( !ext || !ext[1] )
	{
		// No extension => deny
		return false;
	}

	// Skip the dot
	ext = ext + 1;

	// Allowed extensions (lowercase). Add or remove as needed.
	const char *allowedExts[] = {
		"wad", "wav", "mp3", "ogg"
	};
	const size_t allowedCount = sizeof(allowedExts) / sizeof(allowedExts[0]);

	bool bAllowed = false;
	for ( size_t i = 0; i < allowedCount; ++i )
	{
		if ( Q_stricmp( ext, allowedExts[i] ) == 0 )
		{
			bAllowed = true;
			break;
		}
	}

	return bAllowed;
}

void CGameUIViewport::MountWorkshopItem( vgui2::WorkshopItem WorkshopFile, const char *szPath, const char *szRootPath )
{
	// Copies files to zp_addon,
	// or remove them from zp_addon.
	// Depends really, if we are mounting or not.

	bool bIsRoot = szRootPath ? false : true;

	char path[ 4028 ];
	char pathRoot[ 4028 ];
	if ( !szPath )
		Q_snprintf( path, sizeof( path ), "%llu/", WorkshopFile.uWorkshopID );
	else
		Q_snprintf( path, sizeof( path ), "%s", szPath );

	if ( !szRootPath )
		Q_snprintf( pathRoot, sizeof( pathRoot ), "%s", path );
	else
		Q_snprintf( pathRoot, sizeof( pathRoot ), "%s", szRootPath );
	
	// Load our data from zp_workshop
	FileFindHandle_t fh;
	char const *fn = g_pFullFileSystem->FindFirst( vgui2::VarArgs( "%s*.*", path ), &fh, WorkshopFile.bIsWorkshopDownload ? "WORKSHOPDL" : "WORKSHOP" );
	if ( !fn ) return;
	do
	{
		// Setup the path string, and lowercase it, so we don't need to search for both uppercase, and lowercase files.
		char strFile[ 4028 ];
		bool bIsValidFile = true;
		strFile[ 0 ] = 0;
		V_strcpy_safe( strFile, fn );
		Q_strlower( strFile );

		// Ignore the same folder
		// And ignore content folder, unless we are loading workshop content trough it.
		if ( vgui2::FStrEq( strFile, "." ) || vgui2::FStrEq( strFile, ".." ) )
			bIsValidFile = false;

		// If we are a root dir.
		if ( bIsRoot && !g_pFullFileSystem->FindIsDirectory( fh ) )
		{
			// From the root, we only allow certain formats
			bIsValidFile = MountingCheck( strFile );
		}
		// If root, and is a dir, make sure we only allow certain folders.
		else if ( bIsRoot && g_pFullFileSystem->FindIsDirectory( fh ) )
		{
			bIsValidFile = false;
			if ( vgui2::FStrEq( strFile, "logos" )
				|| vgui2::FStrEq( strFile, "maps" )
				|| vgui2::FStrEq( strFile, "media" )
				|| vgui2::FStrEq( strFile, "models" )
				|| vgui2::FStrEq( strFile, "resource" )
				|| vgui2::FStrEq( strFile, "sound" )
				|| vgui2::FStrEq( strFile, "ui" )
				|| vgui2::FStrEq( strFile, "gfx" )
				|| vgui2::FStrEq( strFile, "sprites" ) )
				bIsValidFile = true;
		}

		if ( g_pFullFileSystem->FindIsDirectory( fh ) && bIsValidFile )
		{
			char strNewPath[ 4028 ];
			Q_snprintf( strNewPath, sizeof( strNewPath ), "%s%s/", path, strFile );

			// Create our folders, so we can copy the files over!
			std::string strNewPathDir( strNewPath );
			vgui2::STDReplaceString( strNewPathDir, pathRoot, "" );
			g_pFullFileSystem->CreateDirHierarchy( strNewPathDir.c_str(), "ADDON" );

#if defined( _DEBUG )
			// Only show on Debug mode
			ConPrintf(
				Color( 0, 255, 255, 255 ),
				"MountWorkshopItem [%llu]( %s, %s )\n",
				WorkshopFile.uWorkshopID,
			    strNewPath,
			    pathRoot
			);
#endif

			MountWorkshopItem( WorkshopFile, strNewPath, pathRoot );
		}
		else if ( !g_pFullFileSystem->FindIsDirectory( fh ) && bIsValidFile )
		{
			std::string strNewFilePath( std::string( path ) + std::string( strFile ) );
			std::string strNewFilePathDest( strNewFilePath );
			vgui2::STDReplaceString( strNewFilePathDest, pathRoot, "" );
			CopyPath *data = new CopyPath;
			if ( WorkshopFile.bIsWorkshopDownload )
				data->from = "../../workshop/content/3825360/" + strNewFilePath;
			else
				data->from = "zp_workshop/" + strNewFilePath;
			data->to = "zp_addon/" + strNewFilePathDest;
			data->item = WorkshopFile.uWorkshopID;

#if defined( _DEBUG )
			// Only show on Debug mode
			ConPrintf(
				Color( 0, 255, 255, 255 ),
				"CreateSimpleThread [%llu]( %s )\n",
				WorkshopFile.uWorkshopID,
			    !WorkshopFile.bMounted ? "MOUNTING ADDON" : "DELETING ADDON"
			);
#endif

			if ( !WorkshopFile.bMounted )
				CreateSimpleThread( CopyFilesToNewDestination, data );
			else
			{
				DeleteFile *data = new DeleteFile;
				data->file = strNewFilePathDest;
				CreateSimpleThread( RemoveFilesFromAddons, data );
			}
		}

		fn = g_pFullFileSystem->FindNext( fh );
	}
	while( fn );

	SetMountedState( WorkshopFile.uWorkshopID, !WorkshopFile.bMounted );
	RebuiltAddonList();

	g_pFullFileSystem->FindClose( fh );
}

bool CGameUIViewport::HasConflictingFiles( vgui2::WorkshopItem WorkshopFile )
{
	return WorkshopFile.bFoundConflictingFiles;
}

vgui2::WorkshopItem CGameUIViewport::GetWorkshopItem( PublishedFileId_t nWorkshopID )
{
	for ( int iID = 0; iID < m_Items.size(); iID++ )
	{
		vgui2::WorkshopItem &WorkshopAddon = m_Items[iID];
		if ( WorkshopAddon.uWorkshopID == nWorkshopID )
			return WorkshopAddon;
	}
	return vgui2::WorkshopItem();
}

void CGameUIViewport::SetMountedState( PublishedFileId_t nWorkshopID, bool state )
{
	for ( int iID = 0; iID < m_Items.size(); iID++ )
	{
		vgui2::WorkshopItem &WorkshopAddon = m_Items[iID];
		if ( WorkshopAddon.uWorkshopID == nWorkshopID )
			WorkshopAddon.bMounted = state;
	}
}

void CGameUIViewport::SetConflictingFiles( PublishedFileId_t nWorkshopID, bool state )
{
	for ( int iID = 0; iID < m_Items.size(); iID++ )
	{
		vgui2::WorkshopItem &WorkshopAddon = m_Items[iID];
		if ( WorkshopAddon.uWorkshopID == nWorkshopID )
			WorkshopAddon.bFoundConflictingFiles = state;
	}
}

WorkshopInfoBoxState CGameUIViewport::GetWorkshopInfoBoxState()
{
	if ( !m_hWorkshopInfoBox )
		return WorkshopInfoBoxState::State_GatheringData;
	return m_hWorkshopInfoBox->GetState();
}

void CGameUIViewport::ShowWorkshopInfoBox(const char *szText, WorkshopInfoBoxState nState)
{
	if ( !szText ) return;

	if ( !m_hWorkshopInfoBox )
	{
		m_hWorkshopInfoBox = new CCreateWorkshopInfoBox( this );
		m_hWorkshopInfoBox->Activate();
	}

	char buffer[32];
	Q_snprintf( buffer, sizeof( buffer ), "%s", szText );
	if ( szText[0] == '#' )
	{
		wchar_t *pStr = g_pVGuiLocalize->Find( szText );
		if ( pStr )
			g_pVGuiLocalize->ConvertUnicodeToANSI( pStr, buffer, sizeof(buffer) );
	}

	m_hWorkshopInfoBox->SetData( buffer, nState );

	// if nState is State_DownloadingMapContent, move to center.
	if ( nState == State_DownloadingMapContent )
		m_hWorkshopInfoBox->MoveToCenterOfScreen();
}

void CGameUIViewport::SetWorkshopInfoBoxProgress( float flProgress )
{
	if ( !m_hWorkshopInfoBox ) return;
	m_hWorkshopInfoBox->SetProgressState( flProgress );
}

bool CGameUIViewport::WorkshopIDIsMounted( PublishedFileId_t nWorkshopID )
{
	KeyValues *pAddonList = new KeyValues( "AddonList" );
	KeyValuesAD autodel( pAddonList );
	if ( pAddonList->LoadFromFile( g_pFullFileSystem, "addonlist.txt", "WORKSHOP" ) )
	{
		std::string strWorkshopID( std::to_string( nWorkshopID ) );
		return pAddonList->GetBool( strWorkshopID.c_str(), false );
	}
	return false;
}

bool CGameUIViewport::ShouldAutoMount( PublishedFileId_t nWorkshopID )
{
#if defined( _DEBUG )
	// Only show on Debug mode
	ConPrintf(
		Color( 255, 255, 0, 255 ),
		"ShouldAutoMount [%llu]\n",
	    nWorkshopID
	);
#endif
	// We need to reconnect either way, make sure we copy the files over.
	if ( m_bNeedToReconnectAfterDownload ) return true;
	//if ( FindKey( keyName ) )
	KeyValues *pAddonList = new KeyValues( "AddonList" );
	KeyValuesAD autodel( pAddonList );
	if ( pAddonList->LoadFromFile( g_pFullFileSystem, "addonlist.txt", "WORKSHOP" ) )
	{
		bool bShouldMount = false;
		std::string strWorkshopID( std::to_string( nWorkshopID ) );
		KeyValues *pKey = pAddonList->FindKey( strWorkshopID.c_str() );
		if ( !pKey )
			bShouldMount = true;
		else
		{
#if defined( _DEBUG )
			ConPrintf(
				Color( 0, 255, 0, 255 ),
				"[%s => %s]\n",
			    pKey->GetName(),
			    pKey->GetString()
			);
#endif
			if ( vgui2::FStrEq( pKey->GetString(), "1" ) )
				bShouldMount = true;
		}
		if ( bShouldMount )
		{
#if defined( _DEBUG )
			// Only show on Debug mode
			ConPrintf(
				Color( 0, 255, 0, 255 ),
				"ShouldAutoMount [%llu] -- MOUNT\n",
				nWorkshopID
			);
#endif
			return true;
		}
	}
	else
	{
#if defined( _DEBUG )
		// Only show on Debug mode
		ConPrintf(
			Color( 0, 255, 0, 255 ),
			"ShouldAutoMount [%llu] -- AUTO MOUNT\n",
			nWorkshopID
		);
#endif
		return true;
	}
#if defined( _DEBUG )
	// Only show on Debug mode
	ConPrintf(
		Color( 255, 0, 0, 255 ),
		"ShouldAutoMount [%llu] -- NOT\n",
	    nWorkshopID
	);
#endif
	return false;
}

void CGameUIViewport::OpenFileExplorer( int eFilter, const char *szFolder, const char *szPathID, DialogSelected_t pFunction )
{
	g_pFileDialogManager->OpenFileBrowser( eFilter, szFolder, szPathID, pFunction );
}

void CGameUIViewport::OpenFileExplorer( const char *szFolder, const char *szPathID, DialogSelected_t pFunction )
{
	g_pFileDialogManager->OpenFolderBrowser( szFolder, szPathID, pFunction );
}

void CGameUIViewport::ShowMessageDialog( const char *szTitle, const char *szDescription )
{
	vgui2::MessageBox *pMessageBox = new vgui2::MessageBox( szTitle, szDescription, this );
	pMessageBox->SetProportional( true );
	pMessageBox->SetOKButtonVisible( true );
	pMessageBox->SetCancelButtonVisible( false );
	pMessageBox->DoModal();
}

bool CGameUIViewport::IsVACBanned() const
{
	// Have you been a naughty boy?
	return GetSteamAPI()->SteamApps()->BIsVACBanned();
}

void CGameUIViewport::DownloadWorkshopAddon( PublishedFileId_t nWorkshopID, const bool &bReconnect )
{
#if defined( _DEBUG )
	// Only show on Debug mode
	ConPrintf(
		Color( 255, 255, 0, 255 ),
		"DownloadWorkshopAddon | bReconnect: %s\n",
	    bReconnect ? "TRUE" : "FALSE"
	);
#endif
	ConPrintf( Color( 255, 255, 0, 255 ), "[Workshop] Trying to download Workshop Item: %llu\n", nWorkshopID );
	PrepareForDownload data;
	data.IsDownloading = false;
	data.Reconnect = bReconnect;
	data.WorkshopID = nWorkshopID;
	Q_snprintf( data.Title, sizeof( data.Title ), "%llu", nWorkshopID );
	m_QueryRequests.push_back( data );

	// Show the addon we want to mount
	ShowWorkshopInfoBox( data.Title, WorkshopInfoBoxState::State_GatheringData );

	SetQueryWait( 1.15 );
	m_bDownloadedItemsReady = false;
	m_bPrepareForQueryDownload = true;

#if defined( _DEBUG )
	uint32 eState = GetSteamAPI()->SteamUGC()->GetItemState( nWorkshopID );
	Msg( "SteamUGC()->GetItemState( %llu ) Flags:\n", nWorkshopID );
	if ( eState & k_EItemStateSubscribed ) Msg( "\t - k_EItemStateSubscribed\n" );
	if ( eState & k_EItemStateLegacyItem ) Msg( "\t - k_EItemStateLegacyItem\n" );
	if ( eState & k_EItemStateInstalled ) Msg( "\t - k_EItemStateInstalled\n" );
	if ( eState & k_EItemStateNeedsUpdate ) Msg( "\t - k_EItemStateNeedsUpdate\n" );
	if ( eState & k_EItemStateDownloading ) Msg( "\t - k_EItemStateDownloading\n" );
	if ( eState & k_EItemStateDownloadPending ) Msg( "\t - k_EItemStateDownloadPending\n" );
#endif

	if ( !bReconnect ) return;
#if !defined( _DEBUG )
	uint32 eState = GetSteamAPI()->SteamUGC()->GetItemState( nWorkshopID );
#endif
	if ( (eState & k_EItemStateNeedsUpdate) || eState == k_EItemStateNone )
	{
		gEngfuncs.pfnClientCmd( "disconnect\n" );
		ShowWorkshopInfoBox( data.Title, WorkshopInfoBoxState::State_DownloadingMapContent );
	}
}

// ===================================
// Purpose: Mount said content.
// ===================================
void CGameUIViewport::OnSendQueryUGCRequest( SteamUGCQueryCompleted_t *pCallback, bool bIOFailure )
{
	bool bFailed = ( bIOFailure || ( pCallback->m_eResult != k_EResultOK ) );
	if ( bFailed )
	{
#if defined( SPDLOG )
		SpdLog(
			"workshop_client",
			UTIL_CurrentMapLog(),
			LOGTYPE_WARN,
			"Failed to send query. ErrorID: %i",
			pCallback->m_eResult
		);
#else
		ConPrintf( Color( 255, 22, 22, 255 ), "[Workshop] Failed to send query. ErrorID: %i\n", pCallback->m_eResult );
#endif
		GetSteamAPI()->SteamUGC()->ReleaseQueryUGCRequest( handle );
		return;
	}

	for ( size_t i = 0; i < pCallback->m_unNumResultsReturned; i++ )
	{
		// Create it
		SteamUGCDetails_t *pDetails = new SteamUGCDetails_t;

		// Get our info
		if ( GetSteamAPI()->SteamUGC()->GetQueryUGCResult( pCallback->m_handle, i, pDetails ) )
		{
			PrepareForDownload data;
			data.IsDownloading = false;
			data.Reconnect = false;
			data.WorkshopID = pDetails->m_nPublishedFileId;
			Q_snprintf( data.Title, sizeof( data.Title ), "%s", pDetails->m_rgchTitle );
			m_QueryRequests.push_back( data );

			// Show the addon we want to mount
			ShowWorkshopInfoBox( pDetails->m_rgchTitle, WorkshopInfoBoxState::State_GatheringData );
		}

		// Delete it
		if ( pDetails )
			delete pDetails;
	}

	GetSteamAPI()->SteamUGC()->ReleaseQueryUGCRequest( handle );

	SetQueryWait( 1.15 );
	m_bDownloadedItemsReady = false;
	m_bPrepareForQueryDownload = true;
}

void CGameUIViewport::UpdateHTTPCallback( HTTPRequestCompleted_t *arg, bool bFailed )
{
	uint64 context = arg->m_ulContextValue;
	if ( bFailed || arg->m_eStatusCode < 200 || arg->m_eStatusCode > 299 )
	{
		uint32 size;
		GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodySize( arg->m_hRequest, &size );

		if ( size > 0 )
		{
			uint8* pResponse = new uint8[size + 1];
			GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodyData( arg->m_hRequest, pResponse, size );
			pResponse[size] = '\0';

			Msg( "UpdateHTTPCallback: The data hasn't been received. HTTP error %d. Response: %s\n", arg->m_eStatusCode, pResponse );

			delete[] pResponse;
		}
		else if ( !arg->m_bRequestSuccessful )
		{
			// Internet is either not working, or the site is down.
			Msg( "UpdateHTTPCallback: The data hasn't been received. No response from the server.\n");
		}
		else
			Msg( "UpdateHTTPCallback: The data hasn't been received. HTTP error %d\n", arg->m_eStatusCode );
	}
	else
	{
		uint32 size;
		GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodySize( arg->m_hRequest, &size );
		if ( size > 0 )
		{
			uint8* pResponse = new uint8[size + 1];
			GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodyData( arg->m_hRequest, pResponse, size );
			pResponse[size] = '\0';
			switch ( context )
			{
				// Grab our donator status
				case READ_DONATOR_STATUS:
				{
					std::string strResponse((char *)pResponse);
					nlohmann::json root;
					try
					{
						root = nlohmann::json::parse(strResponse.data());

						// Validate the data
					    bool bValid = true;

						// Basic validation of the JSON data, so we don't crash trying to read invalid data.
					    if ( root.at("tier").is_null() )
						    bValid = false;
						else
						{
						    if ( root.at("tier").at("id").is_null() )
							    bValid = false;
						}

						if ( root.at("special").is_null() )
							bValid = false;
						else
						{
							if ( root.at("special").at("key").is_null() )
								bValid = false;
							if ( root.at("special").at("value").is_null() )
								bValid = false;
					    }

						if ( !bValid )
						{
							Msg( "API Callback: Failed to parse JSON data.\n" );
							Msg( "%s\n\n", strResponse.c_str() );
							break;
						}

						// Version 3 of the API uses nested objects for type and tier
						int tier = root.at("tier").at("id").get<int>();
						// Let's also grab the new special value for game specific data.
						int special_key = root.at("special").at("key").get<int>();
						std::string special_value = root.at("special").at("value").get<std::string>();

						// Store the data, which we will send to the server when connecting.
					    g_ClientAPIData.Tier = (eSupporterTier)tier;
						g_ClientAPIData.Game = (eGameAPIVersion)special_key;
					    g_ClientAPIData.Key = special_value;
#if defined( _DEBUG )
					    Msg( "API Callback:\n" );
					    Msg( "\tSteamID: %llu\n", GetSteamAPI()->SteamUser()->GetSteamID().ConvertToUint64() );
					    Msg( "\tGame: %i\n", g_ClientAPIData.Game );
					    Msg( "\tKey: %s\n", g_ClientAPIData.Key.c_str() );
					    Msg( "\tTier: %i\n", g_ClientAPIData.Tier );
#endif
					}
					catch (const std::exception &e)
					{
						Msg( "UpdateHTTPCallback: Failed to parse json: %s\n", e.what() );
					}
				}
				break;
			}
		}
	}

	GetSteamAPI()->SteamHTTP()->ReleaseHTTPRequest( arg->m_hRequest );
}

bool CGameUIViewport::PrepareForQueryDownload()
{
	for ( size_t i = 0; i < m_QueryRequests.size(); i++ )
	{
		PrepareForDownload &data = m_QueryRequests[i];
		if ( data.IsDownloading ) continue;
		data.IsDownloading = true;
		bool bCanDownload = GetSteamAPI()->SteamUGC()->DownloadItem( m_QueryRequests[i].WorkshopID, true );
		if ( bCanDownload )
		{
			ConPrintf( Color( 0, 255, 255, 255 ), "[Workshop] Downloading Workshop Item: %llu\n", data.WorkshopID );
			ShowWorkshopInfoBox( data.Title, WorkshopInfoBoxState::State_Downloading );
			m_CurrentQueryItem = data;
			m_SteamCallbackResultOnDownloadItemResult.Register( this, &CGameUIViewport::OnDownloadItemResult );
			return true;
		}
	}
	return false;
}

bool CGameUIViewport::RequestStats()
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == GetSteamAPI()->SteamUserStats() || NULL == GetSteamAPI()->SteamUser())
		return false;

	// Is the user logged on?  If not we can't get stats.
	if (!GetSteamAPI()->SteamUser()->BLoggedOn())
		return false;

	// Request user stats.
	return GetSteamAPI()->SteamUserStats()->RequestCurrentStats();
}
