// ============== Copyright (c) 2026 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MELEE_WALKSTICK_H
#define SHARED_WEAPON_MELEE_WALKSTICK_H

#include "CWeaponBaseMelee.h"

class CWeaponMeleeWalkStick : public CWeaponBaseMelee
{
	DECLARE_CLASS_SIMPLE( CWeaponMeleeWalkStick, CWeaponBaseMelee );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_WALKSTICK; }
	void Precache( void );
	const char *GetMeleeWorldModel() const { return "models/w_walkstick.mdl"; }
	const char *GetMeleeViewModel() const { return "models/v_walkstick.mdl"; }
	const char *GetMeleePlayerModel() const { return "models/p_walkstick.mdl"; }
	void DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld ) override;
	void DoWeaponSoundFromMiss( MeleeAttackType attackType ) override;
	float Deploy() override;
	float DoHolsterAnimation() override;
	float DoWeaponIdleAnimation( int iAnim ) override;
	float GetAttackAnimationTime( int iAnim ) override;
};

#endif