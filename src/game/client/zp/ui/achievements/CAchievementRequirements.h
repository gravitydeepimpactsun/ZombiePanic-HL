//========= Copyright (c) 2025 Monochrome Games, All rights reserved. ============//

#ifndef ACHLIST_REQUIREMENT_H
#define ACHLIST_REQUIREMENT_H

#ifdef _WIN32
#pragma once
#endif

#include <utllinkedlist.h>
#include <utlvector.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ButtonImage.h>

class KeyValues;

namespace vgui2
{
	// Backported from Contagion,
	// some minor tweaks and changes has been made for GoldSrc variant
	class CBasicItemCombo : public vgui2::Panel
	{
		DECLARE_CLASS_SIMPLE( CBasicItemCombo, vgui2::Panel );
	public:
		CBasicItemCombo( vgui2::Panel* parent, char const *panelName, bool bEarned1, const char *szLabel1, bool bEarned2, const char *szLabel2 );
		CBasicItemCombo( vgui2::Panel* parent, char const *panelName, bool bEarned, const char *szLabel );
	    bool HasSecondItem() const;
	    void SetSecondItem( bool bEarned, const char *szLabel );
		void UpdateLayout();
	private:
	    vgui2::ImagePanel *m_Image1;
	    vgui2::ImagePanel *m_Image2;
		vgui2::Label *m_Label1;
		vgui2::Label *m_Label2;
	};

	class CAchievementRequirementsHolder : public vgui2::Panel
	{
	    DECLARE_CLASS_SIMPLE(CAchievementRequirementsHolder, Panel);
	    CAchievementRequirementsHolder(vgui2::Panel *parent, char const *panelName);
	    ~CAchievementRequirementsHolder();
	    void AddItem(bool obtained, const char *text);
	    void OnCommand(const char *szCommand);
	    void UpdateSize(bool bExpand);
	    int GetCurrentHeight() const { return m_iSizeHeight; }
	private:
	    std::vector<CBasicItemCombo *> m_pList;
	    ButtonImage *m_pButton;
	    Label *m_pLabel;
	    bool m_bShouldExpand;
	    int m_iSizeHeight;
	    int m_iSizeHeightDefault;
	};
}
#endif // ACHIVLIST_H
