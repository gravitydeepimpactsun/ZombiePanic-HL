// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "event_api.h"
#include "triangleapi.h"
#include "com_model.h"
#include "pmtrace.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

#include "zp_beacons.h"
#include "vgui/client_viewport.h"

DEFINE_HUD_ELEM( CZPBeacons );

#define MAX_BEACON_TYPES 5

static const char *g_BeaconTypeIcons[MAX_BEACON_TYPES] =
{
	"ui/icons/beacons/button",
	"ui/icons/beacons/defend",
	"ui/icons/beacons/destroy",
	"ui/icons/beacons/waypoint",
	"ui/icons/beacons/capturepoint",
};

static const char *g_BeaconTypeIcons_Zombie[MAX_BEACON_TYPES] =
{
	"ui/icons/beacons/zp_button",
	"ui/icons/beacons/zp_destroy",
	"ui/icons/beacons/zp_defend",
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

	PrecacheImages();
}

void CZPBeacons::VidInit()
{
	BaseHudClass::VidInit();
	m_Beacons.clear();
	PrecacheImages();
}

void CZPBeacons::PrecacheImages()
{
	// Precache all beacon icons
	g_BeaconTextures.clear();
	for ( int i = 0; i < MAX_BEACON_TYPES; i++ )
	{
		// Normal icons
		GetBeaconTextureID( g_BeaconTypeIcons[i] );
		GetBeaconTextureID( g_BeaconTypeIcons_Zombie[i] );

		// Human main icons
		char szIconPath[256];
		szIconPath[0] = '\0';
		sprintf( szIconPath, "%s_main", g_BeaconTypeIcons[ i ] );
		GetBeaconTextureID( szIconPath );

		// Zombie main icons
		szIconPath[0] = '\0';
		sprintf( szIconPath, "%s_main", g_BeaconTypeIcons[ i ] );
		GetBeaconTextureID( szIconPath );
	}
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
		int iconX = x - BEACON_ICON_SIZE / 2;
		// Debug: draw a box where the icon is
		//vgui2::surface()->DrawSetColor( r, g, b, 100 );
		//vgui2::surface()->DrawFilledRect( iconX, y, iconX + BEACON_ICON_SIZE, y + BEACON_ICON_SIZE );

		vgui2::surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		vgui2::surface()->DrawSetTexture( beacon.drawData.textureID );
		vgui2::surface()->DrawTexturedRect( iconX, y, iconX + BEACON_ICON_SIZE, y + BEACON_ICON_SIZE );

		// Draw the text
		const char *szText = beacon.text.c_str();
		if ( gEngfuncs.GetLocalPlayer()->curstate.team == ZP::TEAM_ZOMBIE && !beacon.text_zombie.empty() )
			szText = beacon.text_zombie.c_str();

		// Now calculate the text size
		int textWidth, textHeight;
		gEngfuncs.pfnDrawConsoleStringLen( szText, &textWidth, &textHeight );

		int textX = x - textWidth / 2;
		int textY = y + BEACON_ICON_SIZE + 5;

		vgui2::surface()->DrawSetTextFont( m_hBeaconText );
		vgui2::surface()->DrawSetTextColor( r, g, b, 255 );
		vgui2::surface()->DrawSetTextPos( textX, textY );

		wchar_t wsText[256];
		g_pVGuiLocalize->ConvertANSIToUnicode( szText, wsText, sizeof( wsText ) );
		vgui2::surface()->DrawPrintText( wsText, wcslen(wsText) );

		// Draw the health bar if needed
		if ( beacon.drawHealth )
		{
			int barWidth = 100;
			int barHeight = 5;
			int barX = x - barWidth / 2;
			int barY = textY + textHeight + 5;
			// Draw the background
			vgui2::surface()->DrawSetColor( Color( 0, 0, 0, 200 ) );
			vgui2::surface()->DrawFilledRect( barX, barY, barX + barWidth, barY + barHeight );
			// Draw the health
			int healthWidth = ( beacon.health * barWidth ) / 100;
			vgui2::surface()->DrawSetColor( Color( 255, 0, 0, 200 ) );
			vgui2::surface()->DrawFilledRect( barX, barY, barX + healthWidth, barY + barHeight );
			// Draw the border
			vgui2::surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
			vgui2::surface()->DrawOutlinedRect( barX, barY, barX + barWidth, barY + barHeight );
		}
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
		BeaconScreenDrawState_t drawState = DRAWSTATE_YES;
		if ( beacon.range > 0 )
		{
			float dist = ( beacon.position - v_origin ).Length();
			if ( dist > beacon.range )
				drawState = DRAWSTATE_NO;
		}

		switch ( beacon.drawtype )
		{
			// Check if the beacon is visible using a simple tracer from beacon.position to v_origin
			case BEACON_DRAW_COLLUDED:
			{
				pmtrace_t tr;
				gEngfuncs.pEventAPI->EV_SetTraceHull( 2 ); // Point hull
				gEngfuncs.pEventAPI->EV_PlayerTrace( beacon.position, v_origin, 0xFFFFFFFF, -1, &tr );
				if ( tr.fraction < 0.99f ) // If we hit something, the beacon is not visible
				    drawState = DRAWSTATE_NO;
			}
		    break;
			// The same code as above, but inverted
			case BEACON_DRAW_UNCOLLUDED:
			{
				pmtrace_t tr;
				gEngfuncs.pEventAPI->EV_SetTraceHull( 2 ); // Point hull
				gEngfuncs.pEventAPI->EV_PlayerTrace( beacon.position, v_origin, 0xFFFFFFFF, -1, &tr );
				if ( tr.fraction < 0.99f ) // If we hit something, the beacon is visible
				    drawState = DRAWSTATE_NO;
			}
		    break;
		}

		// Project to screen
		gEngfuncs.pTriAPI->WorldToScreen( beacon.position, screen );
		screen[0] = XPROJECT( screen[0] );
		screen[1] = YPROJECT( screen[1] );
		screen[2] = 0.0f;

		// If we have a team filter, check if we should draw it
		if ( beacon.teamfilter == 1 && pPlayer->curstate.team != ZP::TEAM_SURVIVIOR )
			drawState = DRAWSTATE_NO;
		else if ( beacon.teamfilter == 2 && pPlayer->curstate.team != ZP::TEAM_ZOMBIE )
			drawState = DRAWSTATE_NO;

		// If this is true, make sure the beacon X,Y cords are on screen
		if ( drawState != DRAWSTATE_NO )
		{
			// If the beacon is off screen, draw it on the edge of the screen
			if ( screen[0] < 0 || screen[0] > ( GetWide() - BEACON_ICON_SIZE ) || screen[1] < 0 || screen[1] > ( GetTall() - BEACON_ICON_SIZE ) )
				drawState = DRAWSTATE_EDGE;
			// screen Z isn't really working here, so we need to use a different method
			// If the beacon is behind the player, don't draw it
			Vector vDir = beacon.position - v_origin;
			vDir.NormalizeInPlace();
			Vector vForward;
			AngleVectors( pPlayer->angles, vForward, NULL, NULL );
			float flDot = vDir.Dot( vForward );
			if ( flDot < 0.1f ) // If the angle is more than ~84 degrees, draw it on the edge of the screen
				drawState = DRAWSTATE_BEHIND;
		}

		beacon.drawData.visible = ( drawState > DRAWSTATE_NO ) ? true : false;
		if ( drawState == DRAWSTATE_NO ) continue;

		int x = static_cast<int>( screen.x );
		int y = static_cast<int>( screen.y );

		// A simple switch statement to clamp the beacon to the edge of the screen if needed
		// And if we are behind the player, make sure we draw it at the bottom of the screen (or top if inverted)
		switch (drawState)
		{
			case DRAWSTATE_EDGE:
			{
				if (x < 0)
				    x = BEACON_ICON_SIZE;
			    if (x > GetWide() - BEACON_ICON_SIZE)
				    x = GetWide() - BEACON_ICON_SIZE;
				if (y < 0)
				    y = 0;
			    if (y > GetTall() - BEACON_ICON_SIZE)
				    y = GetTall() - BEACON_ICON_SIZE;
			}
			break;
			// The beacon is behind the player, let's allow it to move around the edge of the screen
			case DRAWSTATE_BEHIND:
			{
				// If the beacon is to the left of the player, draw it on the right side of the screen
				if ( x < GetWide() / 2 )
					x = GetWide() - BEACON_ICON_SIZE;
				else
					x = BEACON_ICON_SIZE;
				// If the beacon is above the player, draw it at the bottom of the screen
				if ( y < GetTall() / 2 )
					y = GetTall() - BEACON_ICON_SIZE;
				else
				    y = 0;
			}
			break;
		}

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
