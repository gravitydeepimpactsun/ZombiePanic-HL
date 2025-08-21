// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include "gameui/gameui_viewport.h"
#include <IEngineVGui.h>
#include "IBaseUI.h"
#include "CBaseMenu.h"
#include "CMenuPage.h"
#include "gameui/CImageMenuButton.h"
#include "gameui/serverbrowser/CServerBrowser.h"
#include "gameui/options/adv_options_dialog.h"
#include "zp/ui/achievements/C_AchievementDialog.h"
#include "zp/ui/playerselection/C_PlayerSelection.h"
#include "zp/ui/workshop/CWorkshopDialog.h"

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
	if ( !Q_stricmp( pcCommand, "OpenServerBrowser" ) )
	{
		gHUD.CallOnNextFrame([]() {
			CGameUIViewport::Get()->GetServerBrowser()->OpenBrowser();
		});
		g_pBaseUI->ActivateGameUI();
	}
	else if ( !Q_stricmp( pcCommand, "OpenAdvancedOptions" ) )
	{
		gHUD.CallOnNextFrame([]() {
			CGameUIViewport::Get()->GetOptionsDialog()->Activate();
		});
		g_pBaseUI->ActivateGameUI();
	}
	else if ( !Q_stricmp( pcCommand, "OpenAchievements" ) )
	{
		gHUD.CallOnNextFrame([]() {
			CGameUIViewport::Get()->GetAchievementDialog()->Activate();
		});
		g_pBaseUI->ActivateGameUI();
	}
	else if ( !Q_stricmp( pcCommand, "OpenPlayerSelection" ) )
	{
		gHUD.CallOnNextFrame([]() {
			CGameUIViewport::Get()->GetPlayerSelection()->Activate();
		});
		g_pBaseUI->ActivateGameUI();
	}
	else if ( !Q_stricmp( pcCommand, "OpenWorkshop" ) )
	{
		gHUD.CallOnNextFrame([]() {
			CGameUIViewport::Get()->GetWorkshopDialog()->Activate();
		});
		g_pBaseUI->ActivateGameUI();
	}
	else if ( !Q_stricmp( pcCommand, "OpenCreditsDialog" ) )
	{
		// TODO: Create Credits Dialog
	}
	else if ( !Q_stricmp( pcCommand, "OpenCreateMultiplayerGameDialog" ) )
	{
		// TODO: Create Server Dialog
		// Let's create our own server creation dialog, because it does not want
		// to save the "use steam networking" variable, for example, if you
		// want it turned off...
	}
	else if ( !Q_stricmp( pcCommand, "OpenOptionsDialog" ) )
	{
		// TODO: Create Options Dialog?
		// Or somehow open the GameUI one, but it doesn't want to call it....

		// DEBUG TEST -- Does not seem to work?
		vgui2::VPANEL GameUIDLL = g_pEngineVGui->GetPanel( PANEL_GAMEUIDLL );
		ConPrintf( "PANEL_GAMEUIDLL: [%i]\n", GameUIDLL );
		ConPrintf( "Message: [%s]\n", pcCommand );
		KeyValuesAD message( pcCommand );
		message->SetPtr( "panel", this );
		vgui2::ivgui()->PostMessage( GameUIDLL, message->MakeCopy(), GetVPanel() );
	}
	else if ( !Q_stricmp( pcCommand, "OpenPageMain" ) )
		LoadPage( MenuPagesTable_t::PAGE_MAIN );
	else if ( !Q_stricmp( pcCommand, "OpenPageOptions" ) )
		LoadPage( MenuPagesTable_t::PAGE_OPTIONS );
	else if ( !Q_stricmp( pcCommand, "OpenPageExtras" ) )
		LoadPage( MenuPagesTable_t::PAGE_EXTRAS );
	else if ( !Q_stricmp( pcCommand, "Quit" ) )
	{
		vgui2::MessageBox *pMessageBox = new vgui2::MessageBox( "#GameUI_QuitConfirmationTitle", "#GameUI_QuitConfirmationText", this );
		pMessageBox->SetOKButtonVisible( true );
		pMessageBox->SetCancelButtonVisible( true );
		pMessageBox->AddActionSignalTarget( this );
		pMessageBox->SetCommand( "OnQuitConfirm" );
		pMessageBox->DoModal();
	}
	else if ( !Q_stricmp( pcCommand, "OnQuitConfirm" ) )
		EngineClientCmd( "quit\n" );
	else if ( !Q_stricmp( pcCommand, "OnDisconnect" ) )
		EngineClientCmd( "disconnect\n" );
	else if ( !Q_stricmp( pcCommand, "Disconnect" ) )
	{
		vgui2::MessageBox *pMessageBox = new vgui2::MessageBox( "#GameUI_DisconnectConfirmationTitle", "#GameUI_DisconnectConfirmationText", this );
		pMessageBox->SetOKButtonVisible( true );
		pMessageBox->SetCancelButtonVisible( true );
		pMessageBox->AddActionSignalTarget( this );
		pMessageBox->SetCommand( "OnDisconnect" );
		pMessageBox->DoModal();
	}
	else if ( !Q_stricmp( pcCommand, "ResumeGame" ) )
		g_pBaseUI->HideGameUI();
	else if ( !Q_stricmp( pcCommand, "ShowConsole" ) )
		g_pBaseUI->ShowConsole();
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

void CBaseMenu::OnThink()
{
	// Make sure we move this to the front!
	if ( m_hPatreonButton && HasFocus() )
		m_hPatreonButton->MoveToFront();
}
