// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "eiface.h"
#include "zp/info_beacon.h"

// Our actual objective entity
LINK_ENTITY_TO_CLASS( info_beacon, CInfoBeacon );


extern int gmsgBeaconDraw;

void CInfoBeacon::Spawn() 
{
	m_bForcedOff = false;
	if ( FBitSet( pev->spawnflags, SP_START_ENABLED ) )
		m_bActive = true;
	else
		m_bActive = false;
	BaseClass::Spawn();
}

void CInfoBeacon::Restart()
{
	m_bForcedOff = false;
	if ( FBitSet( pev->spawnflags, SP_START_ENABLED ) )
		m_bActive = true;
	else
		m_bActive = false;
}

void CInfoBeacon::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "text_h" ) )
	{
		m_TextHuman = ALLOC_STRING(pkvd->szValue); // Make a copy of the next objective
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "text_z" ) )
	{
		m_TextZombie = ALLOC_STRING(pkvd->szValue); // Make a copy of the next objective
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "icon" ) )
	{
		m_Type = (BeaconTypes)atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "drawtype" ) )
	{
		m_DrawType = (BeaconDrawTypes)atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "range" ) )
	{
		m_Range = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "important" ) )
	{
		m_bImportant = atoi( pkvd->szValue ) != 0;
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "showhealth" ) )
	{
		m_bShowHealth = atoi( pkvd->szValue ) != 0;
		pkvd->fHandled = TRUE;
	}
	else
		BaseClass::KeyValue( pkvd );
}

void CInfoBeacon::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Toggle the beacon state
	if ( useType == USE_TOGGLE )
		m_bActive = !m_bActive;
	else if ( useType == USE_ON )
	{
		// On forced on, we can enable it again.
		// This is to allow mapmakers to turn it back on if they turned it off
		m_bActive = true;
		m_bForcedOff = false;
	}
	else if ( useType == USE_OFF )
	{
		m_bActive = false;
		m_bForcedOff = true;
	}

	// Cannot be enabled until next round if forced off.
	if ( m_bForcedOff && m_bActive )
		m_bActive = false;

	// Update our message state
	UpdateMessageState();
}

void CInfoBeacon::UpdateMessageState()
{
	BeaconTypes TypeCheck = m_Type;
	if ( m_teamfilter == 2 )
	{
		// If this beacon is for zombies only, reverse defend/destroy types.
		if ( m_Type == BEACON_DEFEND )
			TypeCheck = BEACON_DESTROY;
		else if ( m_Type == BEACON_DESTROY )
			TypeCheck = BEACON_DEFEND;
	}

	// Send our info to the client.
	// All they care about is our state and message.
	MESSAGE_BEGIN( MSG_ALL, gmsgBeaconDraw );
		WRITE_SHORT( ENTINDEX( edict() ) ); // Unique ID for this beacon, so we can update/remove it later. It's using the entity index of the info_beacon entity.
		WRITE_BYTE( m_bActive ? 1 : 0 ); // Is the beacon active or not
		WRITE_BYTE( m_bImportant ? 1 : 0 ); // If true, the beacon is drawn with golden colors
		WRITE_SHORT( (int)TypeCheck ); // Type of beacon, for the icon. Zombies have a different icon that reads from their own seperate file.
		WRITE_SHORT( (int)m_DrawType ); // When to draw the beacon
		WRITE_BYTE( m_bShowHealth ? 1 : 0 ); // If true, show health bar below the beacon icon. Very useful for defend, destroy and capture point beacons.
	    WRITE_SHORT( pev->health ); // Current health of the beacon, only used if m_bShowHealth is true
	    WRITE_SHORT( m_teamfilter ); // Team filter, 0 = all, 1 = humans only, 2 = zombies only (Only matters if we just want zombie or human only beacons)
		WRITE_COORD( pev->origin.x ); // World position of the beacon
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_FLOAT( m_Range ); // Range in units for the beacon to be visible, 0 = always visible
		static char szText[512];
	    szText[0] = 0;
		if ( FStringNull( m_TextHuman ) == FALSE )
			strncpy( szText, STRING(m_TextHuman), sizeof( szText ) );
	    else
			strncpy( szText, "Example Text", sizeof( szText ) );
		WRITE_STRING( szText ); // Text to display below the beacon icon

	    // If the zombie text isn't empty, use that instead.
	    if ( FStringNull( m_TextZombie ) == FALSE )
			strncpy( szText, STRING(m_TextZombie), sizeof( szText ) );
		WRITE_STRING( szText ); // Text to display below the beacon icon
	MESSAGE_END();
}