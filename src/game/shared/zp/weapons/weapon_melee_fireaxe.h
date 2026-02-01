// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MELEE_FIREAXE_H
#define SHARED_WEAPON_MELEE_FIREAXE_H

#include "CWeaponBaseMelee.h"

class CWeaponMeleeFireaxe : public CWeaponBaseMelee
{
	DECLARE_CLASS_SIMPLE( CWeaponMeleeFireaxe, CWeaponBaseMelee );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_FIREAXE; }
	bool CanUseHeavyAttack() const override { return true; }
	void Precache( void );
	const char *GetMeleeWorldModel() const { return "models/w_fireaxe.mdl"; }
	const char *GetMeleeViewModel() const { return "models/v_fireaxe.mdl"; }
	const char *GetMeleePlayerModel() const { return "models/p_fireaxe.mdl"; }
	const char *GetMeleePlayerExt() const { return "2handed"; }
	Bullet GetBulletType() const { return BULLET_PLAYER_SHARP; }
	void DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld ) override;
	void DoWeaponSoundFromMiss( MeleeAttackType attackType ) override;
	float Deploy() override;
	float DoHolsterAnimation() override;
	float DoWeaponIdleAnimation( int iAnim ) override;
};

#endif