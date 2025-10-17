// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "CWeaponBase.h"
#ifndef CLIENT_DLL
#include "gamerules.h"
#endif

/// <summary>
/// Modified iItemSlot function.
/// This is deprecated, so do not use it.
/// </summary>
/// <param name=""></param>
/// <returns>Weapon slot number + 1</returns>
int CWeaponBase::iItemSlot( void )
{
	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	return slot.Slot + 1;
}

BOOL CWeaponBase::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal, int body)
{
	m_bIsHolstering = false;
	ClearWeaponSounds();
	return BaseClass::DefaultDeploy( szViewModel, szWeaponModel, iAnim, szAnimExt, skiplocal, body );
}

int CWeaponBase::DefaultReload(int iAnim, float fDelay, int body)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return FALSE;

	int j = min(GetData().MaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	if (j == 0)
		return FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	SendWeaponAnim(iAnim, 0, body);

	m_fInReload = TRUE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + fDelay + 0.1;
	return TRUE;
}

void CWeaponBase::BeginHolster( CBasePlayerWeapon *pWeapon )
{
	m_pNewWeapon = pWeapon;
	m_bIsHolstering = true;
	m_flHolsterTime = gpGlobals->time;
	DoHolsterAnimation();
}

void CWeaponBase::FinishHolster()
{
	m_bIsHolstering = false;
	m_flHolsterTime = -1;
	BaseClass::Holster();
	if ( !m_pPlayer ) return;
	if ( !m_pNewWeapon ) return;
#if !defined( CLIENT_DLL )
	m_pPlayer->SelectNewActiveWeapon( m_pNewWeapon );
#endif
	m_pNewWeapon = nullptr;
}

void CWeaponBase::StopHolstering()
{
	m_bIsHolstering = false;
	m_flHolsterTime = -1;
	m_pNewWeapon = nullptr;
}

