// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "zp/zp_shared.h"
#include <convar.h>

#include "zp_gamemodebase.h"
#include "zp_survival.h"

#define MAX_ZOMBIE_LIVES 99

extern int gmsgTeamInfo;

ConVar zp_zombielives( "zp_zombielives", "8", FCVAR_SERVER, "Amount of zombie starter lives" );

ZPGameMode_Survival::ZPGameMode_Survival()
{
	SetRoundState( ZP::RoundState_WaitingForPlayers );
	m_bTimeRanOut = false;
	m_bAllSurvivorsDead = false;
	m_iZombieLives = (int)clamp( zp_zombielives.GetInt(), 1, 10 );
	m_flRoundBeginsIn = 0;
}

void ZPGameMode_Survival::OnHUDInit(CBasePlayer *pPlayer)
{
	if ( !pPlayer ) return;
	if ( !pPlayer->IsConnected() ) return;
	MESSAGE_BEGIN(MSG_ONE, gmsgRoundState, NULL, pPlayer->edict());
	WRITE_SHORT(GetRoundState());
	MESSAGE_END();
	BaseClass::OnHUDInit(pPlayer);
}

void ZPGameMode_Survival::GetZombieLifeData( int &current, int &max )
{
	current = m_iZombieLives;
	max = MAX_ZOMBIE_LIVES;
}

void ZPGameMode_Survival::OnZombieLifeUpdated( bool bIncreased )
{
	int iUpdate = m_iZombieLives;
	if ( bIncreased )
		++iUpdate;
	else
		--iUpdate;
	m_iZombieLives = (int)clamp( iUpdate, 0, MAX_ZOMBIE_LIVES );

	// Make sure we tell the client about this!
	UpdateZombieLifesForClient();
}

void ZPGameMode_Survival::OnPlayerDied( CBasePlayer *pPlayer, entvars_t *pKiller, entvars_t *pInflictor )
{
	int iTeam = pPlayer->pev->team;
	if ( iTeam == ZP::TEAM_ZOMBIE )
	{
		if ( m_iZombieLives <= 0 )
			pPlayer->m_bNoLives = true;
		bool bKillerIsZombie = ( pKiller && pKiller->team == ZP::TEAM_ZOMBIE ) ? true : false;
		// We killed ourselves?
		if ( pPlayer->pev == pKiller ) bKillerIsZombie = true;
		// Not a client? (aka, died from the world/triggers
		if ( !( pKiller->flags & FL_CLIENT ) ) bKillerIsZombie = true;
		// Sorry, but you aren't allowed to reduce if
		// killer is a zombie
		if ( bKillerIsZombie ) return;
		OnZombieLifeUpdated( false );
	}
	else if ( iTeam == ZP::TEAM_SURVIVIOR )
		OnZombieLifeUpdated( true );
}

void ZPGameMode_Survival::OnPlayerSpawned( CBasePlayer *pPlayer )
{
	// Always send this to clients!
	UpdateZombieLifesForClient();

	BaseClass::OnPlayerSpawned( pPlayer );
}

ZPGameMode_Survival::WinState_e ZPGameMode_Survival::GetWinState()
{
	if ( m_bTimeRanOut ) return State_SurvivorWin;
	if ( HasNoRemainingZombies() ) return State_SurvivorWin;
	return m_bAllSurvivorsDead ? State_ZombieWin : State_None;
}

void ZPGameMode_Survival::RestartRound()
{
	m_bTimeRanOut = false;
	m_iZombieLives = zp_zombielives.GetInt();
	BaseClass::RestartRound();
}

bool ZPGameMode_Survival::HasNoRemainingZombies() const
{
	if ( m_iZombieLives <= 0 )
	{
		int iZombies = 0;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
			if ( plr )
			{
				int iTeam = plr->pev->team;
				if ( iTeam == ZP::TEAM_ZOMBIE && !plr->m_bNoLives )
					iZombies++;
			}
		}
		return ( iZombies > 0 ) ? false : true;
	}
	return false;
}

void ZPGameMode_Survival::CalculateZombieLives()
{
	int iPlayers = 0;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
		if ( plr )
			iPlayers++;
	}
	// More than our starting lives now that we have more players?
	// Override it
	if ( iPlayers > m_iZombieLives )
	{
		m_iZombieLives = iPlayers;
		UpdateZombieLifesForClient();
	}
}

void ZPGameMode_Survival::OnRoundStateThink( ZP::RoundState state )
{
	switch ( state )
	{
		// Calculate our zombie lives on RoundState_RoundHasBegunPost,
		// As it's only fired and used once.
		case ZP::RoundState_RoundHasBegunPost:
		{
		    CalculateZombieLives();
		}
	    break;
	}
}

void ZPGameMode_Survival::UpdateZombieLifesForClient()
{
	MESSAGE_BEGIN(MSG_ALL, gmsgZombieLives);
	WRITE_SHORT(m_iZombieLives);
	MESSAGE_END();
}

void ZPGameMode_Survival::GiveWeaponsOnRoundStart()
{
	BaseClass::GiveWeaponsOnRoundStart();
	// Make sure we update this!
	UpdateZombieLifesForClient();
}
