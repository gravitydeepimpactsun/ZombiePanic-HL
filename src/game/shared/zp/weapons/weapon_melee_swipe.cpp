// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_melee_swipe.h"
#ifndef CLIENT_DLL
#include "game.h"
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_swipe, CWeaponMeleeSwipe );

#define SWIPE_BODYHIT_VOLUME 128
#define SWIPE_WALLHIT_VOLUME 512

void CWeaponMeleeSwipe::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_swipe.mdl");
	m_iClip = -1;
	pev->team = ZP::TEAM_ZOMBIE;

	FallInit(); // get ready to fall down.
}

void CWeaponMeleeSwipe::Precache(void)
{
	PRECACHE_MODEL("models/v_swipe.mdl");
	PRECACHE_MODEL("models/w_swipe.mdl");
	PRECACHE_MODEL("models/p_swipe.mdl");
	PRECACHE_SOUND("weapons/melee/zarm/strike1.wav");
	PRECACHE_SOUND("weapons/melee/zarm/strike2.wav");
	PRECACHE_SOUND("weapons/melee/zarm/strike3.wav");
	PRECACHE_SOUND("weapons/melee/miss1.wav");
	PRECACHE_SOUND("weapons/melee/miss2.wav");

	m_nEventPrimary = PRECACHE_EVENT(1, "events/swipe.sc");
}

int CWeaponMeleeSwipe::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( pPlayer->pev->team != ZP::TEAM_ZOMBIE )
	{
		UTIL_Remove( this );
		return FALSE;
	}
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup( pPlayer );
		return TRUE;
	}
	UTIL_Remove( this );
	return FALSE;
}

BOOL CWeaponMeleeSwipe::Deploy()
{
	return DefaultDeploy("models/v_swipe.mdl", "models/p_swipe.mdl", ANIM_MELEE_DRAW, "swipe");
}

void CWeaponMeleeSwipe::DoHolsterAnimation()
{
	SendWeaponAnim( ANIM_MELEE_HOLSTER );
	m_flHolsterTime = gpGlobals->time + 0.5f;
}

extern void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity );

void CWeaponMeleeSwipe::PrimaryAttack()
{
	if (!Swing(1))
	{
		SetThink(&CWeaponMeleeSwipe::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CWeaponMeleeSwipe::Smack()
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	DecalGunshot(&m_trHit, gpGlobals->v_forward, BULLET_PLAYER_SWIPE);
}

void CWeaponMeleeSwipe::SwingAgain(void)
{
	Swing(0);
}

int CWeaponMeleeSwipe::Swing(int fFirst)
{
	if ( !m_pPlayer ) return FALSE;

	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 52;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	if (fFirst)
	{
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_nEventPrimary,
		    0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0,
		    0.0, 0, 0.0);
	}

	if (tr.flFraction >= 1.0)
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + PrimaryFireRate() + 1.0f;

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}
	}
	else
	{
		switch (((m_iSwing++) % 2) + 1)
		{
		case 0:
			SendWeaponAnim(ANIM_MELEE_ATTACK1HIT);
			break;
		case 1:
			SendWeaponAnim(ANIM_MELEE_ATTACK2HIT);
			break;
		case 2:
			SendWeaponAnim(ANIM_MELEE_ATTACK3HIT);
			break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL

		// hit
		fDidHit = TRUE;
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();

		// JoshA: Changed from < -> <= to fix the full swing logic since client weapon prediction.
		// -1.0f + 1.0f = 0.0f. UTIL_WeaponTimeBase is always 0 with client weapon prediction (0 time base vs curtime base)
		if ((m_flNextPrimaryAttack + 1.0f <= UTIL_WeaponTimeBase()) || g_pGameRules->IsMultiplayer())
		{
			// first swing does full damage
			pEntity->TraceAttack(m_pPlayer->pev, mp_dmg_swipe.value, gpGlobals->v_forward, &tr, DMG_SLASH);
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack(m_pPlayer->pev, mp_dmg_swipe.value / 2, gpGlobals->v_forward, &tr, DMG_SLASH);
		}
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		int fHitWorld = TRUE;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 2))
				{
				case 0:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/zarm/strike1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/zarm/strike2.wav", 1, ATTN_NORM);
					break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/zarm/strike3.wav", 1, ATTN_NORM);
					break;
				}
				m_pPlayer->m_iWeaponVolume = SWIPE_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return TRUE;
				else
					flVol = 0.1;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_SWIPE);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer,
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/zarm/strike1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/zarm/strike2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 2:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/zarm/strike3.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * SWIPE_WALLHIT_VOLUME;
#endif
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + PrimaryFireRate();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + PrimaryFireRate() + 1.0f;

		SetThink(&CWeaponMeleeSwipe::Smack);
		pev->nextthink = gpGlobals->time + 0.2;
	}
	return fDidHit;
}

void CWeaponMeleeSwipe::WeaponIdle()
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ) return;
	int iAnim;
	float flTime = 1.0f;
	switch ( RANDOM_LONG(0, 4) )
	{
		default:
		case 0:
		    iAnim = ANIM_MELEE_IDLE1;
		    flTime = 3.0f;
		break;
		case 2:
		    iAnim = ANIM_MELEE_IDLE2;
		    flTime = 1.0f;
		break;
		case 4:
		    iAnim = ANIM_MELEE_IDLE3;
		    flTime = 1.06f;
		break;
	}
	SendWeaponAnim( iAnim );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTime; // how long till we do this again.
}
