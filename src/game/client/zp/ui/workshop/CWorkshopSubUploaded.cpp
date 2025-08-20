#include <string>
#include <vector>
#include <FileSystem.h>
#include <tier1/strtools.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertySheet.h>
#include <KeyValues.h>
#include <appversion.h>
#include <bhl_urls.h>
#include "client_vgui.h"
#include "gameui/gameui_viewport.h"
#include "CWorkshopSubUploaded.h"
#include "CWorkshopSubUpload.h"

#include <iostream>

// Spectra, PNG > TGA
#include "stb_image.h"			// Loading File
#include "stb_image_write.h"	// Writing TGA

CWorkshopSubUploaded::CWorkshopSubUploaded(vgui2::Panel *parent)
    : BaseClass(parent, "WorkshopSubUploaded")
{
	SetSize(100, 100); // Silence "parent not sized yet" warning

	// Our Item List
	pList = new vgui2::WorkshopItemList( this, "listpanel" );
	pList->SetPos( 15, 100 );
	pList->SetSize( 600, 302 );
	pList->AddActionSignalTarget( this );

	// Load this last, so we can move our items around.
	LoadControlSettings( VGUI2_ROOT_DIR "resource/workshop/uploaded.res" );

	vgui2::ivgui()->AddTickSignal( GetVPanel(), 25 );

	RefreshItems();
}

CWorkshopSubUploaded::~CWorkshopSubUploaded()
{
}

void CWorkshopSubUploaded::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CWorkshopSubUploaded::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CWorkshopSubUploaded::OnCommand(const char *pcCommand)
{
	if ( !Q_stricmp( pcCommand, "Refresh" ) )
		RefreshItems();
	else
		BaseClass::OnCommand( pcCommand );
}

void CWorkshopSubUploaded::RefreshItems()
{
	if ( !GetSteamAPI() ) return;

	Panel *pPanel = FindChildByName( "Refresh" );
	vgui2::Button *pInfo = (vgui2::Button *)pPanel;
	if ( pInfo )
		pInfo->SetEnabled( false );

	if ( GetSteamAPI()->SteamUGC() && GetSteamAPI()->SteamUser() )
	{
		handle = GetSteamAPI()->SteamUGC()->CreateQueryUserUGCRequest(
			GetSteamAPI()->SteamUser()->GetSteamID().GetAccountID(),
		    k_EUserUGCList_Published,
			k_EUGCMatchingUGCType_Items_ReadyToUse,
			k_EUserUGCListSortOrder_LastUpdatedDesc,
		    (AppId_t)3825360, (AppId_t)3825360, 1
		);
		//GetSteamAPI()->SteamUGC()->SetReturnAdditionalPreviews( handle, true );
		GetSteamAPI()->SteamUGC()->SetReturnChildren( handle, true );
		SteamAPICall_t apiCall = GetSteamAPI()->SteamUGC()->SendQueryUGCRequest( handle );
		m_SteamCallResultOnSendQueryUGCRequest.Set( apiCall, this, &CWorkshopSubUploaded::OnSendQueryUGCRequest );
	}
}

void CWorkshopSubUploaded::UpdateAddonPreview( PublishedFileId_t nWorkshopID )
{
	vgui2::ImagePanel *pIcon = (vgui2::ImagePanel *)pList->GetItemPanel( nWorkshopID );
	if ( !pIcon ) return;
	char buffer[158];
	Q_snprintf( buffer, sizeof( buffer ), "thumb_%llu", nWorkshopID );
	pIcon->SetImage( vgui2::scheme()->GetImage( buffer, false ) );
}

void CWorkshopSubUploaded::OnWorkshopEdit( uint64 workshopID )
{
	if ( !pProperty ) return;
	if ( !pProperty->GetPropertySheet() ) return;
	pProperty->GetPropertySheet()->ChangeActiveTab( 2 );
	pUploadPage->SetUpdating( workshopID );

	WorkshopItem item = GetWorkshopItem( workshopID );
	pUploadPage->SetUploadData( item.Title, item.Desc, item.Tags, item.Visibility );
}

