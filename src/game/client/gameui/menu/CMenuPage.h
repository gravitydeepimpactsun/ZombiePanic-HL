// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef GAMEUI_MENU_PAGE_H
#define GAMEUI_MENU_PAGE_H

#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include "IMenuCore.h"

class CMenuItem;

class CMenuPage : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE( CMenuPage, vgui2::Panel ); 

public:
	CMenuPage( vgui2::Panel *pParent, MenuPagesTable_t nType, const char *szTitle );
	void PopulateMenu();
	void DisablePanel( bool state );
	void OnCommand( const char* pcCommand );

protected:
	virtual void ApplySchemeSettings( vgui2::IScheme *pScheme );
	virtual void OnThink() override;

private:
	CMenuItem *m_pMenuItems[MAX_PAGE_MENU_ITEMS];
	MenuPagesTable_t m_nType;
	vgui2::Label *m_pTitle;
	bool m_IsConnected;
};

#endif