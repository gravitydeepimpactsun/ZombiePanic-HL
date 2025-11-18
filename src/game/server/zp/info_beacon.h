// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_INFO_BEACON_BASE
#define SERVER_INFO_BEACON_BASE
#pragma once

#include "cbase.h"
#include "zp/zp_shared.h"

#define SP_START_ENABLED (1<<0)

class CInfoBeacon : public CPointEntity
{
	SET_BASECLASS( CPointEntity );

public:
	void Spawn( void );
	void Restart();
	void KeyValue( KeyValueData *pkvd ) override;
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void UpdateMessageState();
	void UpdateMessageStateForEntity( CBasePlayer *pPlayer );
	virtual int ObjectCaps(void) { return CBaseEntity::ObjectCaps() | FCAP_MUST_RESET; }
	bool IsActive() const { return m_bActive && !m_bForcedOff; }
	inline void UpdateHealth( int health ) { pev->health = clamp( health, 0, 100 ); UpdateMessageState(); }
	void OnScriptCallBack( KeyValues *pData );

private:
	BeaconTypes m_Type = BEACON_BUTTON; // Type of beacon, for the icon. Zombies have a different icon that reads from their own seperate file.
	string_t m_TextHuman = 0; // Text to display below the beacon icon
	string_t m_TextZombie = 0; // Optional text for zombies, if not set, it will use the human text.
	float m_Range = 0.0f; // Range in hammer units for the beacon to be visible, 0 = always visible
	BeaconDrawTypes m_DrawType = BEACON_DRAW_ALWAYS; // When to draw the beacon
	bool m_bImportant = false; // If true, draw with golden colors
	bool m_bShowHealth = false; // If true, show health bar below the beacon icon. Very useful for defend, destroy and capture point beacons.
	bool m_bShowHealthRem = false; // Ditto, for Restart();
	bool m_bActive = false; // Is the beacon active or not
	bool m_bForcedOff = false; // If true, the beacon is forced off and cannot be enabled until the next round.
};

#endif