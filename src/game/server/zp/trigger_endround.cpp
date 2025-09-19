// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "triggers.h"
#include "player.h"
#include "zp/gamemodes/zp_gamemodebase.h"

class CTriggerEndRound : public CBaseTrigger
{
public:
	void Spawn(void);
	void OnEndRoundTouch(CBaseEntity *pOther);
};
LINK_ENTITY_TO_CLASS( trigger_endround, CTriggerEndRound );		// For backwards compatibility
LINK_ENTITY_TO_CLASS( trigger_escape, CTriggerEndRound );

void CTriggerEndRound::Spawn(void)
{
	InitTrigger();

	SetTouch( &CTriggerEndRound::OnEndRoundTouch );
}

void CTriggerEndRound::OnEndRoundTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() ) return;
	if ( !pOther->IsAlive() ) return;
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