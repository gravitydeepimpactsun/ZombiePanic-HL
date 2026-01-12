// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "triggers.h"
#include "player.h"
#include "zp/gamemodes/zp_gamemodebase.h"

#define SF_CHECK_ZOMBIES 1

class CTriggerEndRound : public CBaseTrigger
{
public:
	void Spawn(void);
	void Restart(void);
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) override;
	void OnEndRoundTouch(CBaseEntity *pOther);

private:
	bool m_Enabled = true;
	bool m_EnableRem = false;
	bool m_CheckZombies = true;
	float m_flNextCheck = 0.0f;
};
LINK_ENTITY_TO_CLASS( trigger_endround, CTriggerEndRound );		// For backwards compatibility
LINK_ENTITY_TO_CLASS( trigger_escape, CTriggerEndRound );

void CTriggerEndRound::Spawn(void)
{
	pev->classname = MAKE_STRING( "trigger_escape" );
	InitTrigger();

	SetTouch( &CTriggerEndRound::OnEndRoundTouch );
}

void CTriggerEndRound::Restart(void)
{
	m_Enabled = m_EnableRem;
	m_CheckZombies = true;
	m_flNextCheck = gpGlobals->time + 2.0f;
}

void CTriggerEndRound::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "enabled" ) )
	{
		m_EnableRem = m_Enabled = atoi(pkvd->szValue) > 0 ? true : false;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue( pkvd );
}

void CTriggerEndRound::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_Enabled = !m_Enabled;
	m_flNextCheck = gpGlobals->time + 2.0f;
}

void CTriggerEndRound::OnEndRoundTouch( CBaseEntity *pOther )
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
				m_flNextCheck = gpGlobals->time + 2.0f;
			return;
		}
	}

	if ( pOther->pev->team != ZP::TEAM_SURVIVIOR ) return;
	IGameModeBase *pGameMode = ZP::GetCurrentGameMode();
	if ( !pGameMode ) return;
	CBasePlayer *pPlayer = (CBasePlayer *)pOther;
	// Set escaped state
	pPlayer->SetHasEscaped( true );
	// Start spectating
	pPlayer->StartObserver();
	pPlayer->GiveAchievement( EAchievements::FIRST_ESCAPE );
	pPlayer->GiveAchievement( EAchievements::ESCAPE_ARTIST );

	UTIL_ClientPrintAll(
		HUD_PRINTTALK,
		UTIL_VarArgs(
			"^2%s ^0 has escaped!\n",
			(pPlayer->pev->netname && STRING(pPlayer->pev->netname)[0] != 0) ? STRING(pPlayer->pev->netname) : "unconnected"
		)
	);
}