void CWorkshopSubUploaded::AddItem( vgui2::WorkshopItem item )
{
	// Fonts
	vgui2::HFont hTextFont;
	vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme(
		vgui2::scheme()->LoadSchemeFromFile( VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme" )
	);

	// Create Image
	vgui2::ImagePanel *pIcon = new vgui2::ImagePanel( this, "Icon" );

	pIcon->SetImage( vgui2::scheme()->GetImage( "thumb_unknown", false ) );
	pIcon->SetSize( 56, 56 );
	pIcon->SetPos( 4, 4 );
	pIcon->SetFillColor( Color( 25, 25, 25, 150 ) );
	pIcon->SetShouldScaleImage( true );

	// Font Text
	vgui2::Label *pTitle = new vgui2::Label( this, "Title", "" );
	pTitle->SetSize( 400, 20 );
	pTitle->SetPos( 70, 5 );
	pTitle->SetPaintBackgroundEnabled( false );
	hTextFont = pScheme->GetFont( "AchievementItemTitle" );
	if ( hTextFont != vgui2::INVALID_FONT )
		pTitle->SetFont( hTextFont );
	pTitle->SetColorCodedText( item.szName );

	vgui2::Label *pAuthor = new vgui2::Label( this, "Author", "" );
	pAuthor->SetSize( 400, 20 );
	pAuthor->SetPos( 70, 35 );
	pAuthor->SetPaintBackgroundEnabled( false );
	hTextFont = pScheme->GetFont( "AchievementItemDescription" );
	if ( hTextFont != vgui2::INVALID_FONT )
		pAuthor->SetFont( hTextFont );
	pAuthor->SetColorCodedText( item.szAuthor );

	vgui2::Label *pDesc = new vgui2::Label( this, "Description", "" );
	pDesc->SetSize( 400, 20 );
	pDesc->SetPos( 70, 15 );
	pDesc->SetPaintBackgroundEnabled( false );
	hTextFont = pScheme->GetFont( "AchievementItemDescription" );
	if ( hTextFont != vgui2::INVALID_FONT )
		pDesc->SetFont( hTextFont );
	pDesc->SetColorCodedText( item.szDesc );

	pList->AddItem(
	    pIcon, pAuthor,
		pTitle, pDesc,
	    nullptr, nullptr,
	    item.uWorkshopID, true
	);

	// Same fix from CWorkshopSubList, this got the same problem.
	pList->InvalidateLayout(true);
	pList->Repaint();
}

void CWorkshopSubUploaded::OnSendQueryUGCRequest( SteamUGCQueryCompleted_t *pCallback, bool bIOFailure )
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
		ConPrintf( Color( 255, 22, 22, 255 ), "[Workshop] Failed to retrieve uploaded addon. ErrorID: %i\n", pCallback->m_eResult );
#endif
		GetSteamAPI()->SteamUGC()->ReleaseQueryUGCRequest( handle );

		Panel *pPanel = FindChildByName( "Refresh" );
		vgui2::Button *pInfo = (vgui2::Button *)pPanel;
		if ( pInfo )
			pInfo->SetEnabled( true );

		return;
	}

	pList->DeleteAllItems();

	for ( size_t i = 0; i < pCallback->m_unNumResultsReturned; i++ )
	{
		// Create it
		SteamUGCDetails_t *pDetails = new SteamUGCDetails_t;

		// Get our info
		if ( GetSteamAPI()->SteamUGC()->GetQueryUGCResult( pCallback->m_handle, i, pDetails ) )
		{
			vgui2::WorkshopItem WorkshopAddon;
			Q_snprintf( WorkshopAddon.szName, sizeof( WorkshopAddon.szName ), "%s", pDetails->m_rgchTitle );
			Q_snprintf( WorkshopAddon.szDesc, sizeof( WorkshopAddon.szDesc ), "%s", pDetails->m_rgchDescription );
			Q_snprintf( WorkshopAddon.szAuthor, sizeof( WorkshopAddon.szAuthor ), "%s", GetSteamAPI()->SteamFriends()->GetPersonaName() );
			WorkshopAddon.iFilterFlag = 0;
			WorkshopAddon.bMounted = false;
			WorkshopAddon.bIsWorkshopDownload = false;
			WorkshopAddon.bFoundConflictingFiles = false;
			WorkshopAddon.uWorkshopID = pDetails->m_nPublishedFileId;

			// Save our data, which we will use strictly for pUploadPage
			WorkshopItem data;
			Q_snprintf( data.Title, sizeof( data.Title ), "%s", pDetails->m_rgchTitle );
			Q_snprintf( data.Desc, sizeof( data.Desc ), "%s", pDetails->m_rgchDescription );
			Q_snprintf( data.Tags, sizeof( data.Tags ), "%s", pDetails->m_rgchTags );
			data.Visibility = pDetails->m_eVisibility;
			data.PublishedFileID = pDetails->m_nPublishedFileId;
			m_Items.push_back( data );

			AddItem( WorkshopAddon );

			// Grab the preview image, and download it.
			char szURL[1024];
			if ( GetSteamAPI()->SteamUGC()->GetQueryUGCPreviewURL( pCallback->m_handle, i, szURL, sizeof( szURL ) ) )
			{
				m_bCanDownloadPreview = true;
				DownloadPreviewImage dlImage;
				Q_snprintf( dlImage.URL, sizeof( dlImage.URL ), "%s", szURL );
				dlImage.WorkshopID = pDetails->m_nPublishedFileId;
				m_DownloadPreviewImages.push_back( dlImage );
			}
		}

		// Delete it
		if ( pDetails )
			delete pDetails;
	}

	Panel *pPanel = FindChildByName( "Refresh" );
	vgui2::Button *pInfo = (vgui2::Button *)pPanel;
	if ( pInfo )
		pInfo->SetEnabled( true );

	GetSteamAPI()->SteamUGC()->ReleaseQueryUGCRequest( handle );
}

