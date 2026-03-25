// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_rifle_sks.h"

LINK_ENTITY_TO_CLASS( weapon_sks, CWeaponRifleSKS );
PRECACHE_WEAPON_REGISTER( weapon_sks );


float CWeaponRifleSKS::DoHolsterAnimation()
{
	SendWeaponAnim( ANIM_AR556_HOLSTER );
	return GetAnimationTime( 10, 35 );
}

float CWeaponRifleSKS::DoWeaponUnload()
{
	SendWeaponAnim( ANIM_AR556_UNLOAD );
	AddWeaponSound( "weapons/556ar/magout_unload.wav", 1, ATTN_NORM, GetAnimationTime( 15, 20 ) );
	AddWeaponSound( "weapons/556ar/magin_unload.wav", 1, ATTN_NORM, GetAnimationTime( 28, 20 ) );
	return GetAnimationTime( 81, 26 );
}

void CWeaponRifleSKS::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_sks.mdl");
	DefaultSpawn();
}

void CWeaponRifleSKS::Precache(void)
{
	PRECACHE_MODEL("models/v_sks.mdl");
	PRECACHE_MODEL("models/w_sks.mdl");
	PRECACHE_MODEL("models/p_sks.mdl");

	PRECACHE_MODEL("models/shell_rifle.mdl"); // brass shellTE_MODEL

	PRECACHE_SOUND("items/ammo_pickup.wav");

	PRECACHE_SOUND("weapons/sks/fire.wav");
	PRECACHE_SOUND("weapons/556ar/magout.wav");
	PRECACHE_SOUND("weapons/556ar/magin.wav");
	PRECACHE_SOUND("weapons/556ar/magout_unload.wav");
	PRECACHE_SOUND("weapons/556ar/magin_unload.wav");
	PRECACHE_SOUND("weapons/556ar/charge.wav");

	m_nEventPrimary = PRECACHE_EVENT(1, "events/sks.sc");
}

int CWeaponRifleSKS::AddToPlayer(CBasePlayer *pPlayer)
{
	if (BaseClass::AddToPlayer(pPlayer))
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

float CWeaponRifleSKS::Deploy()
{
	DoDeploy( "models/v_sks.mdl", "models/p_sks.mdl", ANIM_AR556_DEPLOY, "556ar" );
	return GetAnimationTime( 22, 35 );
}

void CWeaponRifleSKS::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer(iBullets(), vecSrc, vecAiming, GetSpreadVector(PrimaryWeaponSpread()), 8192, BULLET_PLAYER_SKS, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_nEventPrimary, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + PrimaryFireRate();

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + PrimaryFireRate();

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}


void CWeaponRifleSKS::Reload(void)
{
	if (m_pPlayer->ammo_556ar <= 0)
		return;

	int iAnim = IsEmpty() ? ANIM_AR556_RELOAD_EMPTY : ANIM_AR556_RELOAD;
	float fDelay = IsEmpty() ? GetAnimationTime( 81, 25 ) : GetAnimationTime( 95, 30 );
	if ( DefaultReload(iAnim, fDelay) )
	{
		AddWeaponSound( "weapons/556ar/magout.wav", 1, ATTN_NORM, 0.13f );
		AddWeaponSound( "weapons/556ar/magin.wav", 1, ATTN_NORM, 1.2f );
		if ( IsEmpty() )
			AddWeaponSound( "weapons/556ar/charge.wav", 1, ATTN_NORM, 2.03f );
	}
}

void CWeaponRifleSKS::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	float flTime;
	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = ANIM_AR556_LONGIDLE;
		flTime = GetAnimationTime( 25, 3 );
		break;

	default:
	case 1:
		iAnim = ANIM_AR556_IDLE1;
		flTime = GetAnimationTime( 29, 4 );
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTime;
}
