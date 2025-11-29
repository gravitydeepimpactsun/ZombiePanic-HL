// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_explosive_tnt.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS(weapon_handgrenade, CWeaponExplosiveTNT); // for backwards compatibility
LINK_ENTITY_TO_CLASS(weapon_tnt, CWeaponExplosiveTNT);
PRECACHE_WEAPON_REGISTER( weapon_tnt );

void CWeaponExplosiveTNT::Spawn()
{
	pev->classname = MAKE_STRING( "weapon_tnt" );
	Precache();
	SET_MODEL(ENT(pev), "models/w_tnt.mdl");
	DefaultSpawn();
}

void CWeaponExplosiveTNT::DeactivateThrow()
{
	m_flReleaseThrow = -1;
	m_flStartThrow = 0;
}

int CWeaponExplosiveTNT::AddToPlayer(CBasePlayer *pPlayer)
{
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup( pPlayer );
		return TRUE;
	}
	return FALSE;
}

int CWeaponExplosiveTNT::AddDuplicate( CBasePlayerItem *pOriginal )
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

void CWeaponExplosiveTNT::Precache(void)
{
	PRECACHE_MODEL("models/w_tnt.mdl");
	PRECACHE_MODEL("models/w_tnt_thrown.mdl");
	PRECACHE_MODEL("models/v_tnt.mdl");
	PRECACHE_MODEL("models/p_tnt.mdl");
	PRECACHE_SOUND("weapons/tnt/fuse.wav");
	PRECACHE_SOUND("weapons/tnt/lit_1.wav");
	PRECACHE_SOUND("weapons/tnt/lit_2.wav");
	PRECACHE_SOUND("weapons/tnt/spark.wav");
}

float CWeaponExplosiveTNT::Deploy()
{
	m_flReleaseThrow = -1;
	DoDeploy( "models/v_tnt.mdl", "models/p_tnt.mdl", ANIM_THROW_EXPLOSIVES_DRAW, "crowbar" );
	return GetAnimationTime( 26, 30 );
}

BOOL CWeaponExplosiveTNT::CanHolster(void)
{
	// can only holster when not primed!
	return (m_flStartThrow == 0);
}

float CWeaponExplosiveTNT::DoHolsterAnimation()
{
	SendWeaponAnim( ANIM_THROW_EXPLOSIVES_HOLSTER );
	return GetAnimationTime( 8, 20 );
}

void CWeaponExplosiveTNT::PrimaryAttack()
{
	if (!m_flStartThrow && m_iClip > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		AddWeaponSound( "weapons/tnt/lit_1.wav", 1, ATTN_NORM, GetAnimationTime( 2, 25 ) );
		AddWeaponSound( "weapons/tnt/fuse.wav", 1, ATTN_NORM, GetAnimationTime( 24, 25 ) );
		AddWeaponSound( "weapons/tnt/lit_2.wav", 1, ATTN_NORM, GetAnimationTime( 25, 25 ) );
		SendWeaponAnim( ANIM_THROW_EXPLOSIVES_PINPULL );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 34, 25 );
	}
}

void CWeaponExplosiveTNT::SecondaryAttack()
{
	if (!m_flStartThrow && m_iClip > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;

		AddWeaponSound( "weapons/tnt/lit_1.wav", 1, ATTN_NORM, GetAnimationTime( 4, 25 ) );
		AddWeaponSound( "weapons/tnt/fuse.wav", 1, ATTN_NORM, GetAnimationTime( 24, 25 ) );
		AddWeaponSound( "weapons/tnt/lit_2.wav", 1, ATTN_NORM, GetAnimationTime( 24, 25 ) );
		SendWeaponAnim( ANIM_THROW_EXPLOSIVES_PINPULL2 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 35, 25 );
		m_bDoSecondaryAttack = true;
	}
}

void CWeaponExplosiveTNT::WeaponIdle(void)
{
	if ( m_flReleaseThrow == 0 && m_flStartThrow )
		m_flReleaseThrow = gpGlobals->time;

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if (m_flStartThrow)
	{
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float flVel = m_bDoSecondaryAttack ? 280 : 800;

		UTIL_MakeVectors(angThrow);

		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;

		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

		// alway explode 5 seconds after the pin was pulled
		float time = m_flStartThrow - gpGlobals->time + 5.0;
		if (time < 0)
			time = 0;

		CGrenade::ShootTimed(m_pPlayer->pev, vecSrc, vecThrow, time, "models/w_tnt_thrown.mdl");

		SendWeaponAnim( m_bDoSecondaryAttack ? ANIM_THROW_EXPLOSIVES_THROW2 : ANIM_THROW_EXPLOSIVES_THROW );

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		float flTime = m_bDoSecondaryAttack ? GetAnimationTime( 13, 30 ) : GetAnimationTime( 6, 10 );
		m_flReleaseThrow = 1;
		m_flStartThrow = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTime;
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle + 0.1f;
		m_bDoSecondaryAttack = false;

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

		if ( m_iClip > 0 )
			DoDeployAnimation();
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + GetAnimationTime( 26, 30 );
		m_flReleaseThrow = -1;
		return;
	}

	if (m_iClip)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.75)
		{
			iAnim = ANIM_THROW_EXPLOSIVES_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 50, 6 );
		}
		else
		{
			iAnim = ANIM_THROW_EXPLOSIVES_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 117, 16 );
		}

		SendWeaponAnim(iAnim);
	}
}
