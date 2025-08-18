//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include "C_PlayerSelection.h"
#include "client_vgui.h"
#include "gameui/gameui_viewport.h"
#include "IBaseUI.h"

#include <vgui/IVGui.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/RichText.h>

#include <filesystem.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar cl_character;

C_PlayerSelection::C_PlayerSelection(vgui2::Panel *pParent)
    : BaseClass(pParent, "PlayerSelection")
{
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(true);
	SetSizeable(false);
	SetMoveable(true);
	SetVisible(true);
	
	// KeyValues
	kvPlayerData = new KeyValues("PlayerModels");
	
	// Set Our model
	ui_SelectPlayerModel = new vgui2::ComboBox(this, "player_select", 1, false);

	// Random [ID 0]
	ui_SelectPlayerModel->ActivateItem( 0 );
	iHasDescription = 0;

	// Add random model
	AddPlayerOption( "random", "#ZP_UI_Char_Random", "random", "random" );
	
	vgui2::HScheme hScheme = vgui2::scheme()->LoadSchemeFromFile( VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme" );

	SetScheme( hScheme );

	LoadControlSettings( VGUI2_ROOT_DIR "resource/zps/playerselection.res");

	// Get our current language
	strLocalizationString = g_pVGuiLocalize->GetLocalizationFileName(0);
	// Take our filename, and remove everything from start to _
	// and then we remove the .txt at the end, then we get our localization name.
	// We are kinda limited on what to do without engine access,
	// so this is the best hack possibly at this very moment.
	size_t nLocalizationPos = strLocalizationString.find( '_' );
	if ( nLocalizationPos > 0 )
	{
		strLocalizationString = strLocalizationString.substr( nLocalizationPos + 1, strLocalizationString.size() );
		nLocalizationPos = strLocalizationString.find( "." );
		if ( nLocalizationPos > 0 )
			strLocalizationString = strLocalizationString.substr( 0, nLocalizationPos );
	}

	SetTitle( "#ZP_UI_PlayerSelection", true);

	Panel *pPanel = FindChildByName( "Info" );
	vgui2::Label *pInfo = (vgui2::Label *)pPanel;
	if ( pInfo )
		pInfo->SetText( "#ZP_UI_PlayerSelection_Info" );

	pPanel = FindChildByName( "SurvivorBio" );
	vgui2::Label *pSurvivorBio = (vgui2::Label *)pPanel;
	if ( pSurvivorBio )
		pSurvivorBio->SetText( "#ZP_UI_PlayerSelection_Bio" );

	// OnTick
	//======================================
	//======================================
	vgui2::ivgui()->AddTickSignal( GetVPanel() );

	// Survivors
	SetupAvailablePlayerModels();

	// Set the new amount of selections
	ui_SelectPlayerModel->SetNumberOfEditLines( m_playermodels.Count() >= 8 ? 8 : m_playermodels.Count() );

	// Set the position
	ui_SelectPlayerModel->SetPos(16, 264);
	ui_SelectPlayerModel->SetSize(190, 24);

	// Set our player model avatar
	ui_SelectPlayerAvatar = new vgui2::ImagePanel(this, "player_avatar");
	ui_SelectPlayerAvatar->SetPos(17, 31);
	ui_SelectPlayerAvatar->SetSize(190, 227);
	ui_SelectPlayerAvatar->SetShouldScaleImage(true);

	// Before we set the default image, lets check our current model
	const char *szModelName = NULL;
	szModelName = cl_character.GetString();

	for ( int iModel = 0; iModel < m_playermodels.Count(); iModel++ )
	{
		PLAYERMODELS &plMdl = m_playermodels[ iModel ];
		if ( vgui2::FStrEq( plMdl.strType, szModelName ) )
		{
			ui_SelectPlayerModel->ActivateItem( iModel );
			break;
		}
	}
	
	// Label
	//======================================
	//======================================
	ui_Label_pagenum = new vgui2::Label(this, "Character_bio_page", "Page (X/Y)");
	ui_Label_pagenum->SetPos( 745, 445 );
	ui_Label_pagenum->SetSize( 90, 24 );

	// Buttons
	//======================================
	//======================================
	ui_Bttn_Back = new vgui2::Button(this, "page_back", "#ZP_UI_Back");
	ui_Bttn_Back->SetPos( 734, 473 );
	ui_Bttn_Back->SetSize( 42, 24 );
	ui_Bttn_Back->SetEnabled( false );
	ui_Bttn_Back->SetCommand( "previous_page" );

	ui_Bttn_Next = new vgui2::Button(this, "page_next", "#ZP_UI_Next");
	ui_Bttn_Next->SetPos( 781, 473 );
	ui_Bttn_Next->SetSize( 42, 24 );
	ui_Bttn_Next->SetEnabled( false );
	ui_Bttn_Next->SetCommand( "next_page" );

	// Page Number
	//======================================
	//======================================
	iPageNum = 0;
	iMaxPages = 0;

	bReadPages = false;
	
	vgui2::IScheme *pScheme = vgui2::scheme()->GetIScheme( hScheme );
	vgui2::HFont hFont = pScheme->GetFont( "Bio" );

	// Setup survivor BIO
	ui_SelectPlayerBio = new vgui2::Label(this, "Character_bio", "If you read this, it failed to read the bio...");
	ui_SelectPlayerBio->SetPos( 224, 72 );
	ui_SelectPlayerBio->SetSize( 600, 400 );
	ui_SelectPlayerBio->SetFont( hFont );
	ui_SelectPlayerBio->SetWrap( true );
	ui_SelectPlayerBio->SetContentAlignment( vgui2::Label::Alignment::a_northwest );
	
	V_strcpy_safe( strCurrentBio, "random" );
	LoadPageInfo( "random", 0 );

	MoveToCenterOfScreen();
}

void C_PlayerSelection::LoadPageInfo( const char *strBio, int pagenum )
{
	static char strModelBio[ MAX_PATH ];
	strModelBio[0] = 0;

	V_strcpy_safe( strModelBio, "scripts/bios/" );
	V_strcat_safe( strModelBio, strLocalizationString.c_str() );
	V_strcat_safe( strModelBio, "/" );
	V_strcat_safe( strModelBio, strBio );
	V_strcat_safe( strModelBio, ".txt" );

	KeyValues *kvPage = new KeyValues( "BioFile" );
	// If it fails, read the default english one
	if ( !kvPage->LoadFromFile( g_pFullFileSystem, strModelBio, "GAME" ) )
	{
		// Reset
		strModelBio[0] = 0;
		V_strcpy_safe( strModelBio, "scripts/bios/english/" );
		V_strcat_safe( strModelBio, strBio );
		V_strcat_safe( strModelBio, ".txt" );
		kvPage->LoadFromFile( g_pFullFileSystem, strModelBio, "GAME" );
	}

	char prefix[ 512 ];
	prefix[0] = 0;
	Q_snprintf( prefix, sizeof( prefix ), "page_%i", pagenum );

	if ( kvPage )
	{
		KeyValues* pFileSystemInfo = kvPage->FindKey( prefix );

		if ( !pFileSystemInfo )
		{
			kvPage->deleteThis();
			return;
		}

		for ( KeyValues *pKey = pFileSystemInfo->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
		{
			if ( vgui2::FStrEq( pKey->GetName(), "Label" ) )
				ui_SelectPlayerBio->SetText( pKey->GetString() );
		}

		// Count the pages
		if ( !bReadPages )
		{
			// Reset
			iMaxPages = 0;
			for ( KeyValues *pKey = kvPage->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
			{
				if ( Q_stristr( pKey->GetName(), "page_" ) != 0 )
					iMaxPages++;
			}
		}

		bReadPages = true;
	}

	kvPage->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PlayerSelection::SetupPages()
{
	wchar_t *pString = g_pVGuiLocalize->Find( "ZP_UI_Page" );
	// So we don't crash
	if ( !pString ) pString = L"Pages (%s1/%s2)";
	wchar_t wcPageNum[32];
	wchar_t wcMaxPages[32];
	V_snwprintf( wcPageNum, Q_ARRAYSIZE(wcPageNum), L"%i", iPageNum + 1 );
	V_snwprintf( wcMaxPages, Q_ARRAYSIZE(wcMaxPages), L"%i", iMaxPages );
	wchar_t output[128];
	g_pVGuiLocalize->ConstructString(
		output, sizeof(output),
		g_pVGuiLocalize->Find( "ZP_UI_Page" ), 2,
	    wcPageNum,
	    wcMaxPages
	);
	ui_Label_pagenum->SetText( output );
}

void C_PlayerSelection::AddPlayerOption( const char* strPrefix, const char* strModelName, const char* strModelImage, const char* strModelBio )
{
	for ( int iModel = 0; iModel < m_playermodels.Count(); iModel++ )
	{
		PLAYERMODELS &plyMdl = m_playermodels[ iModel ];

		if ( vgui2::FStrEq( plyMdl.strType, strPrefix ) )
		{
			Warning("model \"%s\" already exist!\n", strPrefix);
			return;
		}
	}

	int ModelID = m_playermodels.AddToTail();
	PLAYERMODELS &plyMdl = m_playermodels[ ModelID ];
	V_strcpy_safe( plyMdl.strType, strPrefix );
	V_strcpy_safe( plyMdl.strName, strModelName );
	V_strcpy_safe( plyMdl.strImage, strModelImage );
	V_strcpy_safe( plyMdl.strBio, strModelBio );
	
	// Add to selection
	ui_SelectPlayerModel->AddItem( strModelName, kvPlayerData );
}

void C_PlayerSelection::SetupAvailablePlayerModels()
{
	// Setup the path
	char strFile[ 2048 ];
	strFile[0] = 0;

	// Add our path in
	strcpy_s( strFile, "scripts/survivors.txt" );

	// Setup our search
	KeyValues *manifest = new KeyValues( "Survivors" );
	if ( !manifest->LoadFromFile( g_pFullFileSystem, strFile, "GAME" ) )
		return;

	for ( KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
	{
		KeyValues *subkey = sub;
		if ( subkey )
			LoadSurvivorInfo( sub->GetName(), subkey );
	}

	manifest->deleteThis();
}

void C_PlayerSelection::LoadSurvivorInfo( const char* strPrefix, KeyValues *kvSurvivorInfo )
{
	const char *pstrName = kvSurvivorInfo->GetString( "name", "Eugene Grant" );
	const char *pstrPicture = kvSurvivorInfo->GetString( "picture", "survivor1" );
	const char *pstrBio = kvSurvivorInfo->GetString( "biofile", "eugene" );
	AddPlayerOption( strPrefix, pstrName, pstrPicture, pstrBio );
}

void C_PlayerSelection::OnTick()
{
	BaseClass::OnTick();
	
	if ( iHasDescription != ui_SelectPlayerModel->GetActiveItem() )
	{
		static char url[ MAX_PATH ];
		static char strModelImage[ MAX_PATH ];
		static char strModelType[ MAX_PATH ];
		static char strModelBio[ MAX_PATH ];

		url[0] = 0;
		strModelImage[0] = 0;
		strModelType[0] = 0;
		strModelBio[0] = 0;

		for ( int iModel = 0; iModel < m_playermodels.Count(); iModel++ )
		{
			PLAYERMODELS &plMdl = m_playermodels[ iModel ];
			if ( iModel == ui_SelectPlayerModel->GetActiveItem() )
			{
				// Bio
				V_strcpy_safe( strModelBio, plMdl.strBio );

				// Image
				V_strcpy_safe( strModelImage, "ui/player_selection/" );
				V_strcat_safe( strModelImage, plMdl.strImage );

				// Type
				V_strcpy_safe( strModelType, plMdl.strType );
				break;
			}
		}

		bReadPages = false;

		V_strcpy_safe( strCurrentBio, strModelBio );
		iPageNum = 0;

		cl_character.SetValue( strModelType );
		ui_SelectPlayerAvatar->SetImage( vgui2::scheme()->GetImage( strModelImage, false ) );

		iHasDescription = ui_SelectPlayerModel->GetActiveItem();
	}
	
	if ( iMaxPages > 0 )
	{
		if ( iPageNum >= iMaxPages - 1 )
			ui_Bttn_Next->SetEnabled( false );
		else
			ui_Bttn_Next->SetEnabled( true );
	}
	
	if ( iPageNum <= 0 )
		ui_Bttn_Back->SetEnabled( false );
	else
		ui_Bttn_Back->SetEnabled( true );
	
	SetupPages();
	LoadPageInfo( strCurrentBio, iPageNum );
}

CON_COMMAND(gameui_playerselection, "Opens Achievements dialog")
{
	// Since this command is called from game menu using "engine gameui_achievements"
	// GameUI will hide itself and show the game.
	// We need to show it again and after that activate C_AchievementDialog
	// Otherwise it may be hidden by the dev console
	gHUD.CallOnNextFrame([]() {
		CGameUIViewport::Get()->GetPlayerSelection()->Activate();
	});
	g_pBaseUI->ActivateGameUI();
};

void C_PlayerSelection::OnCommand(const char* pcCommand)
{
	if (!Q_stricmp(pcCommand, "set_model"))
	{
		OnCommand("Close");
	}
	else if ( !Q_stricmp( pcCommand, "next_page" ) )
	{
		LoadPageInfo( strCurrentBio, iPageNum++ );
	}
	else if ( !Q_stricmp( pcCommand, "previous_page" ) )
	{
		LoadPageInfo( strCurrentBio, iPageNum-- );
	}
	else
	{
		BaseClass::OnCommand(pcCommand);
	}
}
