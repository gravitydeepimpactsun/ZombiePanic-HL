// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_shotgun_doublebarrel.h"

// Unfinished, uses placeholder animations/sounds/logic

enum
{
	DBARREL_IDLE = 0,
	DBARREL_IDLE2,
	DBARREL_FIRE,
	DBARREL_RELOAD_START,
	DBARREL_RELOAD1,
	DBARREL_RELOAD2,
	DBARREL_RELOAD_END,
	DBARREL_DRAW
};


LINK_ENTITY_TO_CLASS( weapon_doublebarrel, CWeaponShotgunDoubleBarrel );


BOOL CWeaponShotgunDoubleBarrel::PlayEmptySound()
{
	if (m_iPlayEmptySound)
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/db_dryfire.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));
		m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

void CWeaponShotgunDoubleBarrel::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_doublebarrel.mdl");

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.
}

void CWeaponShotgunDoubleBarrel::Precache(void)
{
	PRECACHE_MODEL("models/v_doublebarrel.mdl");
	PRECACHE_MODEL("models/w_doublebarrel.mdl");
	PRECACHE_MODEL("models/p_doublebarrel.mdl");

	PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("weapons/doublebarrel/dryfire.wav");
	PRECACHE_SOUND("weapons/doublebarrel/fire.wav");
	PRECACHE_SOUND("weapons/doublebarrel/open.wav");
	PRECACHE_SOUND("weapons/doublebarrel/reload1.wav");
	PRECACHE_SOUND("weapons/doublebarrel/reload2.wav");
	PRECACHE_SOUND("weapons/doublebarrel/close.wav");

	m_nEventPrimary = PRECACHE_EVENT(1, "events/dbarrel.sc");
}

int CWeaponShotgunDoubleBarrel::AddToPlayer(CBasePlayer *pPlayer)
{
	if (BaseClass::AddToPlayer(pPlayer))
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

BOOL CWeaponShotgunDoubleBarrel::Deploy()
{
#if defined( SERVER_DLL )
	if ( m_pPlayer )
		m_pPlayer->m_iWeaponKillCount = 0;
#endif
	return DoDeploy( "models/v_shotgun.mdl", "models/p_shotgun.mdl", DBARREL_DRAW, "shotgun" );
}

void CWeaponShotgunDoubleBarrel::Holster( int skiplocal )
{
#if defined( SERVER_DLL )
	m_pPlayer->m_iWeaponKillCount = 0;
#endif
}

void CWeaponShotgunDoubleBarrel::OnRequestedAnimation( SingleActionAnimReq act )
{
	switch ( act )
	{
		case CWeaponBaseSingleAction::ANIM_IDLE: SendWeaponAnim( DBARREL_IDLE ); break;
		case CWeaponBaseSingleAction::ANIM_LONGIDLE: SendWeaponAnim( DBARREL_IDLE2 ); break;
		//case CWeaponBaseSingleAction::ANIM_PRIMARYATTACK: SendWeaponAnim( SHOTGUN_FIRE ); break;
		case CWeaponBaseSingleAction::ANIM_RELOAD_START:
		{
			EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/db_open.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f) );
			SendWeaponAnim( DBARREL_RELOAD_START );
		}
		break;
		case CWeaponBaseSingleAction::ANIM_RELOAD:
		{
			EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/db_reload2.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f) );

			if ( m_iClip == 0 )
				SendWeaponAnim( DBARREL_RELOAD1 );
		    else
				SendWeaponAnim( DBARREL_RELOAD2 );

#if defined( SERVER_DLL )
			m_pPlayer->m_iWeaponKillCount = 0;
#endif

		    m_flNextReload = UTIL_WeaponTimeBase() + 0.5;
		    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		}
		break;
		case CWeaponBaseSingleAction::ANIM_RELOAD_END:
		{
			SendWeaponAnim( DBARREL_RELOAD_END );
			EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/db_close.wav", 1, ATTN_NORM, 0, 105 );
			m_flNextReload = UTIL_WeaponTimeBase() + 1.5;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
		}
		break;
	}
}

void CWeaponShotgunDoubleBarrel::OnWeaponPrimaryAttack()
{
	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(iBullets(), vecSrc, vecAiming, GetSpreadVector( PrimaryWeaponSpread() ), 2048, BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_nEventPrimary, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
}