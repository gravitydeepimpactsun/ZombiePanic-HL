// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SHOTGUN_DOUBLEBARREL_H
#define SHARED_WEAPON_SHOTGUN_DOUBLEBARREL_H

#include "CWeaponBaseSingleAction.h"

class CWeaponShotgunDoubleBarrel : public CWeaponBaseSingleAction
{
	DECLARE_CLASS_SIMPLE( CWeaponShotgunDoubleBarrel, CWeaponBaseSingleAction );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_DOUBLEBARREL; }
	const char *GetEmptySound() const override { return "weapons/doublebarrel/dryfire.wav"; }
	BOOL PlayEmptySound() override;
	bool PumpIsRequired() const override { return false; }
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	BOOL Deploy();
	void Holster( int skiplocal = 0 );
	void OnRequestedAnimation( SingleActionAnimReq act );
	void OnWeaponPrimaryAttack();
};

#endif