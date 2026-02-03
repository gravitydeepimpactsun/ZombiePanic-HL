// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_EXPLOSIVE_SATCHEL_H
#define SHARED_WEAPON_EXPLOSIVE_SATCHEL_H

#include "CWeaponBase.h"

class CBasePlayerItem;

class CWeaponExplosiveIED : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponExplosiveIED, CWeaponBase );

public:
	bool AllowAmmoSteal() const override { return false; }
	bool IsThrowable() override { return true; }
	ZPWeaponID GetWeaponID() override { return WEAPON_SATCHEL; }
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	bool HasSatchelCharge() const;

	void OnClipIncrease( int iAmount ) override;

	float Deploy();

	float DoHolsterAnimation() override;
	void WeaponIdle();
	bool BeginThrowOrPlant();
	bool PlantIED();

private:

	enum IEDState
	{
		IED_STATE_NORMAL,			// Hasn't thrown yet
		IED_STATE_THROWING,			// In the process of throwing
		IED_STATE_HAS_THROWN,		// Has thrown, waiting to detonate
		IED_STATE_DETONATED,		// Has detonated
		IED_STATE_OBTAINED_PACKAGE,	// Picked up package after throwing, we need to holster and then deploy it again.
		IED_STATE_TO_NORMAL			// Transitioning back to normal after obtaining package
	};

	IEDState m_iIEDState;
	int m_iPrevClip;
};

#endif