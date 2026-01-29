// ============== Copyright (c) 2026 Monochrome Games ============== \\

#ifndef HUD_PROGRESS_BAR_H
#define HUD_PROGRESS_BAR_H
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include "../../hud/base.h"

class CZPProgressBar : public CHudElemBase<CZPProgressBar>, public vgui2::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CZPProgressBar, vgui2::Panel );

	CZPProgressBar();

	virtual bool IsAllowedToDraw();
	virtual void Paint();
	virtual void ApplySchemeSettings( vgui2::IScheme *pScheme );
	virtual void DrawText( const char *szString, float flProgressTime );

private:
	float m_flChangedRecently;

	CPanelAnimationVarAliasType( int, m_iTextX, "TextX", "70", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextY, "TextY", "5", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextWide, "TextWide", "800", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextTall, "TextTall", "100", "proportional_int" );

	CPanelAnimationVarAliasType( int, m_iBarX, "BarX", "70", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarY, "BarY", "5", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarWide, "BarWide", "800", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iBarTall, "BarTall", "50", "proportional_int" );

protected:
	vgui2::Label *m_pText;
	vgui2::ImagePanel *m_pBackground;
	vgui2::ImagePanel *m_pProgressBar;
	float m_flProgressTime;
	float m_flStartTime;
};

#endif
