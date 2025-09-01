// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "zp/zp_shared.h"

#include "zp_gamemodebase.h"
#include "zp_objective.h"
#include "zp/info_objective.h"

ZPGameMode_Objective::ZPGameMode_Objective()
{
	SetRoundState( ZP::RoundState_WaitingForPlayers );
	m_bTimeRanOut = false;
	m_bAllSurvivorsDead = false;
	m_bHasPlayersReachedEnd = false;
	m_flRoundBeginsIn = 0;
}

void ZPGameMode_Objective::OnHUDInit(CBasePlayer *pPlayer)
{
	BaseClass::OnHUDInit( pPlayer );
	MESSAGE_BEGIN(MSG_ONE, gmsgRoundState, NULL, pPlayer->edict());
	WRITE_SHORT(GetRoundState());
	MESSAGE_END();
}

ZPGameMode_Objective::WinState_e ZPGameMode_Objective::GetWinState()
{
	if ( m_bHasPlayersReachedEnd || HasAnySurvivorEscaped() ) return State_SurvivorWin;
	if ( m_bTimeRanOut ) return State_ZombieWin;
	return m_bAllSurvivorsDead ? State_ZombieWin : State_None;
}

void ZPGameMode_Objective::GiveWeaponsOnRoundStart()
{
	BaseClass::GiveWeaponsOnRoundStart();
	CObjectiveMessage *pFind = (CObjectiveMessage *)UTIL_FindEntityByClassname( nullptr, "info_objective" );
	while ( pFind )
	{
		// If we find our first objective, make sure it's being fired!
		if ( pFind->IsFirstObjective() )
		{
			pFind->Use( nullptr, nullptr, USE_ON, 0 );
			break;
		}
		pFind = (CObjectiveMessage *)UTIL_FindEntityByClassname( pFind, "info_objective" );
	}
}

void ZPGameMode_Objective::RestartRound()
{
	m_bTimeRanOut = false;
	m_bHasPlayersReachedEnd = false;
	BaseClass::RestartRound();
}

bool ZPGameMode_Objective::HasAnySurvivorEscaped()
{
	bool bHasEscaped = false;
	bool bSurvivorAlive = false;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
		if ( plr && plr->IsConnected() && plr->pev->team == ZP::TEAM_SURVIVIOR )
		{
			if ( !bHasEscaped && plr->HasEscaped() )
				bHasEscaped = true;
			if ( !bSurvivorAlive && plr->IsAlive() && !plr->IsObserver() )
				bSurvivorAlive = true;
		}
	}
	if ( m_bTimeRanOut ) bSurvivorAlive = false;
	if ( bSurvivorAlive ) return false;
	return bHasEscaped;
}