void CWorkshopSubUploaded::UpdateHTTPCallback( HTTPRequestCompleted_t *arg, bool bFailed )
{
	uint64 context = arg->m_ulContextValue;
	if ( bFailed || arg->m_eStatusCode < 200 || arg->m_eStatusCode > 299 )
	{
		ConPrintf( "Callback failed.\n" );
		uint32 size;
		GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodySize( arg->m_hRequest, &size );

		if ( size > 0 )
		{
			uint8* pResponse = new uint8[size + 1];
			GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodyData( arg->m_hRequest, pResponse, size );
			pResponse[size] = '\0';

			std::string strResponse((char*)pResponse);
			ConPrintf(
				Color( 255, 5, 5, 255 ),
				"The data hasn't been received. HTTP error (%i) with the response (%s)\n",
			    arg->m_eStatusCode,
			    strResponse.c_str()
			);

			delete[] pResponse;
		}
		else if ( !arg->m_bRequestSuccessful )
			ConPrintf(
				Color( 255, 5, 5, 255 ),
				"The data hasn't been received. No response from the server.\n"
			);
		else
			ConPrintf(
				Color( 255, 5, 5, 255 ),
				"The data hasn't been received. HTTP error %i\n",
			    arg->m_eStatusCode
			);
	}
	else if ( context > 0 )
	{
		uint32 size;
		GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodySize( arg->m_hRequest, &size );

		if ( size > 0 )
		{
			uint8* pResponse = new uint8[size + 1];
			GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodyData( arg->m_hRequest, pResponse, size );
			pResponse[size] = '\0';

			// Make sure the folder exist.
			g_pFullFileSystem->CreateDirHierarchy( vgui2::VarArgs( "temp/%llu", context ), "WORKSHOP" );

			// Write a temp file, just so we can get our local path.
			FileHandle_t file_h = g_pFullFileSystem->Open( vgui2::VarArgs( "temp/%llu/thumb_%llu", context, context ), "w", "WORKSHOP" );
			g_pFullFileSystem->Write( pResponse, size, file_h );
			g_pFullFileSystem->Close( file_h );

			char szFullPath[1024];
			g_pFullFileSystem->GetLocalPath( vgui2::VarArgs( "temp/%llu/thumb_%llu", context, context ), szFullPath, sizeof( szFullPath ) );

			// Now convert the fucker to TGA
			int width, height, channels;
			// !!! Change this to full path !!!
			unsigned char *data = stbi_load_from_memory( pResponse, size, &width, &height, &channels, 4 );
			if ( data )
			{
				// !!! Change this to full path !!!
				int success = stbi_write_tga( vgui2::VarArgs( "%s.tga", szFullPath ), width, height, 4, data );
				if ( !success )
					ConPrintf( "Failed to write tga\n" );
				stbi_image_free( data );
			}
			else
				ConPrintf( "Failed to load the image!\nReason: %s\n", stbi_failure_reason() );

			g_pFullFileSystem->RemoveFile( vgui2::VarArgs( "temp/%llu/thumb_%llu", context, context ), "WORKSHOP" );

			// Add our path, so our image gets read.
			char buffer[158];
			Q_snprintf( buffer, sizeof( buffer ), "zp_workshop/temp/%llu", context );
			// DO NOT UNCOMMENT, WILL CRASH THE GAME
			// The issue lies with valve's filesystem code,
			// and I doubt they will ever fix this.
			// We could try RemoveAllSearchPaths, but then
			// we need to rebuild all paths.
			//g_pFullFileSystem->RemoveSearchPath( buffer );
			g_pFullFileSystem->AddSearchPathNoWrite( buffer, "WORKSHOP" );

			// Update the preview image
			UpdateAddonPreview( context );

			delete[] pResponse;
		}
	}
	GetSteamAPI()->SteamHTTP()->ReleaseHTTPRequest( arg->m_hRequest );
	m_bCanDownloadPreview = true;
}

