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

/// <summary>
/// Modified ItemPostFrame from CBasePlayerWeapon.
/// Because having 2 cpp files for the same class is stupid.
/// </summary>
/// <param name=""></param>
void CWeaponBase::ItemPostFrame( void )
{
	CBasePlayer *pPlayer = m_pPlayer; // Cache player cos attack could retire weapon and remove it from player

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
			pPlayer->pev->button &= ~IN_ATTACK2;
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

#ifndef CLIENT_DLL
		if (!IsUseable() && m_flNextPrimaryAttack < (UseDecrement() ? 0.0 : gpGlobals->time))
		{
			// weapon isn't useable, switch.
			if (!(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon(pPlayer, this))
			{
				m_flNextPrimaryAttack = (UseDecrement() ? 0.0 : gpGlobals->time) + 0.3;
				return;
			}
		}
		else
#endif
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if ((pPlayer->pev->button & (IN_RELOAD)) && m_iClip == 0 && m_flNextPrimaryAttack < (UseDecrement() ? 0.0 : gpGlobals->time))
			{
				Reload();
				return;
			}
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
