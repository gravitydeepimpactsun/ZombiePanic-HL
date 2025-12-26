// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef IMAGE_MENU_BUTTON
#define IMAGE_MENU_BUTTON

#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>

class CImageMenuButton : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE( CImageMenuButton, vgui2::Panel );

public:
	CImageMenuButton( vgui2::Panel *pParent, const char *szImage, const char *szURL );
	void SetImageHover( const char *szImage );
	void SetContent( const int &x, const int &y, const int &w, const int &h );
	void OnMousePressed( vgui2::MouseCode code ) override;
	void SetText( const char *szText );
	void SetText( const char *szText, const char *szHelpText );

protected:
	virtual void ApplySchemeSettings( vgui2::IScheme *pScheme );
	void OnTick() override;
	virtual void OnCursorEntered();
	virtual void OnCursorExited();
	void UpdateTextColor( vgui2::Label *pLabel, const Color vColorTarget, int xpos );

private:
	vgui2::Label *m_Text;
	vgui2::Label *m_TextHelp;
	vgui2::ImagePanel *m_pPanel;
	int m_TextSize[2];
	char m_szImage[512];
	char m_szImageHover[512];
	char m_szURL[512];
	bool m_bIsHovering;
};

#endif