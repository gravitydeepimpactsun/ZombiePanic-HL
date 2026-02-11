// ============== Copyright (c) 2026 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_PROJECTILES_NAIL_H
#define SHARED_WEAPON_PROJECTILES_NAIL_H

#include "CProjectileBase.h"

class CProjectileNail : public CProjectileBase
{
	DECLARE_CLASS_SIMPLE( CProjectileNail, CProjectileBase );
public:
	void OnProjectileSpawn() override;
	void Precache( void ) override;
	const char *GetHitSound( const bool &bHitBody ) const override;
	float GetDamage() const override;
};
CREATE_PROJECTILE_CLASS( CProjectileNail*, CreateNailProjectile );

#endif