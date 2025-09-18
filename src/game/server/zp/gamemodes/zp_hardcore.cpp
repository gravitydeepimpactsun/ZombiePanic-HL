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
		pPlayer->GiveNamedItem( "weapon_crowbar" );
		if ( !pPlayer->m_bPunishLateJoiner )
		{
			pPlayer->GiveNamedItem( "weapon_sig" );
			pPlayer->GiveAmmo( 14, ZPAmmoTypes::AMMO_PISTOL );

			// Randomly give a secondary weapon
			int randWep = RANDOM_LONG(0, 3);
			switch (randWep)
			{
				case 0:
					pPlayer->GiveNamedItem( "weapon_shotgun" );
					pPlayer->GiveAmmo( 6, ZPAmmoTypes::AMMO_SHOTGUN );
				break;
			    case 1:
					pPlayer->GiveNamedItem( "weapon_556ar" );
					pPlayer->GiveAmmo( 20, ZPAmmoTypes::AMMO_RIFLE );
				break;
			    case 2:
					pPlayer->GiveNamedItem( "weapon_357" );
					pPlayer->GiveAmmo(6, ZPAmmoTypes::AMMO_MAGNUM );
				break;
				case 3:
					pPlayer->GiveNamedItem( "weapon_mp5" );
					pPlayer->GiveAmmo( 30, ZPAmmoTypes::AMMO_PISTOL );
				break;
			}
		}
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
