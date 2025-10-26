// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_shotgun_remington.h"

LINK_ENTITY_TO_CLASS( weapon_shotgun, CWeaponShotgunRemington );
PRECACHE_WEAPON_REGISTER( weapon_shotgun );

float CWeaponShotgunRemington::DoHolsterAnimation()
{
#if defined( SERVER_DLL )
	m_pPlayer->m_iWeaponKillCount = 0;
#endif
	SendWeaponAnim( ANIM_SHOTGUN_HOLSTER );
	return 0.35;
}

void CWeaponShotgunRemington::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_shotgun.mdl");

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall
}

void CWeaponShotgunRemington::Precache( void )
{
	PRECACHE_MODEL( "models/v_shotgun.mdl" );
	PRECACHE_MODEL( "models/w_shotgun.mdl" );
	PRECACHE_MODEL( "models/p_shotgun.mdl" );

	PRECACHE_MODEL( "models/shotgunshell.mdl" ); // shotgun shell

	PRECACHE_SOUND( "items/ammo_pickup.wav" );

	PRECACHE_SOUND( "weapons/sbarrel1.wav" ); //shotgun

	PRECACHE_SOUND( "weapons/reload1.wav" ); // shotgun reload
	PRECACHE_SOUND( "weapons/reload3.wav" ); // shotgun reload

	PRECACHE_SOUND( "weapons/shotgun/dryfire.wav" );
	PRECACHE_SOUND( "weapons/shotgun/fire.wav" );
	PRECACHE_SOUND( "weapons/shotgun/pump.wav" );
	PRECACHE_SOUND( "weapons/shotgun/reload1.wav" );
	PRECACHE_SOUND( "weapons/shotgun/reload2.wav" );

	m_nEventPrimary = PRECACHE_EVENT( 1, "events/shotgun1.sc" );
	m_nEventPump = PRECACHE_EVENT(1, "events/shotgun_pump.sc");
}

int CWeaponShotgunRemington::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup( pPlayer );
		return TRUE;
	}
	return FALSE;
}

float CWeaponShotgunRemington::Deploy()
{
#if defined( SERVER_DLL )
	if ( m_pPlayer )
		m_pPlayer->m_iWeaponKillCount = 0;
#endif
	DoDeploy( "models/v_shotgun.mdl", "models/p_shotgun.mdl", ANIM_SHOTGUN_DRAW, "shotgun" );
	return GetAnimationTime( 19, 30 );
}

void CWeaponShotgunRemington::OnRequestedAnimation( SingleActionAnimReq act )
{
	switch ( act )
	{
		case CWeaponBaseSingleAction::ANIM_IDLE: SendWeaponAnim( ANIM_SHOTGUN_IDLE ); break;
		case CWeaponBaseSingleAction::ANIM_LONGIDLE: SendWeaponAnim( ANIM_SHOTGUN_IDLE2 ); break;
		//case CWeaponBaseSingleAction::ANIM_PRIMARYATTACK: SendWeaponAnim( SHOTGUN_FIRE ); break;
		case CWeaponBaseSingleAction::ANIM_PUMP:
		{
			// reload debounce has timed out
			SendWeaponAnim( ANIM_SHOTGUN_PUMP );
			// play cocking sound
			EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/shotgun/pump.wav", 1, ATTN_NORM, 0, 105 );
			m_flTimeWeaponIdle = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.73f;
		}
		break;
		case CWeaponBaseSingleAction::ANIM_RELOAD_START: SendWeaponAnim( ANIM_SHOTGUN_RELOAD_START ); break;
		case CWeaponBaseSingleAction::ANIM_RELOAD:
		{
		    if (RANDOM_LONG(0, 1))
			    EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/shotgun/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f) );
		    else
			    EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/shotgun/reload2.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f) );

		    SendWeaponAnim( ANIM_SHOTGUN_RELOAD );

#if defined( SERVER_DLL )
			m_pPlayer->m_iWeaponKillCount = 0;
#endif

		    m_flNextReload = UTIL_WeaponTimeBase() + 0.62;
		    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.62;
		}
		break;
		case CWeaponBaseSingleAction::ANIM_RELOAD_END:
		{
			SendWeaponAnim( ANIM_SHOTGUN_RELOAD_END );
			EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/shotgun/pump.wav", 1, ATTN_NORM, 0, 105 );
			m_flNextReload = UTIL_WeaponTimeBase() + 0.81;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.81;
		}
		break;
	}
}

void CWeaponShotgunRemington::OnWeaponPrimaryAttack()
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

#if defined( SERVER_DLL )
	m_pPlayer->GiveAchievement( EAchievements::PUMPUPSHOTGUN );
#endif
}
