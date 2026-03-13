// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "gameui/gameui_viewport.h"
#include <IEngineVGui.h>
#include <IGameConsole.h>
#include "IBaseUI.h"
#include "CBaseMenu.h"
#include "CMenuPage.h"
#include "gameui/CImageMenuButton.h"
#include "gameui/serverbrowser/CServerBrowser.h"
#include "gameui/options/adv_options_dialog.h"
#include "zp/ui/achievements/C_AchievementDialog.h"
#include "zp/ui/playerselection/C_PlayerSelection.h"
#include "zp/ui/createserver/C_CreateServer.h"
#include "zp/ui/credits/C_ZPCredits.h"
#include "zp/ui/workshop/CWorkshopDialog.h"

CON_COMMAND( gameui_background, "Change the background" )
{
	const char *pSetCommand = gEngfuncs.Cmd_Argv( 1 );
	if ( pSetCommand && pSetCommand[0] )
		CGameUIViewport::Get()->GetMenu()->SetNewBackgroundImage( pSetCommand );
}

// Background bounds
struct BackgroundBounds_s
{
public:
	int Size[2];
	int Pos[2];
	BackgroundBounds_s( const int &x, const int &y, const int &w, const int &t )
	{
		Pos[0] = x;
		Pos[1] = y;
		Size[0] = w;
		Size[1] = t;
	}
};

static BackgroundBounds_s BackgroundImageBounds[3][4] = {
	// Top
	{ BackgroundBounds_s( 0, 0, 256, 256 ), BackgroundBounds_s( 256, 0, 256, 256 ), BackgroundBounds_s( 512, 0, 256, 256 ), BackgroundBounds_s( 768, 0, 256, 256 ) },
	// Middle
	{ BackgroundBounds_s( 0, 256, 256, 256 ), BackgroundBounds_s( 256, 256, 256, 256 ), BackgroundBounds_s( 512, 256, 256, 256 ), BackgroundBounds_s( 768, 256, 256, 256 ) },
	// Bottom
	{ BackgroundBounds_s( 0, 512, 256, 88 ), BackgroundBounds_s( 256, 512, 256, 88 ), BackgroundBounds_s( 512, 512, 256, 88 ), BackgroundBounds_s( 768, 512, 256, 88 ) }
};

/// <summary>
/// This is the most hacky shit ever in this piece of code.
/// We have no access to the GameUI, so we gotta do this fucked up shit.
/// I do not recommend doing this, and this is terrible as fuck.
/// </summary>
/// <returns>GameMenu VPanel</returns>
static void GetGameMenuVPanelItems( vgui2::VPANEL &vCreateServer, vgui2::VPANEL &vOpenOptionsDialog, vgui2::VPANEL &vMenuItem1, vgui2::VPANEL &vMenuItem2 )
{
	vgui2::VPANEL vGameUIDLL = g_pEngineVGui->GetPanel( PANEL_GAMEUIDLL );
	vgui2::VPANEL vBasePanel = 0;
	vgui2::VPANEL vTaskBar = 0;
	vgui2::VPANEL vGameMenu = 0;

	vCreateServer = vOpenOptionsDialog = vMenuItem1 = vMenuItem2 = 0;

	int count = g_pVGuiPanel->GetChildCount( vGameUIDLL );
	for ( int i = 0; i < count; i++ )
	{
		vgui2::VPANEL vChild = g_pVGuiPanel->GetChild( vGameUIDLL, i );
		if ( vgui2::FStrEq( g_pVGuiPanel->GetClassName( vChild ), "Panel" ) )
		{
			vBasePanel = vChild;
			break;
		}
	}
	if ( vBasePanel == 0 ) return;

	count = g_pVGuiPanel->GetChildCount( vBasePanel );
	for ( int i = 0; i < count; i++ )
	{
		vgui2::VPANEL vChild = g_pVGuiPanel->GetChild( vBasePanel, i );
		if ( vgui2::FStrEq( g_pVGuiPanel->GetClassName( vChild ), "CTaskbar" ) )
		{
			vTaskBar = vChild;
			break;
		}
	}
	if ( vTaskBar == 0 ) return;
	
	count = g_pVGuiPanel->GetChildCount( vTaskBar );
	for ( int i = 0; i < count; i++ )
	{
		vgui2::VPANEL vChild = g_pVGuiPanel->GetChild( vTaskBar, i );
		if ( vgui2::FStrEq( g_pVGuiPanel->GetName( vChild ), "GameMenu" ) ) vGameMenu = vChild;
		else if ( vgui2::FStrEq( g_pVGuiPanel->GetName( vChild ), "CreateMultiplayerGameDialog" ) ) vCreateServer = vChild;
		else if ( vgui2::FStrEq( g_pVGuiPanel->GetName( vChild ), "OptionsDialog" ) ) vOpenOptionsDialog = vChild;
	}
	if ( vGameMenu == 0 ) return;

/*
	The GameMenu.res needs to only have 2 items, it should look like this:
	"1"
	{
		"label" ""
		"command" "OpenCreateMultiplayerGameDialog"
	}
	"2"
	{
		"label" ""
		"command" "OpenOptionsDialog"
	}
*/
	vMenuItem1 = g_pVGuiPanel->GetChild( vGameMenu, 1 );
	vMenuItem2 = g_pVGuiPanel->GetChild( vGameMenu, 3 );
}

