// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MELEE_CROWBAR_H
#define SHARED_WEAPON_MELEE_CROWBAR_H

#include "CWeaponBase.h"

class CWeaponMeleeCrowbar : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponMeleeCrowbar, CWeaponBase );

public:
	bool IsMeleeWeapon() override { return true; }
	ZPWeaponID GetWeaponID() override { return WEAPON_CROWBAR; }
	void Spawn( void );
	void Precache( void );
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	int Swing( int fFirst );
	void WeaponIdle();
	BOOL Deploy( void );
	void DoHolsterAnimation() override;
	int m_iSwing;
	TraceResult m_trHit;
};

#endif