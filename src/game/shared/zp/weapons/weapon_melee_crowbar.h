// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MELEE_CROWBAR_H
#define SHARED_WEAPON_MELEE_CROWBAR_H

#include "CWeaponBaseMelee.h"

class CWeaponMeleeCrowbar : public CWeaponBaseMelee
{
	DECLARE_CLASS_SIMPLE( CWeaponMeleeCrowbar, CWeaponBaseMelee );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_CROWBAR; }
	void Precache( void );
	const char *GetMeleeWorldModel() const { return "models/w_crowbar.mdl"; }
	const char *GetMeleeViewModel() const { return "models/v_crowbar.mdl"; }
	const char *GetMeleePlayerModel() const { return "models/p_crowbar.mdl"; }
	void DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld ) override;
	void DoWeaponSoundFromMiss( MeleeAttackType attackType ) override;
	float Deploy() override;
	float DoHolsterAnimation() override;
	float DoWeaponIdleAnimation( int iAnim ) override;
};

#endif