extern IGameConsole *g_pGameConsole;

CBaseMenu::CBaseMenu( vgui2::Panel *pParent )
    : BaseClass( pParent, "CBaseMenu" )
{
	SetSize( 100, 80 );
	SetPos( 0, 0 );
	SetPaintBackgroundEnabled( false );
	SetProportional( true );

	for ( int i = 0; i < 4; i++ )
	{
		// Create Top
		CreateBackgroundBase( 0, i );
		// Create Middle
		CreateBackgroundBase( 1, i );
		// Create Bottom
		CreateBackgroundBase( 2, i );
	}

	for ( int i = 0; i < MenuPagesTable_t::PAGE_MAX; i++ )
		pPage[i] = nullptr;
	m_Page = MenuPagesTable_t::PAGE_MAIN;
	m_hDiscordButton = nullptr;
	m_hMessageBox = nullptr;

	ReadBackgroundFolder();
}

void CBaseMenu::OnCommand( const char *pcCommand )
{
	if (!Q_stricmp(pcCommand, "OpenServerBrowser"))
	{
		gHUD.CallOnNextFrame([]()
		    { CGameUIViewport::Get()->GetServerBrowser()->OpenBrowser(); });
		g_pBaseUI->ActivateGameUI();
	}
	else if (!Q_stricmp(pcCommand, "OpenAdvancedOptions"))
	{
		gHUD.CallOnNextFrame([]()
		    { CGameUIViewport::Get()->GetOptionsDialog()->Activate(); });
		g_pBaseUI->ActivateGameUI();
	}
	else if (!Q_stricmp(pcCommand, "OpenAchievements"))
	{
		gHUD.CallOnNextFrame([]()
		    { CGameUIViewport::Get()->GetAchievementDialog()->Activate(); });
		g_pBaseUI->ActivateGameUI();
	}
	else if (!Q_stricmp(pcCommand, "OpenPlayerSelection"))
	{
		gHUD.CallOnNextFrame([]()
		    { CGameUIViewport::Get()->GetPlayerSelection()->Activate(); });
		g_pBaseUI->ActivateGameUI();
	}
	else if (!Q_stricmp(pcCommand, "OpenCreateServerDialog"))
	{
		gHUD.CallOnNextFrame([]()
		    { CGameUIViewport::Get()->GetCreateServerDialog()->Activate(); });
		g_pBaseUI->ActivateGameUI();
	}
	else if (!Q_stricmp(pcCommand, "OpenWorkshop"))
	{
		gHUD.CallOnNextFrame([]()
		    { CGameUIViewport::Get()->GetWorkshopDialog()->Activate(); });
		g_pBaseUI->ActivateGameUI();
	}
	else if (!Q_stricmp(pcCommand, "OpenCreditsDialog"))
	{
		gHUD.CallOnNextFrame([]()
		    { CGameUIViewport::Get()->GetCredits()->Activate(); });
		g_pBaseUI->ActivateGameUI();
	}
	else if (!Q_stricmp(pcCommand, "OpenCreateMultiplayerGameDialog"))
	{
		vgui2::VPANEL vCreateServer, vOptionsDialog, vCreateServerMenuItem, vCreateOptionsDialogMenuItem;
		GetGameMenuVPanelItems( vCreateServer, vOptionsDialog, vCreateServerMenuItem, vCreateOptionsDialogMenuItem );
		KeyValues *message = new KeyValues( "MousePressed", "code", vgui2::MouseCode::MOUSE_LEFT );
		g_pVGuiPanel->SendMessage( vCreateServerMenuItem, message->MakeCopy(), GetVPanel() );
		message->deleteThis();
	}
	else if (!Q_stricmp(pcCommand, "OpenOptionsDialog"))
	{
		vgui2::VPANEL vCreateServer, vOptionsDialog, vCreateServerMenuItem, vCreateOptionsDialogMenuItem;
		GetGameMenuVPanelItems( vCreateServer, vOptionsDialog, vCreateServerMenuItem, vCreateOptionsDialogMenuItem );
		KeyValues *message = new KeyValues( "MousePressed", "code", vgui2::MouseCode::MOUSE_LEFT );
		g_pVGuiPanel->SendMessage( vCreateOptionsDialogMenuItem, message->MakeCopy(), GetVPanel() );
		message->deleteThis();
	}
	else if (!Q_stricmp(pcCommand, "OpenPageMain"))
		LoadPage(MenuPagesTable_t::PAGE_MAIN);
	else if (!Q_stricmp(pcCommand, "OpenPageOptions"))
		LoadPage(MenuPagesTable_t::PAGE_OPTIONS);
	else if (!Q_stricmp(pcCommand, "OpenPageExtras"))
		LoadPage(MenuPagesTable_t::PAGE_EXTRAS);
	else if (!Q_stricmp(pcCommand, "Quit"))
	{
		if ( m_hMessageBox )
		{
			m_hMessageBox->Activate();
			return;
		}
		m_hMessageBox = new vgui2::MessageBox("#GameUI_QuitConfirmationTitle", "#GameUI_QuitConfirmationText", this);
		m_hMessageBox->SetOKButtonVisible(true);
		m_hMessageBox->SetCancelButtonVisible(true);
		m_hMessageBox->AddActionSignalTarget(this);
		m_hMessageBox->SetCommand("OnQuitConfirm");
		m_hMessageBox->SetCancelCommand( new KeyValues("Command", "command", "OnMessageBoxClosed") );
		m_hMessageBox->DoModal();
	}
	else if (!Q_stricmp(pcCommand, "OnQuitConfirm"))
	{
		if ( m_hMessageBox )
			m_hMessageBox->DeletePanel();
		m_hMessageBox = nullptr;
		EngineClientCmd("quit\n");
	}
	else if (!Q_stricmp(pcCommand, "OnDisconnect"))
	{
		if ( m_hMessageBox )
			m_hMessageBox->DeletePanel();
		m_hMessageBox = nullptr;
		EngineClientCmd("disconnect\n");
	}
	else if (!Q_stricmp(pcCommand, "OnMessageBoxClosed"))
	{
		if ( m_hMessageBox )
			m_hMessageBox->DeletePanel();
		m_hMessageBox = nullptr;
	}
	else if (!Q_stricmp(pcCommand, "Disconnect"))
	{
		if ( m_hMessageBox )
		{
			m_hMessageBox->Activate();
			return;
		}
		m_hMessageBox = new vgui2::MessageBox("#GameUI_DisconnectConfirmationTitle", "#GameUI_DisconnectConfirmationText", this);
		m_hMessageBox->SetOKButtonVisible(true);
		m_hMessageBox->SetCancelButtonVisible(true);
		m_hMessageBox->AddActionSignalTarget(this);
		m_hMessageBox->SetCommand("OnDisconnect");
		m_hMessageBox->SetCancelCommand( new KeyValues("Command", "command", "OnMessageBoxClosed") );
		m_hMessageBox->DoModal();
	}
	else if (!Q_stricmp(pcCommand, "ResumeGame"))
		g_pBaseUI->HideGameUI();
	else if (!Q_stricmp(pcCommand, "ShowConsole"))
		g_pBaseUI->ShowConsole();
	else if (!Q_stricmp(pcCommand, "QuickDialogHackFix"))
		DoDialogHackFix();
	else
		BaseClass::OnCommand( pcCommand );
}