/// <summary>
/// Modified ItemPostFrame from CBasePlayerWeapon.
/// Because having 2 cpp files for the same class is stupid.
/// </summary>
/// <param name=""></param>
void CWeaponBase::ItemPostFrame( void )
{
	CBasePlayer *pPlayer = m_pPlayer; // Cache player cos attack could retire weapon and remove it from player
	if ( !pPlayer ) return;

	// If we are holstering, we can't do anything until it's done
	if ( m_bIsHolstering )
	{
		if ( m_flHolsterTime - gpGlobals->time <= 0.0f )
			FinishHolster();
		return;
	}

	if ((m_fInReload) && (pPlayer->m_flNextAttack <= UTIL_WeaponTimeBase()))
	{
		// complete the reload.
		int j = min(iMaxClip() - m_iClip, pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		// Add them to the clip
		m_iClip += j;
		pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;

#ifndef CLIENT_DLL
		pPlayer->TabulateAmmo();
#endif

		m_fInReload = FALSE;
	}

	if ( IsAutomaticWeapon() )
	{
		if ((pPlayer->pev->button & IN_ATTACK2) && CanAttack(m_flNextSecondaryAttack, gpGlobals->time, UseDecrement()))
		{
			if (HasValidAmmoType(false) && !pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
			{
				m_fFireOnEmpty = TRUE;
			}

#ifndef CLIENT_DLL
			pPlayer->TabulateAmmo();
#endif
			SecondaryAttack();
		}
		else if ((pPlayer->pev->button & IN_ATTACK) && CanAttack(m_flNextPrimaryAttack, gpGlobals->time, UseDecrement()))
		{
			if ((m_iClip == 0 && HasValidAmmoType(true)) || (iMaxClip() == WEAPON_NOCLIP && !pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
			{
				m_fFireOnEmpty = TRUE;
			}

#ifndef CLIENT_DLL
			pPlayer->TabulateAmmo();
#endif
			PrimaryAttack();
		}
	}
	else
	{
		if ((pPlayer->m_afButtonPressed & IN_ATTACK2) && CanAttack(m_flNextSecondaryAttack, gpGlobals->time, UseDecrement()))
		{
			if (HasValidAmmoType(false) && !pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
			{
				m_fFireOnEmpty = TRUE;
			}

#ifndef CLIENT_DLL
			pPlayer->TabulateAmmo();
#endif
			SecondaryAttack();
		}
		else if ((pPlayer->m_afButtonPressed & IN_ATTACK) && CanAttack(m_flNextPrimaryAttack, gpGlobals->time, UseDecrement()))
		{
			if ((m_iClip == 0 && HasValidAmmoType(true)) || (iMaxClip() == WEAPON_NOCLIP && !pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
			{
				m_fFireOnEmpty = TRUE;
			}

#ifndef CLIENT_DLL
			pPlayer->TabulateAmmo();
#endif
			PrimaryAttack();
		}
	}

	if ( pPlayer->pev->button & IN_RELOAD
		&& iMaxClip() != WEAPON_NOCLIP
		&& !m_fInReload
		&& CanAttack( m_flNextPrimaryAttack, gpGlobals->time, true ) )
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if (!(pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2)))
	{
		// no fire buttons down

		m_fFireOnEmpty = FALSE;

		// Reload if empty and weapon has waited as long as it has to after firing
		if ((pPlayer->pev->button & (IN_RELOAD)) && m_iClip == 0 && m_flNextPrimaryAttack - gpGlobals->time < 0.0f)
		{
			Reload();
			return;
		}

		WeaponIdle();
		return;
	}
}

void CWeaponBase::DoAudioFrame( void )
{
	for ( int x = 0; x < (int)m_vecWeaponSoundData.size(); x++ )
	{
		WeaponSoundData &soundData = m_vecWeaponSoundData[x];
		if ( soundData.Delay - gpGlobals->time <= 0 )
		{
			// Time to play the sound
			if ( m_pPlayer )
				EMIT_SOUND( ENT(m_pPlayer->pev), CHAN_WEAPON, soundData.File, soundData.Volume, soundData.Attenuation );
			// Remove it from the list
			m_vecWeaponSoundData.erase( m_vecWeaponSoundData.begin() + x );
			break; // Only play one sound per frame
		}
	}
}

void CWeaponBase::AddWeaponSound( const char *szSoundFile, float volume, float attenuation, float delay )
{
	WeaponSoundData newSound;
	newSound.File = szSoundFile;
	newSound.Volume = volume;
	newSound.Attenuation = attenuation;
	newSound.Delay = gpGlobals->time + delay;
	m_vecWeaponSoundData.push_back( newSound );
}

void CWeaponBase::ClearWeaponSounds()
{
	m_vecWeaponSoundData.clear();
}

void CWeaponBase::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Make sure we have a player
	CBasePlayer *pPlayer = (CBasePlayer *)pActivator;
	if ( !pPlayer || !pPlayer->IsPlayer() ) return;

	// Check if we can pickup the weapon
	if ( pPlayer->AddPlayerItem(this) )
	{
		AttachToPlayer( pPlayer );
		if ( pPlayer->pev->team == ZP::TEAM_SURVIVIOR )
			EMIT_SOUND( ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
		return;
	}

	BaseClass::Use( pActivator, pCaller, useType, value );
}

/// <summary>
/// Checks if we can attack or not
/// </summary>
/// <param name="attack_time">Our current attack time</param>
/// <param name="curtime">Current time from the global timer</param>
/// <param name="isPredicted">Is this predicted?</param>
/// <returns></returns>
bool CWeaponBase::CanAttack( float attack_time, float curtime, bool isPredicted )
{
#if defined( CLIENT_WEAPONS )
	if ( !isPredicted )
#else
	if ( 1 )
#endif
		return (attack_time <= curtime) ? true : false;
	return (attack_time <= 0.0) ? true : false;
}
