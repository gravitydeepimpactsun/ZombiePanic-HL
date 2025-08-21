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

protected:
	virtual void OnThink() override;

private:
	vgui2::DHANDLE<CImageMenuButton> m_hPatreonButton;
	MenuPagesTable_t m_Page;
	CMenuPage *pPage[PAGE_MAX];
};

#endif