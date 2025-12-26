// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef GAMEUI_MENU_BASE_H
#define GAMEUI_MENU_BASE_H

#include <vgui_controls/Panel.h>
#include "IMenuCore.h"

class CMenuPage;
class CImageMenuButton;

class CBaseMenu : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE( CBaseMenu, vgui2::Panel ); 

public:
	CBaseMenu( vgui2::Panel *pParent );
	void OnCommand( const char* pcCommand );
	void LoadPage( MenuPagesTable_t nPage );
	CMenuPage *TryCreatePage( MenuPagesTable_t nPage );
	void SetMenuBounds( const int &x, const int &y, const int &w, const int &t );
	void Repopulate();
	void SetNewBackgroundImage( const char *szImage );
	void ToggleBackground( bool bVisible );

protected:
	void CreateBackgroundBase( int iTopIndex, int iImages );
	void SetupBackgroundBaseBounds( int iTopIndex, int iImages );
	void InternalMousePressed( int code ) override;
	inline void InternalMouseDoublePressed( int code ) override { InternalMousePressed( code ); }
	void DoDialogHackFix();

private:
	vgui2::DHANDLE<CImageMenuButton> m_hPatreonButton;
	vgui2::DHANDLE<CImageMenuButton> m_hDiscordButton;
	vgui2::DHANDLE<vgui2::MessageBox> m_hMessageBox;
	MenuPagesTable_t m_Page;
	CMenuPage *pPage[PAGE_MAX];
	vgui2::ImagePanel *m_pBackgroundImage[3][4];
};

#endif