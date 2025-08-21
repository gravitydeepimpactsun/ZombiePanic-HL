//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include "C_ZPCredits_Sub_Info.h"
#include "client_vgui.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <stdio.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui2;

C_ZPCredits_Sub_Info::C_ZPCredits_Sub_Info(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	LoadControlSettings( VGUI2_ROOT_DIR "resource/zps/credits/sub_info.res");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_ZPCredits_Sub_Info::~C_ZPCredits_Sub_Info()
{
}
