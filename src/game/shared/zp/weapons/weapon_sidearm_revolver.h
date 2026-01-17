// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_SIDEARM_REVOLVER_H
#define SHARED_WEAPON_SIDEARM_REVOLVER_H

#include "CWeaponBase.h"

class CWeaponSideArmRevolver : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponSideArmRevolver, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_PYTHON; }
	bool IsAutomaticWeapon() const override { return false; }
	void Spawn( void );
	void Precache( void );
	int AddToPlayer( CBasePlayer *pPlayer );
	void DoAudioFrame( void ) override;
	float DoHolsterAnimation() override;
	float Deploy();
	float DoWeaponUnload();
	void Reload( void );
	void PrimaryAttack( void );
	void WeaponIdle( void );
	inline bool HasBeenUnloaded() const { return ( m_bHasUnloaded && m_iClip == 0 ); }
	bool m_bHasUnloaded = false;
};

#endif