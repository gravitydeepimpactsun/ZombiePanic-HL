// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_BASE_H
#define SHARED_WEAPON_BASE_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"

#ifndef DECLARE_CLASS_SIMPLE
#define DECLARE_CLASS_SIMPLE( className, baseClassName ) typedef baseClassName BaseClass;
#endif

class CWeaponBase : public CBasePlayerWeapon
{
	DECLARE_CLASS_SIMPLE( CWeaponBase, CBasePlayerWeapon );

public:
	virtual int ObjectCaps(void) { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; }

	int iItemSlot( void ) override;
	BOOL DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal = 0, int body = 0) override;
	int DefaultReload(int iAnim, float fDelay, int body = 0) override;

	void BeginHolster( CBasePlayerWeapon *pWeapon );
	virtual void DoHolsterAnimation() {}
	void FinishHolster();

	// We don't use secondary attack in Zombie Panic! by default
	void SecondaryAttack( void ) {}

	virtual void ItemPostFrame( void );

	/// <summary>
	/// Used to play audio, such as delayed reload sounds or other sounds that need to be played
	/// </summary>
	/// <param name=""></param>
	virtual void DoAudioFrame( void );

	virtual BOOL UseDecrement( void )
	{
#if defined(CLIENT_WEAPONS)
		return TRUE;
#else
		return FALSE;
#endif
	}

	virtual bool IsEmpty( void ) const { return ( m_iClip <= 0 ); }
	void AddWeaponSound( const char *szSoundFile, float volume = 1.0f, float attenuation = ATTN_NORM, float delay = 0.0f );
	void ClearWeaponSounds( void );
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	bool IsHolstering( void ) const { return m_bIsHolstering; }

protected:
	bool CanAttack( float attack_time, float curtime, bool isPredicted );

	unsigned short m_nEventPrimary;
	unsigned short m_nEventSecondary;

	struct WeaponSoundData
	{
		const char *File; // Path to sound file
		float Volume; // Volume of sound
		float Attenuation; // Attenuation of sound
		float Delay; // Delay before playing sound
	};
	std::vector<WeaponSoundData> m_vecWeaponSoundData;

	float m_flHolsterTime = 0.0f; // Time when we holstered the weapon

private:
	bool m_bIsHolstering = false; // Are we holstering the weapon?
	CBasePlayerWeapon *m_pNewWeapon = nullptr; // The weapon to switch to after holstering
};

#endif