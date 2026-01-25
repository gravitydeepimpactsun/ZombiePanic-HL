// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MELEE_LEADPIPE_H
#define SHARED_WEAPON_MELEE_LEADPIPE_H

#include "CWeaponBaseMelee.h"

class CWeaponMeleeLeadPipe : public CWeaponBaseMelee
{
	DECLARE_CLASS_SIMPLE( CWeaponMeleeLeadPipe, CWeaponBaseMelee );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_LEADPIPE; }
	bool CanUseHeavyAttack() const override { return true; }
	void Precache( void );
	const char *GetMeleeWorldModel() const { return "models/w_leadpipe.mdl"; }
	const char *GetMeleeViewModel() const { return "models/v_leadpipe.mdl"; }
	const char *GetMeleePlayerModel() const { return "models/p_leadpipe.mdl"; }
	void DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld ) override;
	void DoWeaponSoundFromMiss( MeleeAttackType attackType ) override;
};

#endif