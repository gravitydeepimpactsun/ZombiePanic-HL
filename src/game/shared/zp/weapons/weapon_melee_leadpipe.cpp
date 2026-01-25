// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_melee_leadpipe.h"
#ifndef CLIENT_DLL
#include "game.h"
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_leadpipe, CWeaponMeleeLeadPipe );
PRECACHE_WEAPON_REGISTER( weapon_leadpipe );

void CWeaponMeleeLeadPipe::Precache( void )
{
	PRECACHE_MODEL("models/v_leadpipe.mdl");
	PRECACHE_MODEL("models/w_leadpipe.mdl");
	PRECACHE_MODEL("models/p_leadpipe.mdl");
	PRECACHE_SOUND("weapons/melee/leadpipe/hit1.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hit2.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod1.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod2.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod3.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hit1_heavy.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hit2_heavy.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod1_heavy.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod2_heavy.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod3_heavy.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/miss1.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/miss2.wav");
}

void CWeaponMeleeLeadPipe::DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld )
{
	const char *szSoundFile = nullptr;
	if ( bHitWorld )
	{
		if ( attackType == MELEE_ATTACK_HEAVY )
		{
			switch ( RANDOM_LONG( 0, 1 ) )
			{
				case 0: szSoundFile = "weapons/melee/leadpipe/hit1_heavy.wav"; break;
				case 1: szSoundFile = "weapons/melee/leadpipe/hit2_heavy.wav"; break;
			}
		}
		else
		{
			switch ( RANDOM_LONG( 0, 1 ) )
			{
				case 0: szSoundFile = "weapons/melee/leadpipe/hit1.wav"; break;
				case 1: szSoundFile = "weapons/melee/leadpipe/hit1.wav"; break;
			}
		}
	}
	else
	{
		if (attackType == MELEE_ATTACK_HEAVY)
		{
			switch ( RANDOM_LONG( 0, 2 ) )
			{
				case 0: szSoundFile = "weapons/melee/leadpipe/hitbod1_heavy.wav"; break;
				case 1: szSoundFile = "weapons/melee/leadpipe/hitbod2_heavy.wav"; break;
				case 2: szSoundFile = "weapons/melee/leadpipe/hitbod3_heavy.wav"; break;
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
	}
	EMIT_SOUND( ENT(m_pPlayer->pev), CHAN_ITEM, szSoundFile, 1, ATTN_NORM );
}

void CWeaponMeleeLeadPipe::DoWeaponSoundFromMiss( MeleeAttackType attackType )
{
	const char *szSoundFile = nullptr;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
		case 0: szSoundFile = "weapons/melee/leadpipe/miss1.wav"; break;
		case 1: szSoundFile = "weapons/melee/leadpipe/miss2.wav"; break;
	}
	EMIT_SOUND( ENT(m_pPlayer->pev), CHAN_ITEM, szSoundFile, 1, ATTN_NORM );
}
