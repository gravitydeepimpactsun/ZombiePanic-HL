//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include "C_ZPCredits_Sub_Contrib.h"
#include "client_vgui.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <stdio.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define DATAFILE "scripts/credits.txt"

using namespace vgui2;

C_ZPCredits_Sub_Contrib::C_ZPCredits_Sub_Contrib(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	SetSize(100, 100); // Silence "parent not sized yet" warning

	// Label
	//======================================
	//======================================
	ui_Label_contrib1 = new vgui2::Label(this, "Label1", "");
	ui_Label_contrib2 = new vgui2::Label(this, "Label2", "");
	ui_Label_contrib3 = new vgui2::Label(this, "Label3", "");
	ui_Label_pagenum = new vgui2::Label(this, "Label4", "Page (0/0)");

	// Buttons
	//======================================
	//======================================
	ui_Bttn_Back = new Button(this, "page_back", "");
	ui_Bttn_Next = new Button(this, "page_next", "");

	// Setup Keyvalues
	//======================================
	//======================================
	CreatePageInfo();

	// Page Number
	//======================================
	//======================================
	iPageNum = 0;
	iMaxPages = 0;

	// Control Setting
	//======================================
	//======================================
	LoadControlSettings( VGUI2_ROOT_DIR "resource/zps/credits/sub_contrib.res");

	ui_Bttn_Back->SetText("#GameUI_GameMenu_Back");
	ui_Bttn_Next->SetText("#GameUI_GameMenu_Next");

	// OnTick
	//======================================
	//======================================
	ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_ZPCredits_Sub_Contrib::~C_ZPCredits_Sub_Contrib()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ZPCredits_Sub_Contrib::CreatePageInfo()
{
	kvPage = new KeyValues( "GameCredits" );
	kvPage->LoadFromFile( g_pFullFileSystem, DATAFILE, "GAME" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ZPCredits_Sub_Contrib::LoadPageInfo( int pagenum )
{
	char prefix[ 512 ];
	prefix[0] = 0;
	Q_snprintf( prefix, sizeof( prefix ), "page_%i", pagenum );

	if ( kvPage )
	{
		KeyValues* pFileSystemInfo = kvPage->FindKey( prefix );

		if ( !pFileSystemInfo )
			return;

		for ( KeyValues *pKey = pFileSystemInfo->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
		{
			if ( strcmp( pKey->GetName(), "Label1" ) == 0 )
				ui_Label_contrib1->SetText( pKey->GetString() );
			
			if ( strcmp( pKey->GetName(), "Label2" ) == 0 )
				ui_Label_contrib2->SetText( pKey->GetString() );
			
			if ( strcmp( pKey->GetName(), "Label3" ) == 0 )
				ui_Label_contrib3->SetText( pKey->GetString() );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ZPCredits_Sub_Contrib::SetupPages()
{
	if ( kvPage && iMaxPages == 0 )
	{
		for ( KeyValues *pKey = kvPage->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey() )
		{
			if ( Q_stristr( pKey->GetName(), "page_" ) != 0 )
				iMaxPages++;
		}
	}
	
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
		pString, 2,
	    wcPageNum,
	    wcMaxPages
	);
	ui_Label_pagenum->SetText( output );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ZPCredits_Sub_Contrib::OnTick()
{
	BaseClass::OnTick();

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
	LoadPageInfo( iPageNum );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ZPCredits_Sub_Contrib::OnCommand( const char* pcCommand )
{
	BaseClass::OnCommand(pcCommand);

	if ( !Q_stricmp( pcCommand, "page_next" ) )
		LoadPageInfo( iPageNum++ );
	else if ( !Q_stricmp( pcCommand, "page_back" ) )
		LoadPageInfo( iPageNum-- );
}
