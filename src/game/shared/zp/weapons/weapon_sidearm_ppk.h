// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SIDEARM_PPK_H
#define SHARED_WEAPON_SIDEARM_PPK_H

#include "CWeaponBase.h"

class CWeaponSideArmPPK : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponSideArmPPK, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_PPK; }
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