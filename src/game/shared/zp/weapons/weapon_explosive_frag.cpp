// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_explosive_frag.h"
#include "gamerules.h"

enum
{
	HANDGRENADE_IDLE = 0,
	HANDGRENADE_FIDGET,
	HANDGRENADE_PINPULL,
	HANDGRENADE_THROW1, // toss
	HANDGRENADE_THROW2, // medium
	HANDGRENADE_THROW3, // hard
	HANDGRENADE_HOLSTER,
	HANDGRENADE_DRAW
};


LINK_ENTITY_TO_CLASS(weapon_handgrenade, CWeaponExplosiveFrag);

void CWeaponExplosiveFrag::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_grenade.mdl");

#ifndef CLIENT_DLL
	pev->dmg = gSkillData.plrDmgHandGrenade;
#endif

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.
}

int CWeaponExplosiveFrag::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup( pPlayer );
		return TRUE;
	}
	return FALSE;
}

int CWeaponExplosiveFrag::AddDuplicate( CBasePlayerItem *pOriginal )
{
	CBasePlayerWeapon *pWeapon = dynamic_cast<CBasePlayerWeapon *>( pOriginal );
	if ( pWeapon->m_iClip < pWeapon->iMaxClip() )
	{
		pWeapon->m_iClip += 1;
		EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/ammo_pickup.wav", 1, ATTN_NORM );
		return TRUE;
	}
	return FALSE;
}

void CWeaponExplosiveFrag::Precache(void)
{
	PRECACHE_MODEL("models/w_grenade.mdl");
	PRECACHE_MODEL("models/v_grenade.mdl");
	PRECACHE_MODEL("models/p_grenade.mdl");
	PRECACHE_SOUND("weapons/tnt/fuse.wav");
}

BOOL CWeaponExplosiveFrag::Deploy()
{
	m_flReleaseThrow = -1;
	return DefaultDeploy("models/v_grenade.mdl", "models/p_grenade.mdl", HANDGRENADE_DRAW, "crowbar");
}

BOOL CWeaponExplosiveFrag::CanHolster(void)
{
	// can only holster hand grenades when not primed!
	return (m_flStartThrow == 0);
}

void CWeaponExplosiveFrag::DoHolsterAnimation()
{
	SendWeaponAnim( HANDGRENADE_HOLSTER );
	m_flHolsterTime = gpGlobals->time + 0.4;
}

void CWeaponExplosiveFrag::PrimaryAttack()
{
	if (!m_flStartThrow && m_iClip > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		AddWeaponSound( "weapons/tnt/fuse.wav", 1, ATTN_NORM, 0.96f );
		SendWeaponAnim(HANDGRENADE_PINPULL);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.36f;
	}
}

void CWeaponExplosiveFrag::WeaponIdle(void)
{
	if (m_flReleaseThrow == 0 && m_flStartThrow)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow)
	{
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		static float flMultiplier = 6.5f;
		float flVel = (90 - angThrow.x) * flMultiplier;
		if (flVel > 1000)
			flVel = 1000;

		UTIL_MakeVectors(angThrow);

		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

		// alway explode 3 seconds after the pin was pulled
		float time = m_flStartThrow - gpGlobals->time + 3.0;
		if (time < 0)
			time = 0;

		CGrenade::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow, time);

		if (flVel < 500)
		{
			SendWeaponAnim(HANDGRENADE_THROW1);
		}
		else if (flVel < 1000)
		{
			SendWeaponAnim(HANDGRENADE_THROW2);
		}
		else
		{
			SendWeaponAnim(HANDGRENADE_THROW3);
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		m_flReleaseThrow = 0;
		m_flStartThrow = 0;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.4;

		m_iClip--;

		if (!m_iClip)
		{
			// just threw last grenade
			// set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5; // ensure that the animation can finish playing
#ifndef CLIENT_DLL
			// Auto switch to next best weapon if we have no more grenades.
			m_pPlayer->WeaponSlotSet( this, false );
			DestroyItem();
#endif
		}
		return;
	}
	else if (m_flReleaseThrow > 0)
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0;

		if (m_iClip > 0)
		{
			SendWeaponAnim(HANDGRENADE_DRAW);
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		m_flReleaseThrow = -1;
		return;
	}

	if (m_iClip)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.75)
		{
			iAnim = HANDGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
		}
		else
		{
			iAnim = HANDGRENADE_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
		}

		SendWeaponAnim(iAnim);
	}
}
