// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_sidearm_glock17.h"

LINK_ENTITY_TO_CLASS( weapon_glock17, CWeaponSideArmGlock17 );
PRECACHE_WEAPON_REGISTER( weapon_glock17 );


float CWeaponSideArmGlock17::DoHolsterAnimation()
{
	SendWeaponAnim( IsEmpty() ? ANIM_PISTOL_HOLSTER_EMPTY : ANIM_PISTOL_HOLSTER );
	return GetAnimationTime( IsEmpty() ? 24 : 18, 35 );
}

float CWeaponSideArmGlock17::DoWeaponUnload()
{
	SendWeaponAnim( ANIM_PISTOL_UNLOAD );
	AddWeaponSound( "weapons/glock17/clipout_unload.wav", 1, ATTN_NORM, GetAnimationTime( 8, 32 ) );
	AddWeaponSound( "weapons/glock17/clipin.wav", 1, ATTN_NORM, GetAnimationTime( 33, 32 ) );
	AddWeaponSound( "weapons/glock17/slideback.wav", 1, ATTN_NORM, GetAnimationTime( 54, 32 ) );
	return GetAnimationTime( 87, 32 );
}

void CWeaponSideArmGlock17::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_ppk.mdl");
	DefaultSpawn();
}

void CWeaponSideArmGlock17::Precache(void)
{
	PRECACHE_MODEL("models/v_ppk.mdl");
	PRECACHE_MODEL("models/w_ppk.mdl");
	PRECACHE_MODEL("models/p_ppk.mdl");

	PRECACHE_MODEL("models/shell.mdl"); // brass shell

	PRECACHE_SOUND("items/ammo_pickup.wav");

	PRECACHE_SOUND("weapons/glock17/dryfire.wav"); //handgun
	PRECACHE_SOUND("weapons/glock17/fire.wav"); //handgun
	PRECACHE_SOUND("weapons/glock17/clipin.wav"); //handgun
	PRECACHE_SOUND("weapons/glock17/clipin_unload.wav"); //handgun
	PRECACHE_SOUND("weapons/glock17/clipout.wav"); //handgun
	PRECACHE_SOUND("weapons/glock17/clipout_unload.wav"); //handgun
	PRECACHE_SOUND("weapons/glock17/slideforward.wav"); //handgun
	PRECACHE_SOUND("weapons/glock17/slideback.wav"); //handgun

	m_nEventPrimary = PRECACHE_EVENT(1, "events/glock17.sc");
}

int CWeaponSideArmGlock17::AddToPlayer(CBasePlayer *pPlayer)
{
	if (BaseClass::AddToPlayer(pPlayer))
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

float CWeaponSideArmGlock17::Deploy()
{
	DoDeploy( "models/v_ppk.mdl", "models/p_ppk.mdl", IsEmpty() ? ANIM_PISTOL_DRAW_EMPTY : ANIM_PISTOL_DRAW, "onehanded" );
	return GetAnimationTime( 26, 60 );
}

void CWeaponSideArmGlock17::PrimaryAttack(void)
{
	if ( IsEmpty() )
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	// silenced
	if (pev->body == 1)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	}

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming;

	vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(iBullets(), vecSrc, vecAiming, Vector(PrimaryWeaponSpread(), PrimaryWeaponSpread(), PrimaryWeaponSpread()), 8192, BULLET_PLAYER_PPK, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_nEventPrimary, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, IsEmpty() ? 1 : 0, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + PrimaryFireRate();

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 14, 30 );
}

void CWeaponSideArmGlock17::Reload(void)
{
	if (m_pPlayer->ammo_longrifle <= 0)
		return;

	int iResult = DefaultReload( IsEmpty() ? ANIM_PISTOL_RELOAD_EMPTY : ANIM_PISTOL_RELOAD, GetAnimationTime( 60, 32 ) );
	if ( iResult )
	{
		if ( IsEmpty() )
		{
			AddWeaponSound( "weapons/glock17/clipout.wav", 1, ATTN_NORM, GetAnimationTime( 10, 32 ) );
			AddWeaponSound( "weapons/glock17/clipin.wav", 1, ATTN_NORM, GetAnimationTime( 32, 32 ) );
			AddWeaponSound( "weapons/glock17/slideforward.wav", 1, ATTN_NORM, GetAnimationTime( 43, 32 ) );
		}
		else
		{
			AddWeaponSound( "weapons/glock17/clipout.wav", 1, ATTN_NORM, GetAnimationTime( 8, 32 ) );
			AddWeaponSound( "weapons/glock17/clipin.wav", 1, ATTN_NORM, GetAnimationTime( 36, 32 ) );
		}
	}
}

void CWeaponSideArmGlock17::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ) return;

	float flTime;
	int iAnim;
	switch (RANDOM_LONG(0, 2))
	{
	case 0:
		iAnim = IsEmpty() ? ANIM_PISTOL_IDLE1_EMPTY : ANIM_PISTOL_IDLE1;
		flTime = GetAnimationTime( 41, 10 );
		break;

	default:
	case 1:
		iAnim = IsEmpty() ? ANIM_PISTOL_IDLE2_EMPTY : ANIM_PISTOL_IDLE2;
		flTime = GetAnimationTime( 41, 10 );
		break;

	case 2:
		iAnim = IsEmpty() ? ANIM_PISTOL_IDLE3_EMPTY : ANIM_PISTOL_IDLE3;
		flTime = GetAnimationTime( 41, 10 );
		break;
	}

	SendWeaponAnim(iAnim);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTime;
}
