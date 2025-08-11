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

	// TODO: Add a refresh button.

	// Load this last, so we can move our items around.
	LoadControlSettings( VGUI2_ROOT_DIR "resource/workshop/uploaded.res" );

	vgui2::ivgui()->AddTickSignal( GetVPanel(), 25 );

	if ( !GetSteamAPI() ) return;
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

	char buffer[158];
	Q_snprintf( buffer, sizeof( buffer ), "%llu/thumb", item.uWorkshopID );

	pIcon->SetImage( vgui2::scheme()->GetImage( buffer, false ) );
	pIcon->SetSize( 56, 56 );
	pIcon->SetPos( 4, 4 );
	pIcon->SetFillColor( Color( 25, 25, 25, 150 ) );

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

			// Load the file, and save to TGA under thumb_<num>.tga
			char szURL[1024];
			if ( GetSteamAPI()->SteamUGC()->GetQueryUGCPreviewURL( pDetails->m_hPreviewFile, i, szURL, sizeof( szURL ) ) )
			{
				SteamAPICall_t pCall = k_uAPICallInvalid;

				// Download the URL, and save it as a file.
				HTTPRequestHandle httphandle = GetSteamAPI()->SteamHTTP()->CreateHTTPRequest( EHTTPMethod::k_EHTTPMethodGET, szURL );
				GetSteamAPI()->SteamHTTP()->SetHTTPRequestHeaderValue( httphandle, "Cache-Control", "no-cache");
				GetSteamAPI()->SteamHTTP()->SetHTTPRequestContextValue( httphandle, pDetails->m_nPublishedFileId );
				GetSteamAPI()->SteamHTTP()->SendHTTPRequest( httphandle, &pCall );
				m_SteamCallResultOnHTTPRequest.Set( pCall, this, &CWorkshopSubUploaded::UpdateHTTPCallback );
			}
		}

		// Delete it
		if ( pDetails )
			delete pDetails;
	}

	GetSteamAPI()->SteamUGC()->ReleaseQueryUGCRequest( handle );
}

void CWorkshopSubUploaded::UpdateHTTPCallback( HTTPRequestCompleted_t *arg, bool bFailed )
{
	ConPrintf( "void CWorkshopSubUploaded::UpdateCallback()\n" );
	ConPrintf( "STATUS CODE [%i]\n", arg->m_eStatusCode );
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
		ConPrintf( "WorkshopID Preview [%llu]\n", context );
		uint32 size;
		GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodySize( arg->m_hRequest, &size );

		if ( size > 0 )
		{
			ConPrintf( "Size Returned: [%i]\n", size );
			uint8* pResponse = new uint8[size + 1];
			GetSteamAPI()->SteamHTTP()->GetHTTPResponseBodyData( arg->m_hRequest, pResponse, size );
			pResponse[size] = '\0';

			// Make sure the folder exist.
			g_pFullFileSystem->CreateDirHierarchy( "uploads", "WORKSHOP" );

			// Write the file
			FileHandle_t file_h = g_pFullFileSystem->Open( vgui2::VarArgs( "uploads/thumb_%llu.jpg", context ), "w", "WORKSHOP" );

			// Save the contents to the file
			g_pFullFileSystem->Write( pResponse, size, file_h );

			// Close the file after use
			g_pFullFileSystem->Close( file_h );

			// Now convert the fucker to TGA
			//ilImage Image( vgui2::VarArgs( "zp_workshop/uploads/thumb_%llu.jpg", context ) );
			//ilEnable( IL_FILE_OVERWRITE );
			//Image.Save( vgui2::VarArgs( "zp_workshop/uploads/thumb_%llu.tga", context ) );

			delete[] pResponse;
		}
	}
	GetSteamAPI()->SteamHTTP()->ReleaseHTTPRequest( arg->m_hRequest );
}

CWorkshopSubUploaded::WorkshopItem CWorkshopSubUploaded::GetWorkshopItem( PublishedFileId_t nWorkshopID )
{
	for ( size_t i = 0; i < m_Items.size(); i++ )
	{
		WorkshopItem item = m_Items[i];
		if ( item.PublishedFileID == nWorkshopID )
			return item;
	}
	return WorkshopItem();
}
