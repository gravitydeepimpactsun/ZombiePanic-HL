// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef HUD_ZOMBIELIVES_H
#define HUD_ZOMBIELIVES_H
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include "../../hud/base.h"

class CHudZombieLives : public CHudElemBase<CHudZombieLives>, public vgui2::Panel
{
public:
	DECLARE_CLASS_SIMPLE(CHudZombieLives, vgui2::Panel);

	CHudZombieLives();

	virtual bool IsAllowedToDraw();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

	int MsgFunc_ZombieLives(const char *pszName, int iSize, void *pbuf);

private:
	/*
		0 - 320
		1 - 640
		2 - 1280
		3 - 2560
	*/

	CPanelAnimationVarAliasType( int, m_iIconTexture, "IconTexture", "ui/gfx/hud/zombielives", "textureid" );
	CPanelAnimationVarAliasType( int, m_iIconTextureHC, "IconTexture", "ui/gfx/hud/zombielives_hc", "textureid" );

	// 320
	CPanelAnimationVarAliasType( int, m_iIconWide_0, "IconWide_0", "20", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconTall_0, "IconTall_0", "20", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextYAdd_0, "TextYAdd_0", "5", "proportional_int" );

	// 640
	CPanelAnimationVarAliasType( int, m_iIconWide_1, "IconWide_1", "20", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconTall_1, "IconTall_1", "20", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextYAdd_1, "TextYAdd_1", "5", "proportional_int" );

	// 1280
	CPanelAnimationVarAliasType( int, m_iIconWide_2, "IconWide_2", "40", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconTall_2, "IconTall_2", "40", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextYAdd_2, "TextYAdd_2", "10", "proportional_int" );

	// 2560
	CPanelAnimationVarAliasType( int, m_iIconWide_3, "IconWide_3", "60", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iIconTall_3, "IconTall_3", "60", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextYAdd_3, "TextYAdd_3", "15", "proportional_int" );

	CPanelAnimationVarAliasType( int, m_iTextXAdd, "TextXAdd", "5", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextSizeWide, "TextWide", "100", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextSizeTall, "TextTall", "20", "proportional_int" );
	CPanelAnimationStringVar( 32, m_szLivesText, "Font", "ZPLives" );

	vgui2::Label *m_pLives;
	Color m_clrIcon;
};

#endif
