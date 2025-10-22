// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_EXPLOSIVE_SATCHEL_H
#define SHARED_WEAPON_EXPLOSIVE_SATCHEL_H

#include "CWeaponBase.h"

class CBasePlayerItem;

class CWeaponExplosiveSatchel : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponExplosiveSatchel, CWeaponBase );

public:
	bool IsThrowable() override { return true; }
	ZPWeaponID GetWeaponID() override { return WEAPON_SATCHEL; }
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	int AddDuplicate( CBasePlayerItem *pOriginal );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	bool HasSatchelCharge() const;

	float Deploy();

	float DoHolsterAnimation() override;
	void DeactivateThrow() override;
	void WeaponIdle();
	void Throw();

private:
	bool m_bFirstThrow;
	bool m_bHasThrownSatchel;
	bool m_bHasDetonatedSatchel;
	int m_iPrevClip;
};

#endif