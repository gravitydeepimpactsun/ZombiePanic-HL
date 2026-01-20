// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "triggers.h"
#include "player.h"
#include "zp/info_beacon.h"
#ifdef SCRIPT_SYSTEM
#include "core.h"
#endif

#define SF_CHECK_ZOMBIES (1<<0)

class CTriggerCapturePoint : public CBaseTrigger
{
	SET_BASECLASS( CBaseTrigger );

public:
	void Spawn( void );
	void Restart( void );
	void OnCaptureThink( void );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) override;
	void OnCaptureTouch( CBaseEntity *pOther );
	bool IsEnabled() const { return m_Enabled; }
	void OnScriptCallBack( KeyValues *pData );
	inline void OnPlayerTouched( CBaseEntity *pPlayer )
	{
		for ( size_t i = 0; i < m_List.size(); i++ )
		{
			if ( m_List[i].client == ENTINDEX( pPlayer->edict() ) )
			{
				m_List[i].still_touching = gpGlobals->time + 0.15f;
				return;
			}
		}
		// New touch
		touchplayer_t newtouch;
		newtouch.client = ENTINDEX( pPlayer->edict() );
		newtouch.still_touching = gpGlobals->time + 0.15f;
		m_List.push_back( newtouch );
	}

private:
	struct touchplayer_t
	{
		int client; // The player
		float still_touching; // Are we still touching? GoldSrc doesn't really have a "EndTouch"
	};
	std::vector<touchplayer_t> m_List;

	bool m_CaptureBegun = false;
	bool m_Enabled = true;
	bool m_EnableRem = false;
	bool m_CheckZombies = true;
	float m_flNextCheck = 0.0f;
	float m_flCaptureAddPerTouch = 1.0f;
	float m_flCaptureDecreaseRate = 0.0f;
	string_t m_BeaconTargetName = 0;
};
LINK_ENTITY_TO_CLASS( trigger_capture_point, CTriggerCapturePoint );

void CTriggerCapturePoint::Spawn()
{
	CBaseEntity::Spawn();
	InitTrigger();
	SetTouch( &CTriggerCapturePoint::OnCaptureTouch );
	SetThink( &CTriggerCapturePoint::OnCaptureThink );

	pev->nextthink = gpGlobals->time + 1.0f;

#ifdef SCRIPT_SYSTEM
	// Inputs
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "TurnOn" );
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "TurnOff" );

	// Outputs
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "OnPointCaptureBegun" );
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "OnPointCapturing" );
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "OnPointCaptured" );
	ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, this, "OnPointContested" );

	SetEntityScriptCallback( &CInfoBeacon::OnScriptCallBack );
#endif
}

void CTriggerCapturePoint::OnCaptureThink()
{
	float flCaptureTime = 0.0f;
	float flTime = gpGlobals->time;
	for ( size_t i = 0; i < m_List.size(); i++ )
	{
		touchplayer_t &player = m_List[i];
		if ( player.still_touching - gpGlobals->time > 0 )
			flCaptureTime += 0.2f;
		else
			m_List.erase( m_List.begin() + i );
	}
	pev->nextthink = flTime + 0.1f;
	m_flCaptureDecreaseRate = clamp( flCaptureTime, 0.0f, 1.5f );
}

void CTriggerCapturePoint::OnScriptCallBack( KeyValues *pData )
{
	const char *szAction = pData->GetString( "Action" );
	const char *szValue = pData->GetString( "arg0" );
	// Check what kind of action we got
	if ( FStrEq( szAction, "TurnOn" ) )
		Use( nullptr, nullptr, USE_ON, 0 );
	else if ( FStrEq( szAction, "TurnOff" ) )
		Use( nullptr, nullptr, USE_OFF, 0 );
}

void CTriggerCapturePoint::Restart()
{
	m_Enabled = m_EnableRem;
	m_CheckZombies = true;
	m_flNextCheck = gpGlobals->time + 2.0f;
}

void CTriggerCapturePoint::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "enabled" ) )
	{
		m_EnableRem = m_Enabled = atoi(pkvd->szValue) > 0 ? true : false;
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "beacon" ) )
	{
		m_BeaconTargetName = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "progress" ) )
	{
		m_flCaptureAddPerTouch = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue( pkvd );
}

void CTriggerCapturePoint::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Toggle the states
	if ( useType == USE_TOGGLE )
		m_Enabled = !m_Enabled;
	else if ( useType == USE_ON )
		m_Enabled = true;
	else if ( useType == USE_OFF )
		m_Enabled = false;
	m_flNextCheck = gpGlobals->time + 2.0f;

	// Make sure we update the beacon state as well.
	CInfoBeacon *pFind = (CInfoBeacon *)UTIL_FindEntityByTargetname( nullptr, STRING( m_BeaconTargetName ) );
	if ( pFind )
		pFind->Use( pActivator, pCaller, m_Enabled ? USE_TYPE::USE_ON : USE_TYPE::USE_OFF, value );
	else
		Warning( "ERROR: %s does not have an info_beacon attached!\n", STRING( pev->targetname ) );
}