void CWorkshopSubUploaded::CheckForPreviewDownload()
{
	if ( m_DownloadPreviewImages.size() < 1 ) return;
	if ( !m_bCanDownloadPreview ) return;
	DownloadPreviewImage dlImage = m_DownloadPreviewImages[0];
	m_bCanDownloadPreview = false;

	SteamAPICall_t pCall = k_uAPICallInvalid;

	// Download the URL, and save it as a file.
	HTTPRequestHandle httphandle = GetSteamAPI()->SteamHTTP()->CreateHTTPRequest( EHTTPMethod::k_EHTTPMethodGET, dlImage.URL );
	GetSteamAPI()->SteamHTTP()->SetHTTPRequestHeaderValue( httphandle, "Cache-Control", "no-cache" );
	GetSteamAPI()->SteamHTTP()->SetHTTPRequestContextValue( httphandle, dlImage.WorkshopID );
	GetSteamAPI()->SteamHTTP()->SendHTTPRequest( httphandle, &pCall );
	m_SteamCallResultOnHTTPRequest.Set( pCall, this, &CWorkshopSubUploaded::UpdateHTTPCallback );

	m_DownloadPreviewImages.erase( m_DownloadPreviewImages.begin() );
}

void CWorkshopSubUploaded::OnTick()
{
	BaseClass::OnTick();
	CheckForPreviewDownload();
}

CWorkshopSubUploaded::WorkshopItem CWorkshopSubUploaded::GetWorkshopItem(PublishedFileId_t nWorkshopID)
{
	for ( size_t i = 0; i < m_Items.size(); i++ )
	{
		WorkshopItem item = m_Items[i];
		if ( item.PublishedFileID == nWorkshopID )
			return item;
	}
	return WorkshopItem();
}
