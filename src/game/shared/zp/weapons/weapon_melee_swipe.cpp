// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_melee_swipe.h"
#ifndef CLIENT_DLL
#include "game.h"
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_swipe, CWeaponMeleeSwipe );
PRECACHE_WEAPON_REGISTER( weapon_swipe );

void CWeaponMeleeSwipe::Precache( void )
{
	PRECACHE_MODEL("models/v_swipe.mdl");
	PRECACHE_MODEL("models/w_swipe.mdl");
	PRECACHE_MODEL("models/p_swipe.mdl");
	PRECACHE_SOUND("weapons/melee/zarm/strike1.wav");
	PRECACHE_SOUND("weapons/melee/zarm/strike2.wav");
	PRECACHE_SOUND("weapons/melee/zarm/strike3.wav");
	PRECACHE_SOUND("weapons/melee/zarm/miss1.wav");
	PRECACHE_SOUND("weapons/melee/zarm/miss2.wav");
}

void CWeaponMeleeSwipe::DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld )
{
	const char *szSoundFile = nullptr;
	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0: szSoundFile = "weapons/melee/zarm/strike1.wav"; break;
		case 1: szSoundFile = "weapons/melee/zarm/strike2.wav"; break;
		case 2: szSoundFile = "weapons/melee/zarm/strike3.wav"; break;
	}
	EMIT_SOUND( ENT(m_pPlayer->pev), CHAN_ITEM, szSoundFile, 1, ATTN_NORM );
}

void CWeaponMeleeSwipe::DoWeaponSoundFromMiss( MeleeAttackType attackType )
{
	const char *szSoundFile = nullptr;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
		case 0: szSoundFile = "weapons/melee/zarm/miss1.wav"; break;
		case 1: szSoundFile = "weapons/melee/zarm/miss2.wav"; break;
	}
	EMIT_SOUND( ENT(m_pPlayer->pev), CHAN_ITEM, szSoundFile, 1, ATTN_NORM );
}
