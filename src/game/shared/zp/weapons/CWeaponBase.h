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
	int iItemSlot( void ) override;

	// We don't use secondary attack in Zombie Panic! by default
	void SecondaryAttack( void ) {}

	virtual void ItemPostFrame( void );

	virtual BOOL UseDecrement( void )
	{
#if defined(CLIENT_WEAPONS)
		return TRUE;
#else
		return FALSE;
#endif
	}

	virtual bool IsEmpty( void ) const { return ( m_iClip <= 0 ); }

protected:
	bool CanAttack( float attack_time, float curtime, bool isPredicted );

	unsigned short m_nEventPrimary;
	unsigned short m_nEventSecondary;
};

#endif