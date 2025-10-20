// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_rifle_m16.h"

LINK_ENTITY_TO_CLASS( weapon_556ar, CWeaponRifleM16 );
LINK_ENTITY_TO_CLASS( weapon_9mmar, CWeaponRifleM16 ); // Only for old maps, DO NOT USE THIS.


void CWeaponRifleM16::DoHolsterAnimation()
{
	SendWeaponAnim( ANIM_AR556_HOLSTER );
	m_flNextPrimaryAttack = m_flTimeWeaponIdle = m_flHolsterTime = gpGlobals->time + 0.55;
}

void CWeaponRifleM16::Spawn()
{
	pev->classname = MAKE_STRING( "weapon_556ar" );
	Precache();
	SET_MODEL(ENT(pev), "models/w_556AR.mdl");

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.
}

void CWeaponRifleM16::Precache(void)
{
	PRECACHE_MODEL("models/v_556AR.mdl");
	PRECACHE_MODEL("models/w_556AR.mdl");
	PRECACHE_MODEL("models/p_556AR.mdl");

	PRECACHE_MODEL("models/shell.mdl"); // brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl"); // grenade

	PRECACHE_MODEL("models/w_556ARclip.mdl");
	PRECACHE_SOUND("items/ammo_pickup.wav");

	PRECACHE_SOUND("weapons/556ar/fire1.wav"); // H to the K
	PRECACHE_SOUND("weapons/556ar/fire2.wav"); // H to the K
	PRECACHE_SOUND("weapons/556ar/dryfire.wav");
	PRECACHE_SOUND("weapons/556ar/magout.wav");
	PRECACHE_SOUND("weapons/556ar/magin.wav");
	PRECACHE_SOUND("weapons/556ar/charge.wav");

	m_nEventPrimary = PRECACHE_EVENT(1, "events/m16.sc");
}

int CWeaponRifleM16::AddToPlayer(CBasePlayer *pPlayer)
{
	if (BaseClass::AddToPlayer(pPlayer))
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

BOOL CWeaponRifleM16::Deploy()
{
	DoDeploy( "models/v_556AR.mdl", "models/p_556AR.mdl", ANIM_AR556_DEPLOY, "mp5" );
	m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 21, 30 );
	return TRUE;
}

void CWeaponRifleM16::PrimaryAttack()
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

	vecDir = m_pPlayer->FireBulletsPlayer(iBullets(), vecSrc, vecAiming, GetSpreadVector(PrimaryWeaponSpread()), 8192, BULLET_PLAYER_M16, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);

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


void CWeaponRifleM16::Reload(void)
{
	if (m_pPlayer->ammo_556ar <= 0)
		return;

	int iAnim = IsEmpty() ? ANIM_AR556_RELOAD_EMPTY : ANIM_AR556_RELOAD;
	float fDelay = IsEmpty() ? 3.16f : 2.16f;
	if ( DefaultReload(iAnim, fDelay) )
	{
		AddWeaponSound( "weapons/556ar/magout.wav", 1, ATTN_NORM, 0.13f );
		AddWeaponSound( "weapons/556ar/magin.wav", 1, ATTN_NORM, 1.2f );
		if ( IsEmpty() )
			AddWeaponSound( "weapons/556ar/charge.wav", 1, ATTN_NORM, 2.03f );
	}
}

void CWeaponRifleM16::WeaponIdle(void)
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
		flTime = 1.67f;
		break;

	default:
	case 1:
		iAnim = ANIM_AR556_IDLE1;
		flTime = 1.93f;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTime;
}

class CWeaponRifleM16AmmoClip : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_556ARclip.mdl");
		CBasePlayerAmmo::Spawn();
		WeaponData slot = GetWeaponSlotInfo( ZPWeaponID::WEAPON_556AR );
		m_iAmountLeft = m_iAmmoToGive = slot.AmmoBox;
		m_AmmoType = ZPAmmoTypes::AMMO_RIFLE;
		strncpy(m_szSound, "items/ammo_pickup.wav", 32);
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_556ARclip.mdl");
		PRECACHE_SOUND("items/ammo_pickup.wav");
	}
};
LINK_ENTITY_TO_CLASS(ammo_556AR, CWeaponRifleM16AmmoClip);

class CWeaponRifleM16Chainammo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_556box.mdl");
		CBasePlayerAmmo::Spawn();
		WeaponData slot = GetWeaponSlotInfo( ZPWeaponID::WEAPON_556AR );
		m_iAmountLeft = m_iAmmoToGive = slot.AmmoBox * 2;
		m_AmmoType = ZPAmmoTypes::AMMO_RIFLE;
		strncpy(m_szSound, "items/ammo_pickup.wav", 32);
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_556box.mdl");
		PRECACHE_SOUND("items/ammo_pickup.wav");
	}
};
LINK_ENTITY_TO_CLASS(ammo_556box, CWeaponRifleM16Chainammo);
