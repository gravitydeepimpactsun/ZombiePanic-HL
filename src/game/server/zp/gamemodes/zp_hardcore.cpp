// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "zp/zp_shared.h"
#include <convar.h>

#include "zp_gamemodebase.h"
#include "zp_hardcore.h"

ConVar zp_zombielives_hardcore( "zp_zombielives_hardcore", "12", FCVAR_SERVER, "Amount of zombie starter lives" );

ZPGameMode_HardCore::ZPGameMode_HardCore()
{
	SetRoundState( ZP::RoundState_WaitingForPlayers );
	m_bTimeRanOut = false;
	m_bAllSurvivorsDead = false;
	m_iZombieLives = (int)clamp( zp_zombielives_hardcore.GetInt(), 1, 24 );
	m_flRoundBeginsIn = 0;
}

void ZPGameMode_HardCore::RestartRound()
{
	BaseClass::RestartRound();
	m_iZombieLives = zp_zombielives_hardcore.GetInt();
}

void ZPGameMode_HardCore::GiveWeapons(CBasePlayer *pPlayer)
{
	CBaseEntity *pWeaponEntity = NULL;
	BOOL addDefault = TRUE;

	while (pWeaponEntity = UTIL_FindEntityByClassname(pWeaponEntity, pPlayer->m_bPunishLateJoiner ? "game_player_equip_punish" : "game_player_equip" ))
	{
		pWeaponEntity->Touch( pPlayer );
		addDefault = FALSE;
	}

	if ( addDefault )
	{
		if ( !pPlayer->m_bPunishLateJoiner )
		{
			// Randomly give a melee weapon
			int randWep = UTIL_SharedRandomLong( pPlayer->random_seed, 0, 10 );
			switch (randWep)
			{
				default: pPlayer->GiveNamedItem( "weapon_crowbar" ); break;
			    case 3:
			    case 4:
				case 8: pPlayer->GiveNamedItem( "weapon_leadpipe" ); break;
			}

			// Randomly give a secondary weapon
			randWep = UTIL_SharedRandomLong( pPlayer->random_seed, 0, 4 );
			switch (randWep)
			{
				case 0:
					pPlayer->GiveNamedItem( "weapon_shotgun" );
					pPlayer->GiveAmmo( 12, ZPAmmoTypes::AMMO_SHOTGUN );
				break;
			    case 1:
					pPlayer->GiveNamedItem( "weapon_556ar" );
					pPlayer->GiveAmmo( 40, ZPAmmoTypes::AMMO_RIFLE );
				break;
			    case 2:
					pPlayer->GiveNamedItem( "weapon_ppk" );
					pPlayer->GiveAmmo( 20, ZPAmmoTypes::AMMO_LONGRIFLE );
				break;
				case 3:
					pPlayer->GiveNamedItem( "weapon_mp5" );
					pPlayer->GiveAmmo( 60, ZPAmmoTypes::AMMO_PISTOL );
				break;
				case 4:
					pPlayer->GiveNamedItem( "weapon_sig" );
					pPlayer->GiveAmmo( 14, ZPAmmoTypes::AMMO_PISTOL );
				break;
			}
		}
		else
			pPlayer->GiveNamedItem( "weapon_crowbar" );
		pPlayer->m_bPunishLateJoiner = false;
	}
}

void ZPGameMode_HardCore::GiveWeaponsOnRoundStart()
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
		if ( plr && plr->IsAlive() )
		{
			int iTeam = plr->pev->team;
			if ( iTeam == ZP::TEAM_SURVIVIOR )
				GiveWeapons( plr );
			plr->m_iHideHUD = 0;
			plr->m_flCanSuicide = gpGlobals->time + 20.0f;
			plr->m_flSuicideTimer = -1; // Just in case if the player manages to frame perfect a "kill" command.
			// Zombies can't choose weapons, they only got their arms.
			if ( iTeam == ZP::TEAM_ZOMBIE )
				plr->m_iHideHUD |= HIDEHUD_WEAPONS;
		}
	}

	// Make sure we update this!
	UpdateZombieLifesForClient();
}