void CTriggerCapturePoint::OnCaptureTouch( CBaseEntity *pOther )
{
	if ( !m_Enabled ) return;
	if ( !pOther->IsPlayer() ) return;
	if ( !pOther->IsAlive() ) return;

	if (FBitSet(pev->spawnflags, SF_CHECK_ZOMBIES))
	{
		// A very simple "are there any zombies blocking this trigger" check.
		if ( m_CheckZombies )
		{
			// If we hit 0, or less, then turn this off
			if ( m_flNextCheck - gpGlobals->time <= 0 )
			{
				m_CheckZombies = false;
				return;
			}
			// We found a zombie! Delay again by 2 more seconds
			if ( pOther->pev->team == ZP::TEAM_ZOMBIE )
			{
#ifdef SCRIPT_SYSTEM
				const std::string &szOutput( "OnPointContested" );
				ScriptSystem::CallScriptDelay(
					AvailableScripts_t::InputOutput,
					nullptr,
					szOutput,
					0.0f,
					2,
					std::to_string( entindex() ),
					std::to_string( pOther ? pOther->entindex() : entindex() )
				);
#endif
				m_flNextCheck = gpGlobals->time + 2.0f;
			}
			return;
		}
	}

	if ( pOther->pev->team != ZP::TEAM_SURVIVIOR )
	{
		// We found a zombie, enable the checks again.
		if ( pOther->pev->team == ZP::TEAM_ZOMBIE )
		{
			m_flNextCheck = gpGlobals->time + 2.0f;
			m_CheckZombies = true;
#ifdef SCRIPT_SYSTEM
			const std::string &szOutput( "OnPointContested" );
			ScriptSystem::CallScriptDelay(
				AvailableScripts_t::InputOutput,
				nullptr,
				szOutput,
				0.0f,
				2,
				std::to_string( entindex() ),
				std::to_string( pOther ? pOther->entindex() : entindex() )
			);
#endif
		}
		return;
	}

	if ( m_flNextCheck - gpGlobals->time > 0 ) return;

	CInfoBeacon *pFind = (CInfoBeacon *)UTIL_FindEntityByTargetname( nullptr, STRING( m_BeaconTargetName ) );
	if ( pFind )
	{
		// We grab the beacon's health as our capture progress.
		float nCaptureProgress = clamp( pFind->pev->health, 0, 100 );

		// If it reaches 100, we capture it.
		if ( nCaptureProgress >= 100 )
		{
			// Disable this trigger as well.
			m_Enabled = false;
			m_CaptureBegun = false;
			// Fire off any targets.
			SUB_UseTargets( pOther, USE_TYPE::USE_ON, 0.0f );
#ifdef SCRIPT_SYSTEM
			KeyValuesAD pKvNew( new KeyValues( "Items" ) );
			pKvNew->SetString( "Action", "TurnOff" );
			pKvNew->SetString( "arg0", "" );
			pFind->ScriptCallback( pKvNew );

			const std::string &szOutput( "OnPointCaptured" );
			ScriptSystem::CallScriptDelay(
				AvailableScripts_t::InputOutput,
				nullptr,
				szOutput,
				0.0f,
				2,
				std::to_string( entindex() ),
				std::to_string( pOther ? pOther->entindex() : entindex() )
			);
#endif
		}
		else
		{
			// Increase the capture progress by X per touch.
			nCaptureProgress += m_flCaptureAddPerTouch;
			pFind->UpdateHealth( nCaptureProgress );
#ifdef SCRIPT_SYSTEM
			KeyValuesAD pKvNew( new KeyValues( "Items" ) );
			pKvNew->SetString( "Action", "ShowBar" );
			pKvNew->SetString( "arg0", "1" );
			pFind->ScriptCallback( pKvNew );

			const std::string &szOutput( m_CaptureBegun ? "OnPointCapturing" : "OnPointCaptureBegun" );
			ScriptSystem::CallScriptDelay(
				AvailableScripts_t::InputOutput,
				nullptr,
				szOutput,
				0.0f,
				2,
				std::to_string( entindex() ),
				std::to_string( pOther ? pOther->entindex() : entindex() )
			);
#endif
			m_CaptureBegun = true;
		}
	}

	m_flNextCheck = gpGlobals->time + 2.0f - m_flCaptureDecreaseRate;

	// When the player has touched, update our touch list.
	OnPlayerTouched( pOther );
}

void UTIL_EnableTriggerCapturePoints()
{
	CBaseEntity *pEnt = nullptr;
	while ( ( pEnt = UTIL_FindEntityByClassname( pEnt, "trigger_capture_point" ) ) != nullptr )
	{
		CTriggerCapturePoint *pTrigger = static_cast<CTriggerCapturePoint *>( pEnt );
		if ( pTrigger && pTrigger->IsEnabled() )
			pTrigger->Use( nullptr, nullptr, USE_TYPE::USE_ON, 0.0f );
	}
}