// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SMG_MP5_H
#define SHARED_WEAPON_SMG_MP5_H

#include "CWeaponBase.h"

class CWeaponSMGMP5 : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponSMGMP5, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_MP5; }
	const char *GetEmptySound() const override { return "weapons/mp5/dryfire.wav"; }
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy();
	void Reload( void );
	void PrimaryAttack( void );
	void WeaponIdle( void );
};

#endif