void CBaseMenu::LoadPage( MenuPagesTable_t nPage )
{
	CMenuPage *pNewPage = TryCreatePage( nPage );
	if ( !pNewPage ) return;
	pNewPage->PopulateMenu();
	pNewPage->DisablePanel( false );
}

CMenuPage *CBaseMenu::TryCreatePage( MenuPagesTable_t nPage )
{
	// If valid, hide current.
	if ( pPage[m_Page] )
		pPage[m_Page]->DisablePanel( true );

	m_Page = nPage;

	// If it already exist, switch to it, no need to create it.
	if ( pPage[nPage] )
		return pPage[nPage];

	const char *szTitle = nullptr;
	switch ( nPage )
	{
		case PAGE_MAIN: szTitle = "#ZP_GameTitle"; break;
		case PAGE_OPTIONS: szTitle = "#ZP_UI_MenuTitle_Options"; break;
		case PAGE_EXTRAS: szTitle = "#ZP_UI_MenuTitle_Extras"; break;
	}
	if ( !szTitle ) return nullptr;
	pPage[m_Page] = new CMenuPage( this, m_Page, szTitle );
	pPage[m_Page]->MakePopup( false, false );
	pPage[m_Page]->SetZPos( 0 );
	return pPage[m_Page];
}

void CBaseMenu::SetMenuBounds( const int &x, const int &y, const int &w, const int &t )
{
	SetBounds( x, y, w, t );

	// Setup the size and pos for the background images
	for ( int i = 0; i < 4; i++ )
	{
		SetupBackgroundBaseBounds( 0, i );
		SetupBackgroundBaseBounds( 1, i );
		SetupBackgroundBaseBounds( 2, i );
	}

	// Create our dialog right away!
	if ( !m_hDiscordButton )
	{
		int wide = GetScaledValue( 25 );
		int tall = GetScaledValue( 25 );
		int nudge = GetScaledValue( 50 );
		m_hDiscordButton = new CImageMenuButton( this, "ui/discord_button", "https://discord.gg/zps" );
		m_hDiscordButton->SetImageHover( "ui/discord_button_hover" );
		m_hDiscordButton->MakePopup( false, false );
		m_hDiscordButton->SetText( "#ZP_Discord", "#ZP_Discord_Tip" );
		m_hDiscordButton->SetContent( (w - wide) - nudge, (t - tall) - nudge, wide, tall );
		m_hDiscordButton->MoveToFront();
	}
}

