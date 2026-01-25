// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_melee_crowbar.h"
#ifndef CLIENT_DLL
#include "game.h"
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_crowbar, CWeaponMeleeCrowbar );
PRECACHE_WEAPON_REGISTER( weapon_crowbar );

void CWeaponMeleeCrowbar::Precache( void )
{
	PRECACHE_MODEL( "models/v_crowbar.mdl" );
	PRECACHE_MODEL( "models/w_crowbar.mdl" );
	PRECACHE_MODEL( "models/p_crowbar.mdl" );
	PRECACHE_SOUND( "weapons/melee/crowbar/hit1.wav" );
	PRECACHE_SOUND( "weapons/melee/crowbar/hit2.wav" );
	PRECACHE_SOUND( "weapons/melee/crowbar/hitbod1.wav" );
	PRECACHE_SOUND( "weapons/melee/crowbar/hitbod2.wav" );
	PRECACHE_SOUND( "weapons/melee/crowbar/hitbod3.wav" );
	PRECACHE_SOUND( "weapons/melee/crowbar/miss.wav" );
}

void CWeaponMeleeCrowbar::DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld )
{
	const char *szSoundFile = nullptr;
	if ( bHitWorld )
	{
		switch ( RANDOM_LONG( 0, 1 ) )
		{
			case 0: szSoundFile = "weapons/melee/crowbar/hit1.wav"; break;
			case 1: szSoundFile = "weapons/melee/crowbar/hit1.wav"; break;
		}
	}
	else
	{
		switch ( RANDOM_LONG( 0, 2 ) )
		{
			case 0: szSoundFile = "weapons/melee/crowbar/hitbod1.wav"; break;
			case 1: szSoundFile = "weapons/melee/crowbar/hitbod2.wav"; break;
			case 2: szSoundFile = "weapons/melee/crowbar/hitbod3.wav"; break;
		}
	}
	EMIT_SOUND( ENT(m_pPlayer->pev), CHAN_ITEM, szSoundFile, 1, ATTN_NORM );
}

void CWeaponMeleeCrowbar::DoWeaponSoundFromMiss( MeleeAttackType attackType )
{
	EMIT_SOUND( ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/crowbar/miss.wav", 1, ATTN_NORM );
}
