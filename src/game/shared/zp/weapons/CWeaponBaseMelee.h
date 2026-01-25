// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_BASE_MELEE_H
#define SHARED_WEAPON_BASE_MELEE_H

#include "CWeaponBase.h"

#define MELEE_SND_BODYHIT_VOLUME 128
#define MELEE_SND_WALLHIT_VOLUME 512

class CWeaponBaseMelee : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponBaseMelee, CWeaponBase );

public:
	bool IsMeleeWeapon() override { return true; }
	virtual bool CanUseHeavyAttack() const { return false; }
	void LoadMeleeConfigFile();
	int GetMeleeDamageTypeFromString( const char *szDmgType ) const;
	void Spawn( void );
	virtual void Precache( void ) {}
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void WeaponIdle();

	// Default deploy and holster animations, make sure you override these.
	virtual float Deploy( void );
	virtual float DoHolsterAnimation();
	virtual float DoWeaponIdleAnimation( int iAnim );

	enum MeleeAttackType
	{
		MELEE_ATTACK_NONE = 0,	// Default state
		MELEE_ATTACK_LIGHT,		// Primary attack
		MELEE_ATTACK_HEAVY		// Secondary attack
	};
	
	bool DidMeleeAttackHit( MeleeAttackType attackTrace );
	void DoMeleeAttack();

	bool IsAttackInProgress() const { return ( n_meleeAttackType != MELEE_ATTACK_NONE ); }
	bool IsInHeavyAttack() const { return ( n_meleeAttackType == MELEE_ATTACK_HEAVY ); }
	bool IsInLightAttack() const { return ( n_meleeAttackType == MELEE_ATTACK_LIGHT ); }

	// Melee models, uses crowbar model by default
	virtual const char *GetMeleeWorldModel() const { return "models/w_crowbar.mdl"; }
	virtual const char *GetMeleeViewModel() const { return "models/v_crowbar.mdl"; }
	virtual const char *GetMeleePlayerModel() const { return "models/p_crowbar.mdl"; }
	virtual const char *GetMeleePlayerExt() const { return "crowbar"; }

	float GetMeleeAttackRange( MeleeAttackType attackType ) const;
	float GetMeleeAttackDamage( MeleeAttackType attackType ) const;
	float GetMeleeAttackAnimationTime( MeleeAttackType attackType ) const;
	int GetMeleeDamageType( MeleeAttackType attackType ) const;

	virtual void DoWeaponSoundFromAttack( MeleeAttackType attackType, bool bHitWorld ) {}
	virtual void DoWeaponSoundFromMiss( MeleeAttackType attackType ) {}

protected:
	MeleeAttackType n_meleeAttackType;
	int m_iAttackTracerCount; // Amount of tracers per attack

	// Melee attack tracer info
	struct MeleeAttackTrace
	{
		int iDmgType;
		int iAmount;
		float flAnimTime;
		float flAnimTimeHold; // Only used for heavy attacks for now.
		float flRange;
		float flDamage;
		Vector vecStart;
		Vector vecEnd;
	};
	MeleeAttackTrace m_attackTracers[2];

	// Attack tracer
	TraceResult m_trHit;

private:
	struct MeleeAttackRecord
	{
		edict_t		*HitEntity;
		int			HitGroup;
		float		Fraction;
		bool		IsWorld;
		Vector		End;
		Vector		Direction;
	};
	bool IsEntityAlreadyHit( edict_t *pEntity, const std::vector<MeleeAttackRecord> &hitEntities );
};

#endif