void CBaseMenu::Repopulate()
{
	for ( int i = 0; i < MenuPagesTable_t::PAGE_MAX; i++ )
	{
		if ( pPage[i] )
			pPage[i]->PopulateMenu();
	}
}

void CBaseMenu::SetNewBackgroundImage( const char *szImage )
{
	if ( !szImage ) return;
	if ( vgui2::FStrEq( szImage, "" ) ) return;
	for ( int i = 0; i < 4; i++ )
	{
		// Top
		m_pBackgroundImage[0][i]->SetImage( vgui2::VarArgs( "ui/backgrounds/%s/%i_%i", szImage, 0, i ) );
		// Middle
		m_pBackgroundImage[1][i]->SetImage( vgui2::VarArgs( "ui/backgrounds/%s/%i_%i", szImage, 1, i ) );
		// Bottom
		m_pBackgroundImage[2][i]->SetImage( vgui2::VarArgs( "ui/backgrounds/%s/%i_%i", szImage, 2, i ) );
	}
}

void CBaseMenu::ToggleBackground( bool bVisible )
{
	for ( int i = 0; i < 4; i++ )
	{
		m_pBackgroundImage[0][i]->SetVisible( bVisible );
		m_pBackgroundImage[1][i]->SetVisible( bVisible );
		m_pBackgroundImage[2][i]->SetVisible( bVisible );
	}
}

