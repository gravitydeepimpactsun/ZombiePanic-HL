// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SHOTGUN_REMINGTON_H
#define SHARED_WEAPON_SHOTGUN_REMINGTON_H

#include "CWeaponBaseSingleAction.h"

class CWeaponShotgunRemington : public CWeaponBaseSingleAction
{
	DECLARE_CLASS_SIMPLE( CWeaponShotgunRemington, CWeaponBaseSingleAction );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_SHOTGUN; }
	const char *GetEmptySound() const override { return "weapons/shotgun/dryfire.wav"; }
	float DoHolsterAnimation() override;
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	float Deploy();
	void OnRequestedAnimation( SingleActionAnimReq act );
	void OnWeaponPrimaryAttack();
};

#endif