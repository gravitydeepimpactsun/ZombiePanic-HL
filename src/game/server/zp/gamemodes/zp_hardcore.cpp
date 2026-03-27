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
ConVar zp_hardcore_fairbalance( "zp_hardcore_fairbalance", "1", FCVAR_SERVER, "Limits the amount of powerful weapons the survivors may spawn with" );

struct FairBalance_s
{
private:
	char Weapon[32];
	int Amount;
	float Percentage;
	ZPAmmoTypes AmmoType;
	int AmmoGive;

	// Reset remember
	int AmountRem;
	float PercentageRem;

public:
	FairBalance_s( const char *szWeapon, int iAmount, float flChance, ZPAmmoTypes nAmmo, int iAmmoGive )
	{
		Q_strcpy( Weapon, szWeapon );
		AmountRem = Amount = iAmount;
		PercentageRem = Percentage = flChance;
		AmmoType = nAmmo;
		AmmoGive = iAmmoGive;
	}

	void Reset()
	{
		Amount = AmountRem;
		Percentage = PercentageRem;
	}

	const char *GetName() const { return Weapon; }
	int GetAmount() const { return Amount; }
	float GetChance() const { return Percentage; }

	int GetAmmoAmount() const { return AmmoGive; }
	ZPAmmoTypes GetAmmoType() const { return AmmoType; }

	void ReduceAmount()
	{
		if ( Amount == -1 ) return;
		Amount--;
	}
};

static FairBalance_s BalanceStruct[] = {
	FairBalance_s( "weapon_sig", -1, 1.0, ZPAmmoTypes::AMMO_PISTOL, 14 ),
	FairBalance_s( "weapon_ppk", -1, 1.0, ZPAmmoTypes::AMMO_LONGRIFLE, 40 ),
	FairBalance_s( "weapon_cz75", -1, 1.0, ZPAmmoTypes::AMMO_LONGRIFLE, 40 ),
	FairBalance_s( "weapon_556ar", 2, 0.45, ZPAmmoTypes::AMMO_RIFLE, 40 ),
	FairBalance_s( "weapon_sks", 2, 0.15, ZPAmmoTypes::AMMO_RIFLE, 40 ),
	FairBalance_s( "weapon_shotgun", 2, 0.25, ZPAmmoTypes::AMMO_SHOTGUN, 12 ),
	FairBalance_s( "weapon_mp5", 3, 0.55, ZPAmmoTypes::AMMO_PISTOL, 60 )
};

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
	for ( size_t i = 0; i < ARRAYSIZE( BalanceStruct ); i++ )
	{
		FairBalance_s &item = BalanceStruct[ i ];
		item.Reset();
	}
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

			if ( zp_hardcore_fairbalance.GetBool() )
			{
				float flChance = RandomFloat( 0.0f, 100.0f ) / 100.0f;
				int nValue = RandomInt( 0, ARRAYSIZE( BalanceStruct ) - 1 );
				bool bHasItem = false;
				do
				{
					FairBalance_s &item = BalanceStruct[ nValue ];
					if ( item.GetAmount() == 0 )
					{
						nValue = RandomInt( 0, ARRAYSIZE( BalanceStruct ) - 1 );
						continue;
					}
					if ( flChance <= item.GetChance() || item.GetChance() == 1.0f )
					{
						pPlayer->GiveNamedItem( item.GetName() );
						pPlayer->GiveAmmo( item.GetAmmoAmount(), item.GetAmmoType() );
						item.ReduceAmount();
						bHasItem = true;
						break;
					}
					nValue = RandomInt( 0, ARRAYSIZE( BalanceStruct ) - 1 );
				}
				while ( !bHasItem );
			}
			else
			{
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

			// Now, let's give the player 35 armor.
			pPlayer->pev->armorvalue = 35;
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
