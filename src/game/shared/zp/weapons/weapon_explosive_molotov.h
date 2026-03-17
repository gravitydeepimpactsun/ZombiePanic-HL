// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_EXPLOSIVE_MOLOTOV_H
#define SHARED_WEAPON_EXPLOSIVE_MOLOTOV_H

#include "CWeaponBase.h"

class CBasePlayerItem;

class CWeaponExplosiveMolotov : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponExplosiveMolotov, CWeaponBase );

public:
	bool IsThrowable() override { return true; }
	ZPWeaponID GetWeaponID() override { return WEAPON_MOLOTOV; }
	BOOL UseDecrement() override { return FALSE; }
	int AddToPlayer( CBasePlayer *pPlayer );
	int AddDuplicate( CBasePlayerItem *pOriginal );
	void Spawn( void );
	void Precache( void );
	void PrimaryAttack();
	void SecondaryAttack();
	float Deploy();
	BOOL CanHolster();
	float DoHolsterAnimation() override;
	void WeaponIdle();

private:
	bool m_bDoSecondaryAttack = false;
	float m_flCanHolster = -1;
};

#endif
