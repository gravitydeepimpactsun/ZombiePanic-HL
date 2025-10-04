// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_sidearm_revolver.h"
#ifndef CLIENT_DLL
#include "gamerules.h"
#endif

enum
{
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

LINK_ENTITY_TO_CLASS(weapon_python, CWeaponSideArmRevolver);
LINK_ENTITY_TO_CLASS(weapon_357, CWeaponSideArmRevolver);

int CWeaponSideArmRevolver::AddToPlayer(CBasePlayer *pPlayer)
{
	if (BaseClass::AddToPlayer(pPlayer))
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

void CWeaponSideArmRevolver::Spawn()
{
	pev->classname = MAKE_STRING("weapon_357"); // hack to allow for old names
	Precache();
	SET_MODEL(ENT(pev), "models/w_357.mdl");

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.
}

void CWeaponSideArmRevolver::Precache(void)
{
	PRECACHE_MODEL("models/v_357.mdl");
	PRECACHE_MODEL("models/w_357.mdl");
	PRECACHE_MODEL("models/p_357.mdl");

	PRECACHE_MODEL("models/w_357ammobox.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/revolver/reload.wav");
	PRECACHE_SOUND("weapons/revolver/dryfire.wav");
	PRECACHE_SOUND("weapons/revolver/fire1.wav");
	PRECACHE_SOUND("weapons/revolver/fire2.wav");

	m_nEventPrimary = PRECACHE_EVENT(1, "events/python.sc");
}

BOOL CWeaponSideArmRevolver::Deploy()
{
#if defined( SERVER_DLL )
	if ( m_pPlayer )
		m_pPlayer->m_iWeaponKillCount = 0;
#endif
	return DefaultDeploy("models/v_357.mdl", "models/p_357.mdl", PYTHON_DRAW, "python");
}

void CWeaponSideArmRevolver::Holster(int skiplocal /* = 0 */)
{
	m_fInReload = FALSE; // cancel any reload in progress.
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
#if defined( SERVER_DLL )
	m_pPlayer->m_iWeaponKillCount = 0;
#endif
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	SendWeaponAnim( PYTHON_HOLSTER );
}

void CWeaponSideArmRevolver::PrimaryAttack()
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
		if (!m_fFireOnEmpty)
		{
			Reload();
		}
		else
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(iBullets(), vecSrc, vecAiming, GetSpreadVector(PrimaryWeaponSpread()), 8192, BULLET_PLAYER_357, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

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

	m_flNextPrimaryAttack = PrimaryFireRate();
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

void CWeaponSideArmRevolver::Reload(void)
{
	if (m_pPlayer->ammo_357 <= 0)
		return;

	int bUseScope = FALSE;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	if (DefaultReload(PYTHON_RELOAD, 2.0, bUseScope))
	{
#if defined( SERVER_DLL )
		m_pPlayer->m_iWeaponKillCount = 0;
#endif
		m_flSoundDelay = 1.5;
	}
}

void CWeaponSideArmRevolver::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	// ALERT( at_console, "%.2f\n", gpGlobals->time - m_flSoundDelay );
	if (m_flSoundDelay != 0 && m_flSoundDelay <= UTIL_WeaponTimeBase())
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/revolver/reload.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);
		m_flSoundDelay = 0;
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	if (flRand <= 0.5)
	{
		iAnim = PYTHON_IDLE1;
		m_flTimeWeaponIdle = (70.0 / 30.0);
	}
	else if (flRand <= 0.7)
	{
		iAnim = PYTHON_IDLE2;
		m_flTimeWeaponIdle = (60.0 / 30.0);
	}
	else if (flRand <= 0.9)
	{
		iAnim = PYTHON_IDLE3;
		m_flTimeWeaponIdle = (88.0 / 30.0);
	}
	else
	{
		iAnim = PYTHON_FIDGET;
		m_flTimeWeaponIdle = (170.0 / 30.0);
	}

	int bUseScope = FALSE;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0, bUseScope);
}

class CWeaponSideArmRevolverAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_357ammobox.mdl");
		CBasePlayerAmmo::Spawn();
		WeaponData slot = GetWeaponSlotInfo( ZPWeaponID::WEAPON_PYTHON );
		m_iAmountLeft = m_iAmmoToGive = slot.AmmoBox;
		m_AmmoType = ZPAmmoTypes::AMMO_MAGNUM;
		strncpy(m_szSound, "items/9mmclip1.wav", 32);
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_357ammobox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
};
LINK_ENTITY_TO_CLASS(ammo_357, CWeaponSideArmRevolverAmmo);
