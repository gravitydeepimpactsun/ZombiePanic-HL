// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SIDEARM_FAFO_H
#define SHARED_WEAPON_SIDEARM_FAFO_H

#include "CWeaponBase.h"

class CWeaponSideArmFafo : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponSideArmFafo, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_FAFO_ERW; }
	bool IsAutomaticWeapon() const override { return false; }
	float DoHolsterAnimation() override;
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	float Deploy();
	void PrimaryAttack( void );
	void WeaponIdle( void );
	void DeleteMe();
};

#endif