// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "ag_playerid.h"
#include "ag_global.h"
#include "hud/ammohistory.h"

#define MAX_ID_STRING   256
#define COLOR_GREEN		Color(0, 255, 0)
#define COLOR_YELLOW	Color(255, 255, 0)
#define COLOR_ORANGE	Color(255, 128, 0)
#define COLOR_RED		Color(255, 0, 0)

DEFINE_HUD_ELEM(AgHudPlayerId);

void AgHudPlayerId::Init()
{
	HookMessage<&AgHudPlayerId::MsgFunc_PlayerId>("PlayerId");

	m_iFlags = 0;
	m_flTurnoff = 0.0;
	m_iPlayer = 0;
	m_bTeam = false;
	m_iHealth = 0;
	m_iArmour = 0;

	m_pCvarHudPlayerId = gEngfuncs.pfnRegisterVariable("hud_playerid", "1", FCVAR_BHL_ARCHIVE);
}

void AgHudPlayerId::VidInit()
{
}

void AgHudPlayerId::Reset(void)
{
	m_iFlags &= ~HUD_ACTIVE;
	m_iPlayer = 0;
}

void AgHudPlayerId::DrawSurvivorID( CPlayerInfo *pInfo )
{
	int nHealth = max( 0, m_iHealth );
	int nArmour = max( 0, m_iArmour );
	Color clr;
	if ( nHealth > 75 )
		clr = COLOR_GREEN;
	else if ( nHealth > 50 )
		clr = COLOR_YELLOW;
	else if ( nHealth > 35 )
		clr = COLOR_ORANGE;
	else
		clr = COLOR_RED;
	int ypos = ScreenHeight - ScreenHeight / 4 + 25;

	// Draw the name of the player (the color being the health)
	DrawText( pInfo->GetDisplayName(), ypos, clr );

	char szText[MAX_ID_STRING];
	szText[0] = '\0';

	// Draw the inventory
	bool bHasWeapons = false;
	for ( int i = 1; i < MAX_WEAPON_SLOTS; i++ )
	{
		WEAPON *pWeapon = gWR.GetWeaponBySlot( i );
		if ( !pWeapon->bHasWeapon ) continue;
		if ( !bHasWeapons )
		{
			sprintf( szText, "%s", pWeapon->szName );
			bHasWeapons = true;
		}
		else
			sprintf( szText, "%s, %s", szText, pWeapon->szName );
	}

	if ( bHasWeapons && gHUD.m_GameMode != ZP::GAMEMODE_HARDCORE )
	{
		ypos += 30;
		int r, g, b;
		UnpackRGB( r, g, b, RGB_GREENISH );
		DrawText( szText, ypos, Color( r, g, b ) );
	}

	// Draw the armor
	if ( nArmour <= 0 ) return;
	szText[0] = '\0';
	ypos += 30;
	sprintf( szText, "Armor: %d%%", nArmour );
	if ( nArmour > 75 )
		clr = COLOR_GREEN;
	else if ( nArmour > 50 )
		clr = COLOR_YELLOW;
	else if ( nArmour > 25 )
		clr = COLOR_ORANGE;
	else
		clr = COLOR_RED;
	DrawText( szText, ypos, clr );
}

void AgHudPlayerId::DrawZombieID( CPlayerInfo *pInfo )
{
	int r, g, b;
	UnpackRGB( r, g, b, RGB_GREENISH );
	int ypos = ScreenHeight - ScreenHeight / 4;
	DrawText( pInfo->GetDisplayName(), ypos, Color( r, g, b ) );

	int nHealth = max( 0, m_iHealth );
	Color clr;
	if ( nHealth > 160 )
		clr = COLOR_GREEN;
	else if ( nHealth > 120 )
		clr = COLOR_YELLOW;
	else if ( nHealth > 80 )
		clr = COLOR_ORANGE;
	else
		clr = COLOR_RED;
	ypos += 30;

	char szText[MAX_ID_STRING];
	const int iMaxHealth = ZP::MaxHealth[ 1 ]; // Grab the max health for the zombie
	const float flHealthPercentage = static_cast<float>( nHealth ) / static_cast<float>( iMaxHealth ) * 100.0f;
	sprintf( szText, "%d / %d (%.0f %%)", nHealth, iMaxHealth, flHealthPercentage );
	DrawText( szText, ypos, clr );
}

void AgHudPlayerId::DrawText(const char *pszText, const int &ypos, const Color &rgb)
{
	int r, g, b, a;
	r = rgb.r();
	g = rgb.g();
	b = rgb.b();
	a = 255 * gHUD.GetHudTransparency();
	ScaleColors( r, g, b, a );
	char szText[MAX_ID_STRING];
	sprintf( szText, "%s", pszText );
	AgDrawHudStringCentered( ScreenWidth / 2, ypos, ScreenWidth, szText, r, g, b );
}

void AgHudPlayerId::Draw(float fTime)
{
	if (m_iPlayer <= 0 || m_pCvarHudPlayerId->value == 0)
		return;

	if ( gHUD.m_flTime > m_flTurnoff )
	{
		Reset();
		return;
	}

	// Not on the same team, ignore the player.
	if ( !m_bTeam ) return;

	CPlayerInfo *pi = GetPlayerInfo( m_iPlayer )->Update();
	if ( pi->IsConnected() )
	{
		if ( pi->GetTeamNumber() == ZP::TEAM_ZOMBIE )
			DrawZombieID( pi );
		else
			DrawSurvivorID( pi );
	}
}

int AgHudPlayerId::MsgFunc_PlayerId(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iPlayer = READ_BYTE();
	m_bTeam = READ_BYTE() == 1;
	m_iHealth = READ_SHORT();
	m_iArmour = READ_SHORT();

	if (m_pCvarHudPlayerId->value == 0)
		m_iFlags &= ~HUD_ACTIVE;
	else
		m_iFlags |= HUD_ACTIVE;

	m_flTurnoff = gHUD.m_flTime + 2; // Hold for 2 seconds.

	return 1;
}
