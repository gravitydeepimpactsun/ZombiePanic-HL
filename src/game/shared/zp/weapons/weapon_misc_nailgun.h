// ============== Copyright (c) 2026 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MISC_NAILGUN_H
#define SHARED_WEAPON_MISC_NAILGUN_H

#include "CWeaponBase.h"

class CWeaponNailGun : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponNailGun, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_NAILGUN; }
	const char *GetEmptySound() const override { return "weapons/nailgun/nailgun_dryfire.wav"; }
	float DoHolsterAnimation() override;
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	float Deploy();
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void WeaponIdle( void );

private:
};

#endif