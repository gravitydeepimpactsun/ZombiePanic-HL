// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_sidearm_revolver.h"
#ifndef CLIENT_DLL
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS(weapon_python, CWeaponSideArmRevolver);
LINK_ENTITY_TO_CLASS(weapon_357, CWeaponSideArmRevolver);
PRECACHE_WEAPON_REGISTER( weapon_357 );

int CWeaponSideArmRevolver::AddToPlayer(CBasePlayer *pPlayer)
{
	if (BaseClass::AddToPlayer(pPlayer))
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

void CWeaponSideArmRevolver::DoAudioFrame( void )
{
	// Same as our baseclass, but we play an event when we find out.wav
	for ( int x = 0; x < (int)m_vecWeaponSoundData.size(); x++ )
	{
		WeaponSoundData &soundData = m_vecWeaponSoundData[x];
		if ( soundData.Delay - gpGlobals->time <= 0 )
		{
			// Time to play the sound
			if ( m_pPlayer )
			{
				// Don't play sounds in client DLL
				// If this is the "EVENT_SHELLS", play a shell ejection event. It's a shitty hack, but eh.
				// At least the origin won't be fucked...
				if ( FStrEq( soundData.File, "EVENT_SHELLS" ) )
				{
					int flags;
#if defined(CLIENT_WEAPONS)
					flags = FEV_NOTHOST;
#else
					flags = 0;
#endif
					PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_nEventSecondary, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, ( m_iClip == 1 ) ? 1 : 0, 0, 0, 0 );
				}
				else
				{
#if !defined( CLIENT_DLL )
					EMIT_SOUND( ENT(m_pPlayer->pev), CHAN_WEAPON, soundData.File, soundData.Volume, soundData.Attenuation );
#endif
				}
			}
			// Remove it from the list
			m_vecWeaponSoundData.erase( m_vecWeaponSoundData.begin() + x );
			break; // Only play one sound per frame
		}
	}
}

float CWeaponSideArmRevolver::DoHolsterAnimation()
{
#if defined( SERVER_DLL )
	m_pPlayer->m_iWeaponKillCount = 0;
#endif
	SendWeaponAnim( ANIM_357_HOLSTER );
	return GetAnimationTime( 15, 30 );
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

	PRECACHE_SOUND("items/ammo_pickup.wav");

	PRECACHE_SOUND("weapons/revolver/open.wav");
	PRECACHE_SOUND("weapons/revolver/remove.wav");
	PRECACHE_SOUND("weapons/revolver/insert.wav");
	PRECACHE_SOUND("weapons/revolver/close.wav");
	PRECACHE_SOUND("weapons/revolver/dryfire.wav");
	PRECACHE_SOUND("weapons/revolver/fire1.wav");
	PRECACHE_SOUND("weapons/revolver/fire2.wav");

	m_nEventPrimary = PRECACHE_EVENT(1, "events/python.sc");
	m_nEventSecondary = PRECACHE_EVENT(1, "events/python_shells.sc");
}

float CWeaponSideArmRevolver::Deploy()
{
#if defined( SERVER_DLL )
	if ( m_pPlayer )
		m_pPlayer->m_iWeaponKillCount = 0;
#endif
	DoDeploy( "models/v_357.mdl", "models/p_357.mdl", ANIM_357_DRAW, "python" );
	return GetAnimationTime( 21, 16 );
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
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

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
	m_flTimeWeaponIdle = GetAnimationTime( 18, 30 );
}

void CWeaponSideArmRevolver::Reload(void)
{
	if ( m_pPlayer->ammo_357 <= 0 ) return;
	if ( DefaultReload( ANIM_357_RELOAD, GetAnimationTime( 75, 20 ) ) )
	{
#if defined( SERVER_DLL )
		m_pPlayer->m_iWeaponKillCount = 0;
#endif
		AddWeaponSound( "weapons/revolver/open.wav", 1, ATTN_NORM, GetAnimationTime( 16, 20 ) );
		AddWeaponSound( "weapons/revolver/remove.wav", 1, ATTN_NORM, GetAnimationTime( 23, 20 ) );
		AddWeaponSound( "EVENT_SHELLS", 1, ATTN_NORM, GetAnimationTime( 30, 20 ) );
		AddWeaponSound( "weapons/revolver/insert.wav", 1, ATTN_NORM, GetAnimationTime( 43, 20 ) );
		AddWeaponSound( "weapons/revolver/close.wav", 1, ATTN_NORM, GetAnimationTime( 55, 20 ) );
	}
}

void CWeaponSideArmRevolver::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	if (flRand <= 0.5)
	{
		iAnim = ANIM_357_IDLE1;
		m_flTimeWeaponIdle = GetAnimationTime( 41, 10 );
	}
	else if (flRand <= 0.7)
	{
		iAnim = ANIM_357_IDLE2;
		m_flTimeWeaponIdle = GetAnimationTime( 41, 5 );
	}
	else if (flRand <= 0.9)
	{
		iAnim = ANIM_357_IDLE3;
		m_flTimeWeaponIdle = GetAnimationTime( 41, 10 );
	}
	else
	{
		iAnim = ANIM_357_FIDGET;
		m_flTimeWeaponIdle = GetAnimationTime( 100, 30 );
	}

	int bUseScope = FALSE;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0, bUseScope);
}
