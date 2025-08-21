//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#ifndef VGUI_ZPCREDITS_SUB_INFO_H
#define VGUI_ZPCREDITS_SUB_INFO_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CLabeledCommandComboBox;
class CCvarToggleCheckButton;

namespace vgui2
{
	class Label;
	class Panel;
}

//-----------------------------------------------------------------------------
// Purpose: Mouse Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class C_ZPCredits_Sub_Info : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE( C_ZPCredits_Sub_Info, vgui2::PropertyPage );

public:
	C_ZPCredits_Sub_Info(vgui2::Panel *parent);
	~C_ZPCredits_Sub_Info();
};



#endif // VGUI_ZPCREDITS_SUB_INFO_H
