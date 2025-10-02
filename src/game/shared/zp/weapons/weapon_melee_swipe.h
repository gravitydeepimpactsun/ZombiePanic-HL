// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MELEE_SWIPE_H
#define SHARED_WEAPON_MELEE_SWIPE_H

#include "CWeaponBase.h"

class CWeaponMeleeSwipe : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponMeleeSwipe, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_SWIPE; }
	void Spawn( void );
	void Precache( void );
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack(void);
	void WeaponIdle();
	int Swing( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwing;
	TraceResult m_trHit;
};

#endif