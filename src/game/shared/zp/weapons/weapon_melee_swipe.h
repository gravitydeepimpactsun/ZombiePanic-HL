// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MELEE_SWIPE_H
#define SHARED_WEAPON_MELEE_SWIPE_H

#include "CWeaponBaseMelee.h"

class CWeaponMeleeSwipe : public CWeaponBaseMelee
{
	DECLARE_CLASS_SIMPLE( CWeaponMeleeSwipe, CWeaponBaseMelee );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_SWIPE; }
	void Spawn( void ) override;
	void Precache( void );
	const char *GetMeleeWorldModel() const { return "models/w_swipe.mdl"; }
	const char *GetMeleeViewModel() const { return "models/v_swipe.mdl"; }
	const char *GetMeleePlayerModel() const { return "models/p_swipe.mdl"; }
	const char *GetMeleePlayerExt() const { return "swipe"; }
	Bullet GetBulletType() const { return BULLET_PLAYER_SWIPE; }
	void DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld ) override;
	void DoWeaponSoundFromMiss( MeleeAttackType attackType ) override;
	float Deploy() override;
	float DoHolsterAnimation() override;
	float DoWeaponIdleAnimation( int iAnim ) override;
	float GetAttackAnimationTime( int iAnim ) override;
};

#endif