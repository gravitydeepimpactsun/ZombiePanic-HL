// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef HUD_OBJECTIVE_TEXT_H
#define HUD_OBJECTIVE_TEXT_H
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include "../../hud/base.h"

class CObjectiveText : public CHudElemBase<CObjectiveText>, public vgui2::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CObjectiveText, vgui2::Panel );

	CObjectiveText();

	virtual bool IsAllowedToDraw();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

	int MsgFunc_ObjMsg(const char *pszName, int iSize, void *pbuf);

private:
	float m_flChangedRecently;
	static bool m_bObjectiveChanged;
	static ObjectiveState m_State;
	static std::string m_strObjectiveString;
	static std::string m_strObjectiveString_Changed;

	CPanelAnimationVarAliasType( int, m_iTextX, "TextX", "70", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextY, "TextY", "5", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextWide, "TextWide", "800", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextTall, "TextTall", "100", "proportional_int" );

protected:
	vgui2::Label *m_pText;
};

#endif
