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
	m_iFlags = 0;
	m_iPlayer = 0;
	m_iHealth = 0;
	m_iArmour = 0;
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
	int ypos = 1.0f;

	// Draw the name of the player (the color being the health)
	DrawText( pInfo->GetDisplayName(), ypos, clr );

	char szText[MAX_ID_STRING];
#if 0
	szText[0] = '\0';

	// Draw the inventory
	bool bHasWeapons = false;
	for ( int i = 1; i < MAX_WEAPON_SLOTS; i++ )
	{
		WEAPON *pWeapon = gWR.GetWeaponBySlot( i );
		if ( !pWeapon ) continue;
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
		ypos += 1.0f;
		int r, g, b;
		UnpackRGB( r, g, b, RGB_GREENISH );
		DrawText( szText, ypos, Color( r, g, b ) );
	}
#endif

	// Draw the armor
	if ( nArmour <= 0 ) return;
	szText[0] = '\0';
	ypos += 1.5f;
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
	int ypos = 1.0f;
	DrawText( pInfo->GetDisplayName(), ypos, Color( r, g, b ) );

	int nHealth = max( 0, m_iHealth );
	Color clr;
	if ( nHealth > 75 )
		clr = COLOR_GREEN;
	else if ( nHealth > 50 )
		clr = COLOR_YELLOW;
	else if ( nHealth > 35 )
		clr = COLOR_ORANGE;
	else
		clr = COLOR_RED;
	ypos += 1.5f;

	char szText[MAX_ID_STRING];
	sprintf( szText, "%d%%", nHealth );
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

	int TextHeight, TextWidth;
	GetConsoleStringSize( szText, &TextWidth, &TextHeight );

	int x = ScreenWidth / 2;
	int y = (ScreenHeight / 2) + (TextHeight * ypos);
	AgDrawHudStringCentered( x, y, ScreenWidth, szText, r, g, b );
}

void AgHudPlayerId::Draw(float fTime)
{
	if ( m_iPlayer <= 0 )
	{
		Reset();
		return;
	}

	CPlayerInfo *pi = GetPlayerInfo( m_iPlayer )->Update();
	CPlayerInfo *localPlayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( pi->IsConnected() && localPlayer->GetTeamNumber() == pi->GetTeamNumber() )
	{
		if ( pi->GetTeamNumber() == ZP::TEAM_ZOMBIE )
			DrawZombieID( pi );
		else
			DrawSurvivorID( pi );
	}
}

void AgHudPlayerId::SetPlayerID( int iPlayerID, int iHealth, int iArmor )
{
	m_iPlayer = iPlayerID;
	m_iHealth = iHealth;
	m_iArmour = iArmor;
	if ( iPlayerID == 0 )
		m_iFlags &= ~HUD_ACTIVE;
	else
		m_iFlags |= HUD_ACTIVE;
}
