
#include "cbase.h"
#include "zp/zp_shared.h"
#include "zp/gamemodes/zp_gamemodebase.h"
#include "zp_spawnpoint_ent.h"
#ifdef SCRIPT_SYSTEM
#include "core.h"
#endif

LINK_ENTITY_TO_CLASS( info_player_team1, CBasePlayerSpawnPoint );
LINK_ENTITY_TO_CLASS( info_player_team2, CBasePlayerSpawnPoint );

void CBasePlayerSpawnPoint::Spawn()
{
#ifdef SCRIPT_SYSTEM
	// Outputs
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "OnTurnOn" );
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "OnTurnOff" );

	// Inputs
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "Activate" );
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "DeActivate" );

	SetEntityScriptCallback( &CBasePlayerSpawnPoint::OnScriptCallBack );
#endif
}

void CBasePlayerSpawnPoint::OnScriptCallBack( KeyValues *pData )
{
	const char *szAction = pData->GetString( "Action" );
	const char *szValue = pData->GetString( "arg0" );
	// Check what kind of action we got
	if ( FStrEq( szAction, "Activate" ) )
	{
		m_bEnabled = true;
		m_bOccupied = false;
		FireEntityOutput( this, "OnTurnOn" );
	}
	else if ( FStrEq( szAction, "DeActivate" ) )
	{
		m_bEnabled = false;
		FireEntityOutput( this, "OnTurnOff" );
	}
}

bool CBasePlayerSpawnPoint::HasSpawned()
{
	if ( m_bOccupied )
	{
		// Waiting for players to spawn in, so we don't spawn in the same spot
		// until the player has fully spawned in.
		if ( m_flLastOccupied == -1 ) return true;
		// If the time has not yet passed, then we are still occupied
		return ( m_flLastOccupied - gpGlobals->time ) > 0 ? true : false;
	}
	return false;
}

void CBasePlayerSpawnPoint::SetOccupied(bool bOccupied)
{
	m_bOccupied = bOccupied;
	// If we are occupied, set the timer to 3 seconds from now
	// This will prevent spawn camping, and players spawning on top of each other
	if ( ZP::GetCurrentRoundState() == ZP::RoundState::RoundState_RoundHasBegun )
		m_flLastOccupied = gpGlobals->time + 3.0f;
	else
		m_flLastOccupied = -1;
}

void CBasePlayerSpawnPoint::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "enabled" ) )
	{
		m_bEnabledRem = m_bEnabled = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CBasePlayerSpawnPoint::Restart()
{
	m_bEnabled = m_bEnabledRem;
	m_bOccupied = false;
}

void CBasePlayerSpawnPoint::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( useType == USE_TOGGLE )
		m_bEnabled = !m_bEnabled;
	else if ( useType == USE_ON )
		m_bEnabled = true;
	else if ( useType == USE_OFF )
		m_bEnabled = false;
}
