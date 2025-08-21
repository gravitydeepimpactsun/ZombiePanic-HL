//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#ifndef VGUI_ZPCREDITS_H
#define VGUI_ZPCREDITS_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyDialog.h>

//-----------------------------------------------------------------------------
// Purpose: Holds all the game option pages
//-----------------------------------------------------------------------------
class C_ZPCredits : public vgui2::PropertyDialog
{
	DECLARE_CLASS_SIMPLE( C_ZPCredits, vgui2::PropertyDialog );

public:
	C_ZPCredits( vgui2::Panel *pParent );

	void Run();
	virtual void Activate();
};

#define MAX_NUM_ITEMS 15

#endif // VGUI_ZPCREDITS_H
