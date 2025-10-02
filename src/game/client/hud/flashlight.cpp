/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// flashlight.cpp
//
// implementation of CHudFlashlight class
//

#include <string.h>
#include <stdio.h>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "flashlight.h"
#include "event_api.h"

#define BAT_NAME "sprites/%d_Flashlight.spr"

#define SOUND_ZOMBVISION_ON  "items/zvision_on.wav"
#define SOUND_ZOMBVISION_OFF "items/zvision_off.wav"

extern globalvars_t *gpGlobals;

DEFINE_HUD_ELEM(CHudFlashlight);

void CHudFlashlight::Init(void)
{
	BaseHudClass::Init();

	m_fFade = 0;
	m_fOn = 0;

	HookMessage<&CHudFlashlight::MsgFunc_Flashlight>("Flashlight");
	HookMessage<&CHudFlashlight::MsgFunc_FlashBat>("FlashBat");

	m_iFlags |= HUD_ACTIVE;
};

void CHudFlashlight::Reset(void)
{
	m_fFade = 0;
	m_fOn = 0;
	m_iBat = 100;
	m_flBat = 1.0;
}

void CHudFlashlight::VidInit(void)
{
	int HUD_flash_empty = gHUD.GetSpriteIndex("flash_empty");
	int HUD_flash_full = gHUD.GetSpriteIndex("flash_full");
	int HUD_flash_beam = gHUD.GetSpriteIndex("flash_beam");

	m_hSprite1 = gHUD.GetSprite(HUD_flash_empty);
	m_hSprite2 = gHUD.GetSprite(HUD_flash_full);
	m_hBeam = gHUD.GetSprite(HUD_flash_beam);
	m_rc1 = gHUD.GetSpriteRect(HUD_flash_empty);
	m_rc2 = gHUD.GetSpriteRect(HUD_flash_full);
	m_rcBeam = gHUD.GetSpriteRect(HUD_flash_beam);
	m_iWidth = m_rc2.right - m_rc2.left;
};

int CHudFlashlight::MsgFunc_FlashBat(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x) / 100.0;

	return 1;
}

int CHudFlashlight::MsgFunc_Flashlight(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_fOn = READ_BYTE();
	gHUD.m_bUseZombVision = ( READ_BYTE() == 1 ) ? true : false;
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x) / 100.0;

	// Zombie team only
	if ( gEngfuncs.GetLocalPlayer()->curstate.team == ZP::TEAM_ZOMBIE )
		gEngfuncs.pEventAPI->EV_PlaySound( -1, gEngfuncs.GetLocalPlayer()->origin, 0, gHUD.m_bUseZombVision ? SOUND_ZOMBVISION_ON : SOUND_ZOMBVISION_OFF, 1.0, ATTN_NORM, 0, PITCH_NORM );

	return 1;
}

void CHudFlashlight::Draw(float flTime)
{
	if (gHUD.m_iHideHUDDisplay & (HIDEHUD_FLASHLIGHT | HIDEHUD_ALL))
		return;

	int r, g, b, x, y;
	float a;
	wrect_t rc;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return;

	// Don't draw if zombie team.
	// We use zombie vision, not a flashlight
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( localplayer->GetTeamNumber() == ZP::TEAM_ZOMBIE ) return;

	if (m_fOn)
		a = 225;
	else
		a = MIN_ALPHA;
	a *= gHUD.GetHudTransparency();

	if (m_flBat < 0.20)
		UnpackRGB(r, g, b, RGB_REDISH);
	else
		gHUD.GetHudColor(HudPart::Common, 0, r, g, b);

	ScaleColors(r, g, b, a);

	y = gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
	int xOffset = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).GetWidth() * 5 + gHUD.GetSpriteRect(gHUD.GetSpriteIndex("flash_full")).GetWidth() + 10;
	x = ScreenWidth - m_iWidth - m_iWidth / 2 - xOffset;

	// Draw the flashlight casing
	SPR_Set(m_hSprite1, r, g, b);
	SPR_DrawAdditive(0, x, y, &m_rc1);

	if (m_fOn)
	{ // draw the flashlight beam
		x = ScreenWidth - m_iWidth / 2 - xOffset;

		SPR_Set(m_hBeam, r, g, b);
		SPR_DrawAdditive(0, x, y, &m_rcBeam);
	}

	// draw the flashlight energy level
	x = ScreenWidth - m_iWidth - m_iWidth / 2 - xOffset;
	int iOffset = m_iWidth * (1.0 - m_flBat);
	if (iOffset < m_iWidth)
	{
		rc = m_rc2;
		rc.left += iOffset;

		SPR_Set(m_hSprite2, r, g, b);
		SPR_DrawAdditive(0, x + iOffset, y, &rc);
	}
}
