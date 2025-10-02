// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_melee_leadpipe.h"
#ifndef CLIENT_DLL
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_leadpipe, CWeaponMeleeLeadPipe );

enum
{
	LEADPIPE_IDLE = 0,
	LEADPIPE_DRAW,
	LEADPIPE_HOLSTER,
	LEADPIPE_ATTACK1HIT,
	LEADPIPE_ATTACK1MISS,
	LEADPIPE_ATTACK2MISS,
	LEADPIPE_ATTACK2HIT,
	LEADPIPE_ATTACK3MISS,
	LEADPIPE_ATTACK3HIT
};

#define LEADPIPE_BODYHIT_VOLUME 128
#define LEADPIPE_WALLHIT_VOLUME 512

void CWeaponMeleeLeadPipe::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_leadpipe.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}

void CWeaponMeleeLeadPipe::Precache(void)
{
	PRECACHE_MODEL("models/v_leadpipe.mdl");
	PRECACHE_MODEL("models/w_leadpipe.mdl");
	PRECACHE_MODEL("models/p_leadpipe.mdl");
	PRECACHE_SOUND("weapons/melee/leadpipe/hit1.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hit2.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod1.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod2.wav");
	PRECACHE_SOUND("weapons/melee/leadpipe/hitbod3.wav");
	PRECACHE_SOUND("weapons/melee/miss1.wav");

	m_nEventPrimary = PRECACHE_EVENT(1, "events/leadpipe.sc");
}

int CWeaponMeleeLeadPipe::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

BOOL CWeaponMeleeLeadPipe::Deploy()
{
	return DefaultDeploy("models/v_leadpipe.mdl", "models/p_leadpipe.mdl", LEADPIPE_DRAW, "crowbar");
}

void CWeaponMeleeLeadPipe::Holster(int skiplocal /* = 0 */)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(LEADPIPE_HOLSTER);
}

extern void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity);

void CWeaponMeleeLeadPipe::PrimaryAttack()
{
	if (!Swing(1))
	{
		SetThink(&CWeaponMeleeLeadPipe::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CWeaponMeleeLeadPipe::Smack()
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	DecalGunshot(&m_trHit, gpGlobals->v_forward, BULLET_PLAYER_CROWBAR);
}

void CWeaponMeleeLeadPipe::SwingAgain(void)
{
	Swing(0);
}

int CWeaponMeleeLeadPipe::Swing(int fFirst)
{
	if ( !m_pPlayer ) return FALSE;

	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 30;

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

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}
	}
	else
	{
		switch (((m_iSwing++) % 2) + 1)
		{
		case 0:
			SendWeaponAnim(LEADPIPE_ATTACK1HIT);
			break;
		case 1:
			SendWeaponAnim(LEADPIPE_ATTACK2HIT);
			break;
		case 2:
			SendWeaponAnim(LEADPIPE_ATTACK3HIT);
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
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgLeadPipe, gpGlobals->v_forward, &tr, DMG_CLUB);
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgLeadPipe / 2, gpGlobals->v_forward, &tr, DMG_CLUB);
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
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/leadpipe/hitbod1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/leadpipe/hitbod2.wav", 1, ATTN_NORM);
					break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/leadpipe/hitbod3.wav", 1, ATTN_NORM);
					break;
				}
				m_pPlayer->m_iWeaponVolume = LEADPIPE_BODYHIT_VOLUME;
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
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

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
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/leadpipe/hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/melee/leadpipe/hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * LEADPIPE_WALLHIT_VOLUME;
#endif
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + PrimaryFireRate();

		SetThink(&CWeaponMeleeLeadPipe::Smack);
		pev->nextthink = gpGlobals->time + 0.2;
	}
	return fDidHit;
}
