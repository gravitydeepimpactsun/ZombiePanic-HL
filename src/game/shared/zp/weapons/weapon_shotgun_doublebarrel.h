// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SHOTGUN_DOUBLEBARREL_H
#define SHARED_WEAPON_SHOTGUN_DOUBLEBARREL_H

#include "CWeaponBase.h"

class CWeaponShotgunDoubleBarrel : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponShotgunDoubleBarrel, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_DOUBLEBARREL; }
	const char *GetEmptySound() const override { return "weapons/doublebarrel/dryfire.wav"; }
	float DoHolsterAnimation() override;
	void DoAudioFrame( void ) override;
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	float Deploy();
	float DoWeaponUnload();
	void Reload( void );
	void PrimaryAttack();
	void WeaponIdle( void );
};

#endif