void CBaseMenu::ReadBackgroundFolder()
{
	std::vector<std::string> m_List;

	// Check our folders, and see if we have the TGA files.
	FileFindHandle_t fh;
	char const *fn = g_pFullFileSystem->FindFirst( "ui/backgrounds/*.*", &fh );
	do
	{
		// Setup the path string, and lowercase it, so we don't need to search for both uppercase, and lowercase files.
		char strFile[ 4028 ];
		bool isSameDir = false;
		strFile[ 0 ] = 0;
		V_strcpy_safe( strFile, fn );
		Q_strlower( strFile );

		// Ignore the same folder
		if ( vgui2::FStrEq( strFile, "." ) || vgui2::FStrEq( strFile, ".." ) )
			isSameDir = true;

		// Folder found!
		if ( g_pFullFileSystem->FindIsDirectory( fh ) && !isSameDir )
		{
			// Setup the string
			std::string strBasePath = "ui/backgrounds/" + std::string( strFile );
			bool bHasFile = true;
			for ( int i = 0; i < 4; i++ )
			{
				// Top
				if ( !g_pFullFileSystem->FileExists( vgui2::VarArgs( "%s/%i_%i.tga", strBasePath.c_str(), 0, i ) ) )
					bHasFile = false;
				// Middle
				if ( !g_pFullFileSystem->FileExists( vgui2::VarArgs( "%s/%i_%i.tga", strBasePath.c_str(), 1, i ) ) )
					bHasFile = false;
				// Bottom
				if ( !g_pFullFileSystem->FileExists( vgui2::VarArgs( "%s/%i_%i.tga", strBasePath.c_str(), 2, i ) ) )
					bHasFile = false;
			}
			if ( bHasFile )
				m_List.push_back( strFile );
		}

		fn = g_pFullFileSystem->FindNext( fh );
	}
	while(fn);

	g_pFullFileSystem->FindClose( fh );

	// If the list is empty, just read background01
	if ( m_List.size() == 0 )
		SetNewBackgroundImage( "background01" );
	else
		SetNewBackgroundImage( m_List[ RandomInt( 0, m_List.size() - 1 ) ].c_str() );
}

void CBaseMenu::CreateBackgroundBase(int iTopIndex, int iImages)
{
	m_pBackgroundImage[iTopIndex][iImages] = new vgui2::ImagePanel( this, vgui2::VarArgs( "bg%i_%i", iTopIndex, iImages ) );
	m_pBackgroundImage[iTopIndex][iImages]->MakePopup( false, false );
	m_pBackgroundImage[iTopIndex][iImages]->SetFillColor( Color( 0, 0, 0, 255 ) );
	m_pBackgroundImage[iTopIndex][iImages]->SetSize( GetWide(), GetTall() );
	m_pBackgroundImage[iTopIndex][iImages]->SetPos( 0, 0 );
	m_pBackgroundImage[iTopIndex][iImages]->SetShouldScaleImage( true );
	m_pBackgroundImage[iTopIndex][iImages]->SetMouseInputEnabled( false );
	m_pBackgroundImage[iTopIndex][iImages]->SetKeyBoardInputEnabled( false );
	m_pBackgroundImage[iTopIndex][iImages]->SetZPos( -500 );
}

