// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_smg_mp5.h"

LINK_ENTITY_TO_CLASS( weapon_mp5, CWeaponSMGMP5 );

void CWeaponSMGMP5::DoHolsterAnimation()
{
	SendWeaponAnim( ANIM_MP5_HOLSTER );
	m_flHolsterTime = gpGlobals->time + GetAnimationTime( 11, 30 );
}

void CWeaponSMGMP5::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_mp5.mdl");

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.
}

void CWeaponSMGMP5::Precache(void)
{
	PRECACHE_MODEL("models/v_mp5.mdl");
	PRECACHE_MODEL("models/w_mp5.mdl");
	PRECACHE_MODEL("models/p_mp5.mdl");

	PRECACHE_MODEL("models/shell.mdl"); // brass shellTE_MODEL

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/ammo_pickup.wav");

	PRECACHE_SOUND("weapons/mp5/dryfire.wav");
	PRECACHE_SOUND("weapons/mp5/fire.wav");
	PRECACHE_SOUND("weapons/mp5/magout.wav");
	PRECACHE_SOUND("weapons/mp5/magin.wav");
	PRECACHE_SOUND("weapons/mp5/slide_back.wav");
	PRECACHE_SOUND("weapons/mp5/slide_forward.wav");

	m_nEventPrimary = PRECACHE_EVENT(1, "events/mp5.sc");
}

int CWeaponSMGMP5::AddToPlayer(CBasePlayer *pPlayer)
{
	if (BaseClass::AddToPlayer(pPlayer))
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

BOOL CWeaponSMGMP5::Deploy()
{
	DoDeploy( "models/v_mp5.mdl", "models/p_mp5.mdl", ANIM_MP5_DEPLOY, "mp5" );
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 21, 30 );
	return TRUE;
}

void CWeaponSMGMP5::PrimaryAttack()
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

	vecDir = m_pPlayer->FireBulletsPlayer(iBullets(), vecSrc, vecAiming, GetSpreadVector(PrimaryWeaponSpread()), 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);

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

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 13, 30 );
}


void CWeaponSMGMP5::Reload(void)
{
	if ( m_pPlayer->ammo_9mm <= 0 ) return;
	float flDelay = GetAnimationTime( IsEmpty() ? 77 : 48, 20 );
	if ( IsEmpty() )
	{
		AddWeaponSound( "weapons/mp5/slide_back.wav", 1, ATTN_NORM, GetAnimationTime( 4, 20 ) );
		AddWeaponSound( "weapons/mp5/magout.wav", 1, ATTN_NORM, GetAnimationTime( 18, 20 ) );
		AddWeaponSound( "weapons/mp5/magin.wav", 1, ATTN_NORM, GetAnimationTime( 38, 20 ) );
		AddWeaponSound( "weapons/mp5/slide_forward.wav", 1, ATTN_NORM, GetAnimationTime( 51, 20 ) );
	}
	else
	{
		AddWeaponSound( "weapons/mp5/magout.wav", 1, ATTN_NORM, GetAnimationTime( 4, 20 ) );
		AddWeaponSound( "weapons/mp5/magin.wav", 1, ATTN_NORM, GetAnimationTime( 26, 20 ) );
	}
	DefaultReload( IsEmpty() ? ANIM_MP5_RELOAD_EMPTY : ANIM_MP5_RELOAD, flDelay );
}

void CWeaponSMGMP5::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	float flDelay;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = ANIM_MP5_LONGIDLE;
		flDelay = GetAnimationTime( 25, 15 );
		break;

	default:
	case 1:
		iAnim = ANIM_MP5_IDLE1;
		flDelay = GetAnimationTime( 29, 5 );
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flDelay;
}

class CWeaponSMGMP5AmmoClip : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
		CBasePlayerAmmo::Spawn();
		WeaponData slot = GetWeaponSlotInfo( ZPWeaponID::WEAPON_MP5 );
		m_iAmountLeft = m_iAmmoToGive = slot.AmmoBox;
		m_AmmoType = ZPAmmoTypes::AMMO_PISTOL;
		strncpy(m_szSound, "items/ammo_pickup.wav", 32);
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/ammo_pickup.wav");
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5clip, CWeaponSMGMP5AmmoClip);
LINK_ENTITY_TO_CLASS(ammo_9mmAR, CWeaponSMGMP5AmmoClip);
