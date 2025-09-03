// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "com_model.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

#include "zp_beacons.h"
#include "vgui/client_viewport.h"

DEFINE_HUD_ELEM( CZPBeacons );

static const char *g_BeaconTypeIcons[] =
{
	"ui/icons/beacons/button",
	"ui/icons/beacons/defend",
	"ui/icons/beacons/destroy",
	"ui/icons/beacons/waypoint",
	"ui/icons/beacons/capturepoint",
};

static const char *g_BeaconTypeIcons_Zombie[] =
{
	"ui/icons/beacons/zp_button",
	"ui/icons/beacons/zp_defend",
	"ui/icons/beacons/zp_destroy",
	"ui/icons/beacons/zp_waypoint",
	"ui/icons/beacons/capturepoint",
};

struct BeaconTexture
{
	std::string iconPath;
	int textureID;
};
static std::vector<BeaconTexture> g_BeaconTextures;

CZPBeacons::CZPBeacons()
    : vgui2::Panel(NULL, "ZPBeacons")
{
	SetParent( g_pViewport );
	SetScheme( "ClientScheme" );
	m_hBeaconText = vgui2::INVALID_FONT;
}

void CZPBeacons::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor( Color(0, 0, 0, 0) );
	m_hBeaconText = pScheme->GetFont( "Default", true );
}

void CZPBeacons::Init()
{
	BaseHudClass::Init();
	m_Beacons.clear();

	// For our label & icon panels to be drawn correctly, we need to be the size of the screen
	int w, t;
	vgui2::surface()->GetScreenSize( w, t );
	SetWide( w );
	SetTall( t );
}

void CZPBeacons::VidInit()
{
	BaseHudClass::VidInit();
	m_Beacons.clear();
}

void CZPBeacons::Paint()
{
	// Draw all active beacons
	for ( size_t i = 0; i < m_Beacons.size(); i++ )
	{
		BeaconData &beacon = m_Beacons[i];
		if ( !beacon.drawData.visible ) continue;
		int r = 255, g = 255, b = 255;
		int x = beacon.drawData.x;
		int y = beacon.drawData.y;
		if ( beacon.important )
			r = 255, g = 215, b = 0;

		// Draw the icon
		int iconSize = 64;
		int iconX = x - iconSize / 2;
		// Debug: draw a box where the icon is
		//vgui2::surface()->DrawSetColor( r, g, b, 100 );
		//vgui2::surface()->DrawFilledRect( iconX, y, iconX + iconSize, y + iconSize );

		vgui2::surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		vgui2::surface()->DrawSetTexture( beacon.drawData.textureID );
		vgui2::surface()->DrawTexturedRect( iconX, y, iconX + iconSize, y + iconSize );

		// Draw the text
		const char *szText = beacon.text.c_str();
		if ( gEngfuncs.GetLocalPlayer()->curstate.team == ZP::TEAM_ZOMBIE && !beacon.text_zombie.empty() )
			szText = beacon.text_zombie.c_str();

		// Now calculate the text size
		int textWidth, textHeight;
		gEngfuncs.pfnDrawConsoleStringLen( szText, &textWidth, &textHeight );

		int textX = x - textWidth / 2;
		int textY = y + iconSize + 5;

		vgui2::surface()->DrawSetTextFont( m_hBeaconText );
		vgui2::surface()->DrawSetTextColor( r, g, b, 255 );
		vgui2::surface()->DrawSetTextPos( textX, textY );

		wchar_t wsText[256];
		g_pVGuiLocalize->ConvertANSIToUnicode( szText, wsText, sizeof( wsText ) );
		vgui2::surface()->DrawPrintText( wsText, wcslen(wsText) );
	}
}

void CZPBeacons::DrawPositions()
{
	// Get the local player origin
	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
	if ( !pPlayer ) return;
	Vector v_origin = pPlayer->origin;
	Vector screen;
	// Draw all active beacons
	for ( size_t i = 0; i < m_Beacons.size(); i++ )
	{
		BeaconData &beacon = m_Beacons[i];
		if ( !beacon.active ) continue;
		bool bShouldDraw = true;
		if ( beacon.range > 0 )
		{
			float dist = ( beacon.position - v_origin ).Length();
			if ( dist > beacon.range )
				bShouldDraw = false;
		}

		// Project to screen
		gEngfuncs.pTriAPI->WorldToScreen( beacon.position, screen );
		screen[0] = XPROJECT( screen[0] );
		screen[1] = YPROJECT( screen[1] );
		screen[2] = 0.0f;

		// If this is true, make sure the beacon X,Y cords are on screen
		if ( bShouldDraw )
		{
			if ( screen[0] < 0 || screen[0] > GetWide() || screen[1] < 0 || screen[1] > GetTall() )
				bShouldDraw = false;
			// screen Z isn't really working here, so we need to use a different method
			// If the beacon is behind the player, don't draw it
			Vector vDir = beacon.position - v_origin;
			vDir.NormalizeInPlace();
			Vector vForward;
			AngleVectors( pPlayer->angles, vForward, NULL, NULL );
			float flDot = vDir.Dot( vForward );
			if ( flDot < 0.1f ) // If the angle is more than ~84 degrees, don't draw it
				bShouldDraw = false;
		}

		beacon.drawData.visible = bShouldDraw;
		if ( !bShouldDraw ) continue;

		int x = static_cast<int>( screen.x );
		int y = static_cast<int>( screen.y );

		char szIconPath[256];
		szIconPath[0] = '\0';
		const char *iconPath = g_BeaconTypeIcons[ beacon.type ];
		if ( gEngfuncs.GetLocalPlayer()->curstate.team == ZP::TEAM_ZOMBIE )
			iconPath = g_BeaconTypeIcons_Zombie[ beacon.type ];

		sprintf( szIconPath, "%s%s", iconPath, beacon.important ? "_main" : "" );

		beacon.drawData.x = x;
		beacon.drawData.y = y;
		beacon.drawData.textureID = GetBeaconTextureID( szIconPath );
	}
}

int CZPBeacons::GetBeaconTextureID( const char *szIcon ) const
{
	for ( size_t i = 0; i < g_BeaconTextures.size(); i++ )
	{
		if ( g_BeaconTextures[i].iconPath == szIcon )
			return g_BeaconTextures[i].textureID;
	}
	int hTex = vgui2::surface()->CreateNewTextureID();
	vgui2::surface()->DrawSetTextureFile( hTex, szIcon, true, false );
	BeaconTexture newTex;
	newTex.iconPath = szIcon;
	newTex.textureID = hTex;
	g_BeaconTextures.push_back( newTex );
	return hTex;
}

void CZPBeacons::AddBeacon( BeaconData beacon )
{
	// Check if we already have this beacon, if so, update it
	for ( size_t i = 0; i < m_Beacons.size(); i++ )
	{
		if ( m_Beacons[i].id == beacon.id )
		{
			m_Beacons[i] = beacon;
			return;
		}
	}
	// Add new beacon
	m_Beacons.push_back( beacon );
}