void CBaseMenu::SetupBackgroundBaseBounds( int iTopIndex, int iImages )
{
	BackgroundBounds_s background = BackgroundImageBounds[iTopIndex][iImages];
	m_pBackgroundImage[iTopIndex][iImages]->SetSize( GetScaledValue( background.Size[0] ), GetScaledValue( background.Size[1] ) );
	m_pBackgroundImage[iTopIndex][iImages]->SetPos( GetScaledValue( background.Pos[0] ), GetScaledValue( background.Pos[1] ) );
}

void CBaseMenu::InternalMousePressed(int code)
{
	DoDialogHackFix();
}

struct VPanelMoveData_t
{
	vgui2::VPANEL panel;
	int zOrder;
};

void CBaseMenu::DoDialogHackFix()
{
	// The moment we press on the menu, the items will be in front of the windows, due to it being MakePopup function.
	// To fix this, let's simply move all the windows to the front (excluding CMenuPage). But still containing the same order.
	std::vector< VPanelMoveData_t > m_List;

	vgui2::VPANEL vCreateServer, vOptionsDialog, vCreateServerMenuItem, vCreateOptionsDialogMenuItem;
	GetGameMenuVPanelItems( vCreateServer, vOptionsDialog, vCreateServerMenuItem, vCreateOptionsDialogMenuItem );
	if ( vCreateServer != 0 )
		m_List.push_back( { vCreateServer, g_pVGuiPanel->GetZPos( vCreateServer ) } );
	if ( vOptionsDialog != 0 )
		m_List.push_back( { vOptionsDialog, g_pVGuiPanel->GetZPos( vOptionsDialog ) } );

	// If any of these are valid, and visible, add them to the list as well.
	for ( int i = 0; i < GameUIDialogs::UIDialog_MAX; i++ )
	{
		if ( CGameUIViewport::Get()->GetDialog( (GameUIDialogs)i ) && CGameUIViewport::Get()->GetDialog( (GameUIDialogs)i )->IsVisible() )
			m_List.push_back( { CGameUIViewport::Get()->GetDialog( (GameUIDialogs)i )->GetVPanel(), g_pVGuiPanel->GetZPos( CGameUIViewport::Get()->GetDialog( (GameUIDialogs)i )->GetVPanel() ) } );
	}

	// Now, let's sort the list by the z order. From the lowest to highest.
	std::sort( m_List.begin(), m_List.end(), []( const VPanelMoveData_t &a, const VPanelMoveData_t &b )
	{
		return a.zOrder < b.zOrder; 
	});

	// Make sure we move this to the front!
	if ( m_hDiscordButton && HasFocus() )
		m_hDiscordButton->MoveToFront();

	// If our console is valid, and visible, then do it.
	if ( g_pGameConsole && g_pGameConsole->IsConsoleVisible() )
		g_pGameConsole->Activate();

	// Now, let's move the items to the front, in the same order as they were before.
	for ( const auto &data : m_List )
		g_pVGuiPanel->MoveToFront( data.panel );

	// Let's move our server browser dialogs to front as well.
	if ( CGameUIViewport::Get()->GetServerBrowser() )
		CGameUIViewport::Get()->GetServerBrowser()->MoveAllDialogsToFront();

	// This has higher priorty than the popups, so move it to the front after them.
	if ( m_hMessageBox && m_hMessageBox->IsVisible() )
		m_hMessageBox->MoveToFront();
}
