//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include "C_ZPCredits_Sub_Credits.h"
#include "client_vgui.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <stdio.h>
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui2;

C_ZPCredits_Sub_Credits::C_ZPCredits_Sub_Credits(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	LoadControlSettings( VGUI2_ROOT_DIR "resource/zps/credits/sub_credits.res");

	Panel *pPanel = FindChildByName( "Label_Menu1" );
	vgui2::Label *pLabel_Menu1 = (vgui2::Label *)pPanel;
	if ( pLabel_Menu1 )
		pLabel_Menu1->SetText("#ZP_UI_TeamLeaders");

	pPanel = FindChildByName( "Label_Menu2" );
	vgui2::Label *pLabel_Menu2 = (vgui2::Label *)pPanel;
	if ( pLabel_Menu2 )
		pLabel_Menu2->SetText("#ZP_UI_Artists");

	pPanel = FindChildByName( "Label_Menu3" );
	vgui2::Label *pLabel_Menu3 = (vgui2::Label *)pPanel;
	if ( pLabel_Menu3 )
		pLabel_Menu3->SetText("#ZP_UI_LevelDesigners");

	pPanel = FindChildByName( "Label_Menu5" );
	vgui2::Label *pLabel_Menu5 = (vgui2::Label *)pPanel;
	if ( pLabel_Menu5 )
		pLabel_Menu5->SetText("#ZP_UI_Programmers");

	pPanel = FindChildByName( "Label_Menu7" );
	vgui2::Label *pLabel_Menu7 = (vgui2::Label *)pPanel;
	if ( pLabel_Menu7 )
		pLabel_Menu7->SetText("#ZP_UI_SoundDesigners");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_ZPCredits_Sub_Credits::~C_ZPCredits_Sub_Credits()
{
}
