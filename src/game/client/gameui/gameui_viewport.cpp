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

bool g_bIsConnected = false;

#if defined(_DEBUG)
CON_COMMAND(gameui_cl_open_test_panel, "Opens a test panel for client GameUI")
{
	CGameUIViewport::Get()->OpenTestPanel();
}
#endif

enum HttpRequestTypes
{
	// Read our donator status from the API.
	READ_DONATOR_STATUS = 0,
};

CGameUIViewport::CGameUIViewport()
    : BaseClass(nullptr, "ClientGameUIViewport"),
	m_CallbackUserStatsReceived( this, &CGameUIViewport::OnUserStatsReceived )
{
	Assert(!m_sInstance);
	m_sInstance = this;

	SetParent(g_pEngineVGui->GetPanel(PANEL_GAMEUIDLL));
	SetScheme(vgui2::scheme()->LoadSchemeFromFile(VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme"));

	SetSize(0, 0);

	m_bPrepareForQueryDownload = false;
	m_hWorkshopInfoBox = nullptr;
	m_flQueryWait = 0.0f;

	LoadWorkshopItems( false );
	LoadWorkshop();

	RequestStats();

	// API call to grab our donators dynamically. We only do this on game start.
	if ( GetSteamAPI()->SteamHTTP() && GetSteamAPI()->SteamUser() )
	{
		SteamAPICall_t pCall = k_uAPICallInvalid;
		HTTPRequestHandle request = GetSteamAPI()->SteamHTTP()->CreateHTTPRequest(
			k_EHTTPMethodGET,
			vgui2::VarArgs( "https://api.wuffesan.com/donators/check/?steamid=%llu&v=2", GetSteamAPI()->SteamUser()->GetSteamID().ConvertToUint64() )
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
				m_flQueryWait = 2.0f;
				ShowWorkshopInfoBox( m_CurrentQueryItem.Title, WorkshopInfoBoxState::State_Done );
				m_CurrentQueryItem.WorkshopID = 0;
			}
			return;
		}

		if ( m_flQueryWait > 0.0f )
		{
			m_flQueryWait -= 0.25f;
			return;
		}

		// Prepare the next query.
		if ( PrepareForQueryDownload() )
			return;

		// Clear it
		m_QueryRequests.clear();

		// Returned false? then stop the query download.
		m_bPrepareForQueryDownload = false;

		// Now check the client workshop content
		LoadWorkshopItems( true );
	}
}

void CGameUIViewport::GetCurrentItems( std::vector<vgui2::WorkshopItem> &items )
{
	items = m_Items;
}

void CGameUIViewport::UpdateAddonList()
{
	// Update addonlist.txt
	KeyValues *pAddonList = new KeyValues( "AddonList" );
	KeyValuesAD autodel( pAddonList );
	if ( pAddonList->LoadFromFile( g_pFullFileSystem, "addonlist.txt", "WORKSHOP" ) )
	{
		for ( int iID = 0; iID < m_Items.size(); iID++ )
		{
			vgui2::WorkshopItem &WorkshopAddon = m_Items[iID];
			std::string strWorkshopID( std::to_string( WorkshopAddon.uWorkshopID ) );
			pAddonList->SetBool( strWorkshopID.c_str(), WorkshopAddon.bMounted );
		}
	}
	// Save the file
	pAddonList->SaveToFile( g_pFullFileSystem, "addonlist.txt", "WORKSHOP" );
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
	// Load our data from zp_workshop
	FileFindHandle_t fh;
	char const *fn = g_pFullFileSystem->FindFirst( "*.*", &fh, bWorkshopFolder ? "WORKSHOPDL" : "WORKSHOP" );
	if ( !fn ) return;
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

void CGameUIViewport::AutoMountWorkshopItem( vgui2::WorkshopItem &WorkshopFile )
{
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

		// If we are a root dir, ignore.
		// We do NOT want to override any CFG files and so on.
		if ( bIsRoot && !g_pFullFileSystem->FindIsDirectory( fh ) )
			bIsValidFile = false;
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
	UpdateAddonList();

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

void CGameUIViewport::ShowWorkshopInfoBox( const char *szText, WorkshopInfoBoxState nState )
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
	pMessageBox->SetOKButtonVisible( true );
	pMessageBox->SetCancelButtonVisible( false );
	pMessageBox->DoModal();
}

bool CGameUIViewport::IsVACBanned() const
{
	// Have you been a naughty boy?
	return GetSteamAPI()->SteamApps()->BIsVACBanned();
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

	m_flQueryWait = 1.15f;
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

			ConPrintf( "UpdateHTTPCallback: The data hasn't been received. HTTP error %d. Response: %s\n", arg->m_eStatusCode, pResponse );

			delete[] pResponse;
		}
		else if ( !arg->m_bRequestSuccessful )
		{
			// Internet is either not working, or the site is down.
			ConPrintf( "UpdateHTTPCallback: The data hasn't been received. No response from the server.\n");
		}
		else
			ConPrintf( "UpdateHTTPCallback: The data hasn't been received. HTTP error %d\n", arg->m_eStatusCode );
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
						gEngfuncs.PlayerInfo_SetValueForKey( "donor_type", vgui2::VarArgs( "%i", root.at("type").get<int>() ) );
						gEngfuncs.PlayerInfo_SetValueForKey( "donor_tier", vgui2::VarArgs( "%i", root.at("tier").get<int>() ) );
					}
					catch (const std::exception &e)
					{
						ConPrintf( "UpdateHTTPCallback: Failed to parse json: %s\n", e.what() );
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
			ShowWorkshopInfoBox( data.Title, WorkshopInfoBoxState::State_Downloading );
			m_CurrentQueryItem = data;
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
