// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_RIFLE_SKS_H
#define SHARED_WEAPON_RIFLE_SKS_H

#include "CWeaponBase.h"

class CWeaponRifleSKS : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponRifleSKS, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_SKS; }
	bool IsAutomaticWeapon() const override { return false; }
	const char *GetEmptySound() const override { return "weapons/556ar/dryfire.wav"; }
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