// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_RIFLE_M16_H
#define SHARED_WEAPON_RIFLE_M16_H

#include "CWeaponBase.h"

class CWeaponRifleM16 : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponRifleM16, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_556AR; }
	const char *GetEmptySound() const override { return "weapons/556ar/dryfire.wav"; }
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