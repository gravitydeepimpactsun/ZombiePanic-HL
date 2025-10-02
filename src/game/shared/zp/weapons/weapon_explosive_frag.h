// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_EXPLOSIVE_FRAG_H
#define SHARED_WEAPON_EXPLOSIVE_FRAG_H

#include "CWeaponBase.h"

class CBasePlayerItem;

class CWeaponExplosiveFrag : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponExplosiveFrag, CWeaponBase );

public:
	bool IsThrowable() override { return true; }
	ZPWeaponID GetWeaponID() override { return WEAPON_HANDGRENADE; }
	int AddToPlayer( CBasePlayer *pPlayer );
	void Spawn( void );
	void Precache( void );
	void PrimaryAttack();
	BOOL Deploy();
	BOOL CanHolster();
	void Holster( int skiplocal = 0 );
	void WeaponIdle();
};

#endif