// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef GAMEUI_MENU_ITEM_H
#define GAMEUI_MENU_ITEM_H

#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>

class CMenuItem : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE( CMenuItem, vgui2::Panel );

public:
	CMenuItem( vgui2::Panel *pParent, const char *szImage, const char *szText, const char *szHelp, const char *szCommand );
	void SetContent( const int &x, const int &y, const int &w, const int &h );
	void OnMousePressed( vgui2::MouseCode code ) override;
	
protected:
	virtual void ApplySchemeSettings( vgui2::IScheme *pScheme );

	virtual void OnCursorEntered();
	virtual void OnCursorExited();

private:
	vgui2::ImagePanel *m_pPanel;
	vgui2::Label *m_pText;
	vgui2::Label *m_pHelpText;
	char m_szCommand[64];
};

#endif