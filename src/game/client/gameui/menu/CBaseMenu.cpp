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
#include "zp/ui/workshop/CWorkshopDialog.h"

/// <summary>
/// This is the most hacky shit ever in this piece of code.
/// We have no access to the GameUI, so we gotta do this fucked up shit.
/// I do not recommend doing this, and ths is terrible as fuck.
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
	for ( int i = 0; i < MenuPagesTable_t::PAGE_MAX; i++ )
		pPage[i] = nullptr;
	m_Page = MenuPagesTable_t::PAGE_MAIN;
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
	else if (!Q_stricmp(pcCommand, "OpenWorkshop"))
	{
		gHUD.CallOnNextFrame([]()
		    { CGameUIViewport::Get()->GetWorkshopDialog()->Activate(); });
		g_pBaseUI->ActivateGameUI();
	}
	else if (!Q_stricmp(pcCommand, "OpenCreditsDialog"))
	{
		// TODO: Create Credits Dialog
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
		vgui2::MessageBox *pMessageBox = new vgui2::MessageBox("#GameUI_QuitConfirmationTitle", "#GameUI_QuitConfirmationText", this);
		pMessageBox->SetOKButtonVisible(true);
		pMessageBox->SetCancelButtonVisible(true);
		pMessageBox->AddActionSignalTarget(this);
		pMessageBox->SetCommand("OnQuitConfirm");
		pMessageBox->DoModal();
	}
	else if (!Q_stricmp(pcCommand, "OnQuitConfirm"))
		EngineClientCmd("quit\n");
	else if (!Q_stricmp(pcCommand, "OnDisconnect"))
		EngineClientCmd("disconnect\n");
	else if (!Q_stricmp(pcCommand, "Disconnect"))
	{
		vgui2::MessageBox *pMessageBox = new vgui2::MessageBox("#GameUI_DisconnectConfirmationTitle", "#GameUI_DisconnectConfirmationText", this);
		pMessageBox->SetOKButtonVisible(true);
		pMessageBox->SetCancelButtonVisible(true);
		pMessageBox->AddActionSignalTarget(this);
		pMessageBox->SetCommand("OnDisconnect");
		pMessageBox->DoModal();
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
	return pPage[m_Page];
}

void CBaseMenu::SetMenuBounds( const int &x, const int &y, const int &w, const int &t )
{
	SetBounds( x, y, w, t );
	// Create our dialog right away!
	if ( !m_hPatreonButton )
	{
		m_hPatreonButton = new CImageMenuButton( this, "ui/patreon_button", "https://patreon.com/wuffesan" );
		m_hPatreonButton->MakePopup( false, false );
		m_hPatreonButton->SetContent( (w - 256) - 50, (t - 128) - 50, 256, 128 );
		m_hPatreonButton->MoveToFront();
	}
}

void CBaseMenu::InternalMousePressed( int code )
{
	DoDialogHackFix();
}

void CBaseMenu::DoDialogHackFix()
{
	// Make sure we move this to the front!
	if ( m_hPatreonButton && HasFocus() )
		m_hPatreonButton->MoveToFront();

	// If any of these are valid, and visible, move them to front.
	for ( int i = 0; i < GameUIDialogs::UIDialog_MAX; i++ )
	{
		if ( CGameUIViewport::Get()->GetDialog( (GameUIDialogs)i ) && CGameUIViewport::Get()->GetDialog( (GameUIDialogs)i )->IsVisible() )
			CGameUIViewport::Get()->GetDialog( (GameUIDialogs)i )->MoveToFront();
	}

	// If our console is valid, and visible, then do it.
	if ( g_pGameConsole && g_pGameConsole->IsConsoleVisible() )
		g_pGameConsole->Activate();

	// Let's do the same for our Create Server Dialog and Options Dialog.
	vgui2::VPANEL vCreateServer, vOptionsDialog, vCreateServerMenuItem, vCreateOptionsDialogMenuItem;
	GetGameMenuVPanelItems( vCreateServer, vOptionsDialog, vCreateServerMenuItem, vCreateOptionsDialogMenuItem );
	if ( vCreateServer != 0 )
		g_pVGuiPanel->MoveToFront( vCreateServer );
	if ( vOptionsDialog != 0 )
		g_pVGuiPanel->MoveToFront( vOptionsDialog );
}
