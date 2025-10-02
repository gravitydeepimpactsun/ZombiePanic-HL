// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_sidearm_sig.h"


enum
{
	SIG_IDLE = 0,
	SIG_IDLE_EMPTY,
	SIG_SHOOT,
	SIG_SHOOT_EMPTY,
	SIG_RELOAD,
	SIG_RELOAD_EMPTY,
	SIG_DRAW,
	SIG_HOLSTER,
	SIG_DRAW_EMPTY,
	SIG_HOLSTER_EMPTY,
};

LINK_ENTITY_TO_CLASS(weapon_sig, CWeaponSideArmSig);
LINK_ENTITY_TO_CLASS(weapon_9mmhandgun, CWeaponSideArmSig); // Only used by old custom maps, don't remove this


void CWeaponSideArmSig::Spawn()
{
	pev->classname = MAKE_STRING( "weapon_sig" );
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmhandgun.mdl");

	WeaponData slot = GetWeaponSlotInfo(GetWeaponID());
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.
}

void CWeaponSideArmSig::Precache(void)
{
	PRECACHE_MODEL("models/v_9mmhandgun.mdl");
	PRECACHE_MODEL("models/w_9mmhandgun.mdl");
	PRECACHE_MODEL("models/p_9mmhandgun.mdl");

	PRECACHE_MODEL("models/shell.mdl"); // brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("weapons/sig/dryfire.wav"); //handgun
	PRECACHE_SOUND("weapons/sig/fire.wav"); //handgun
	PRECACHE_SOUND("weapons/sig/reload1.wav"); //handgun
	PRECACHE_SOUND("weapons/sig/reload2.wav"); //handgun

	m_nEventPrimary = PRECACHE_EVENT(1, "events/sig.sc");
}

int CWeaponSideArmSig::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		CBasePlayerWeapon::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

BOOL CWeaponSideArmSig::Deploy()
{
	return DefaultDeploy("models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", IsEmpty() ? SIG_DRAW_EMPTY : SIG_DRAW, "onehanded");
}

void CWeaponSideArmSig::PrimaryAttack(void)
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
	vecDir = m_pPlayer->FireBulletsPlayer(iBullets(), vecSrc, vecAiming, Vector(PrimaryWeaponSpread(), PrimaryWeaponSpread(), PrimaryWeaponSpread()), 8192, BULLET_PLAYER_9MM, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_nEventPrimary, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, IsEmpty() ? 1 : 0, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + PrimaryFireRate();

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

void CWeaponSideArmSig::Reload(void)
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	int iResult = DefaultReload( IsEmpty() ? SIG_RELOAD_EMPTY : SIG_RELOAD, 1.84f );
	if ( iResult )
	{
		// play reload sound
		if ( IsEmpty() )
			AddWeaponSound( "weapons/sig/reload2.wav", 1, ATTN_NORM, 0.28f );
		else
			AddWeaponSound( "weapons/sig/reload1.wav", 1, ATTN_NORM, 0.28f );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	}
}

void CWeaponSideArmSig::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	// only idle if the slid isn't back
	if ( IsEmpty() )
	{
		SendWeaponAnim( SIG_IDLE_EMPTY, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.8f;
	}
	else
	{
		SendWeaponAnim( SIG_IDLE, 1 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.8f;
	}
}

class C9MMAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn();
		WeaponData slot = GetWeaponSlotInfo( ZPWeaponID::WEAPON_SIG );
		m_iAmountLeft = m_iAmmoToGive = slot.AmmoBox;
		m_AmmoType = ZPAmmoTypes::AMMO_PISTOL;
		strncpy(m_szSound, "items/9mmclip1.wav", 32);
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
};
LINK_ENTITY_TO_CLASS(ammo_9mmclip, C9MMAmmo);
