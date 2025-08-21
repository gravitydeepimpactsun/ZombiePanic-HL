//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#ifndef VGUI_ZPCREDITS_SUB_CONTRIB_H
#define VGUI_ZPCREDITS_SUB_CONTRIB_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Button.h>

namespace vgui2
{
	class Panel;
	class Button;
}

//-----------------------------------------------------------------------------
// Purpose: Mouse Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class C_ZPCredits_Sub_Contrib : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE( C_ZPCredits_Sub_Contrib, vgui2::PropertyPage );
	
	// Labels
	vgui2::Label *ui_Label_contrib1;
	vgui2::Label *ui_Label_contrib2;
	vgui2::Label *ui_Label_contrib3;
	vgui2::Label *ui_Label_pagenum;

	// Buttons
	vgui2::Button *ui_Bttn_Back;
	vgui2::Button *ui_Bttn_Next;

	// Page Num
	int iPageNum;
	int iMaxPages;

	// Keyvalues
	KeyValues *kvPage;

public:
	C_ZPCredits_Sub_Contrib(vgui2::Panel *parent);
	~C_ZPCredits_Sub_Contrib();

	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);

	void LoadPageInfo( int pagenum );
	void CreatePageInfo();
	void SetupPages();
};



#endif // VGUI_ZPCREDITS_SUB_CONTRIB_H
