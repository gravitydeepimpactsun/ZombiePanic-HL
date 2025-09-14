// ============== Copyright (c) 2025 Monochrome Games ============== \\


#ifndef HUD_BEACONS_H
#define HUD_BEACONS_H
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include "zp/zp_shared.h"
#include "../../hud/base.h"

static const int BEACON_ICON_SIZE = 64;

enum BeaconScreenDrawState_t
{
	DRAWSTATE_NO = 0, // Not drawn at all
	DRAWSTATE_YES, // Drawn normally
	DRAWSTATE_EDGE, // Drawn at the edge of the screen
	DRAWSTATE_BEHIND, // Behind the player, drawn at the edge of the screen
};

class CZPBeacons : public CHudElemBase<CZPBeacons>, public vgui2::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CZPBeacons, vgui2::Panel );

	struct DrawData
	{
		int x = 0, y = 0; // Screen position to draw at
		bool visible = false; // Is the beacon visible or not
		int textureID = -1; // Texture ID of the icon
	};

	struct BeaconData
	{
		int id; // Unique ID for this beacon, so we can update/remove it later. It's using the entity index of the info_beacon entity.
		std::string text; // Text to display below the beacon icon
		std::string text_zombie; // Optional text for zombies, if not set, it will use the human text.
		BeaconTypes type; // Type of beacon, for the icon. Zombies have a different icon that reads from their own seperate file.
		BeaconDrawTypes drawtype; // How to draw the beacon
		bool drawHealth; // If true, show health bar below the beacon icon. Very useful for defend, destroy and capture point beacons.
		int health; // Current health of the beacon, only used if drawHealth is true
		int teamfilter; // Team filter, 0 = all, 1 = humans only, 2 = zombies only (Only matters if we just want zombie or human only beacons)
		Vector position; // World position of the beacon
		float range; // Range in units for the beacon to be visible, 0 = always visible
		bool active; // Is the beacon active or not
		bool important; // If true, the beacon is drawn with golden colors
		DrawData drawData; // Data calculated each frame for drawing
	};

	CZPBeacons();

	void Init();
	void VidInit();
	void Paint();

	virtual void DrawPositions();
	int GetBeaconTextureID( const char *szIcon ) const;

	void AddBeacon( BeaconData beacon );
	inline void ResetBeacons() { m_Beacons.clear(); }

private:
	virtual void ApplySchemeSettings( vgui2::IScheme *pScheme );

	std::vector<BeaconData> m_Beacons;
	vgui2::HFont m_hBeaconText;
};

#endif
