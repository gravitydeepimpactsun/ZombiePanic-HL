// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SIDEARM_CZ75_H
#define SHARED_WEAPON_SIDEARM_CZ75_H

#include "CWeaponBase.h"

class CWeaponSideArmCZ75 : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponSideArmCZ75, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_CZ75; }
	bool IsAutomaticWeapon() const override { return false; }
	const char *GetEmptySound() const override { return "weapons/ppk/dryfire.wav"; }
	float DoHolsterAnimation() override;
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	float Deploy();
	float DoWeaponUnload();
	void Reload( void );
	void PrimaryAttack( void );
	void WeaponIdle( void );
};

#endif