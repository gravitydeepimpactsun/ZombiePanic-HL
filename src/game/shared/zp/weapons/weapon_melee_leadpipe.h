// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_MELEE_LEADPIPE_H
#define SHARED_WEAPON_MELEE_LEADPIPE_H

#include "CWeaponBase.h"

class CWeaponMeleeLeadPipe : public CWeaponBase
{
	DECLARE_CLASS_SIMPLE( CWeaponMeleeLeadPipe, CWeaponBase );

public:
	ZPWeaponID GetWeaponID() override { return WEAPON_LEADPIPE; }
	void Spawn( void );
	void Precache( void );
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int AddToPlayer( CBasePlayer *pPlayer );

	void PrimaryAttack( void );
	void SecondaryAttack( void ) override;
	int Swing( int fFirst );
	void DoHeavyAttack();
	void WeaponIdle();
	BOOL Deploy( void );
	void DoHolsterAnimation() override;
	int m_iSwing;
	TraceResult m_trHit;

private:
	bool m_bIsInHeavyAttack = false;
};

#endif