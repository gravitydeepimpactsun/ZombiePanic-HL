// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SIDEARM_SIG_H
#define SHARED_WEAPON_SIDEARM_SIG_H

#include "CWeaponBase.h"

class CWeaponSideArmSig : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponSideArmSig, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_SIG; }
	bool IsAutomaticWeapon() const override { return false; }
	const char *GetEmptySound() const override { return "weapons/sig/dryfire.wav"; }
	void DoHolsterAnimation() override;
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy();
	void Reload( void );
	void PrimaryAttack( void );
	void WeaponIdle( void );
};

#endif