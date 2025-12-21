
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
	// Check if this is a human spawn or not
	if ( FStrEq( STRING(pev->classname), "info_player_team1" ) )
		m_bHumanSpawn = true;
	else
		m_bHumanSpawn = false;
}

void CBasePlayerSpawnPoint::OnScriptCallBack( KeyValues *pData )
{
	const char *szAction = pData->GetString( "Action" );
	const char *szValue = pData->GetString( "arg0" );
	// Check what kind of action we got
	if ( FStrEq( szAction, "Activate" ) )
	{
		m_bEnabled = true;
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
	if ( !m_bEnabled ) return true;
	if ( m_flDisableTime != -1 )
	{
		if ( m_flDisableTime - gpGlobals->time > 0.0f ) return true;
		m_flDisableTime = -1;
	}
	return false;
}

void CBasePlayerSpawnPoint::DisableSpawn()
{
	m_flDisableTime = gpGlobals->time + 5.0f; // Disabled for 5 seconds, if we step inside the spawn blocker
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
	m_flDisableTime = -1;
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
