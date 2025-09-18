// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SIDEARM_GLOCK17_H
#define SHARED_WEAPON_SIDEARM_GLOCK17_H

#include "CWeaponBase.h"

class CWeaponSideArmGlock17 : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponSideArmGlock17, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_GLOCK17; }
	bool IsAutomaticWeapon() const override { return false; }
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy();
	void Reload( void );
	void PrimaryAttack( void );
	void WeaponIdle( void );
};

#endif