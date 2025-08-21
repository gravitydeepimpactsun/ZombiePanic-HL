//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include "ienginevgui.h"
#include "C_ZPCredits.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/QueryBox.h>

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>

#include "KeyValues.h"
#include "C_ZPCredits_Sub_Credits.h"
#include "C_ZPCredits_Sub_Contrib.h"
#include "C_ZPCredits_Sub_Info.h"

using namespace vgui2;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: Constructs the class
//-----------------------------------------------------------------------------
C_ZPCredits::C_ZPCredits( vgui2::Panel *pParent ) : PropertyDialog( pParent, "ZPCredits" )
{
	SetDeleteSelfOnClose(true);
	SetBounds(0, 0, 600, 406);
	SetSizeable( false );

	SetTitle( "#ZP_UI_Credits", true );

	AddPage(new C_ZPCredits_Sub_Contrib(this), "#ZP_UI_Credits");
	AddPage(new C_ZPCredits_Sub_Info(this), "#ZP_UI_SUB_Info");

	// Only show OK button
	SetOKButtonVisible( true );
	SetApplyButtonVisible( false );
	SetCancelButtonVisible( false );

	GetPropertySheet()->SetTabWidth(84);
}

//-----------------------------------------------------------------------------
// Purpose: Brings the dialog to the fore
//-----------------------------------------------------------------------------
void C_ZPCredits::Activate()
{
	BaseClass::Activate();
	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Opens the dialog
//-----------------------------------------------------------------------------
void C_ZPCredits::Run()
{
	Activate();
}
