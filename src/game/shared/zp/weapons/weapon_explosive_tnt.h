// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_EXPLOSIVE_FRAG_H
#define SHARED_WEAPON_EXPLOSIVE_FRAG_H

#include "CWeaponBase.h"

class CBasePlayerItem;

class CWeaponExplosiveTNT : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponExplosiveTNT, CWeaponBase );

public:
	bool IsThrowable() override { return true; }
	void DeactivateThrow() override;
	ZPWeaponID GetWeaponID() override { return WEAPON_TNT; }
	int AddToPlayer( CBasePlayer *pPlayer );
	int AddDuplicate( CBasePlayerItem *pOriginal );
	void Spawn( void );
	void Precache( void );
	void PrimaryAttack();
	BOOL Deploy();
	BOOL CanHolster();
	void DoHolsterAnimation() override;
	void WeaponIdle();
};

#endif