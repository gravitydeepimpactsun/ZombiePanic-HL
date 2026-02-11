// ============== Copyright (c) 2026 Monochrome Games ============== \\

#include "CProjectileNail.h"

LINK_ENTITY_TO_CLASS( proj_nail, CProjectileNail );
DECLARE_PROJECTILE_FUNCTION( CProjectileNail*, CreateNailProjectile );

void CProjectileNail::OnProjectileSpawn()
{
	SET_MODEL( ENT(pev), "models/proj_nail.mdl" );
}

void CProjectileNail::Precache( void )
{
	PRECACHE_MODEL( "models/proj_nail.mdl" );
}

const char *CProjectileNail::GetHitSound( const bool &bHitBody ) const
{
	if ( bHitBody ) return "weapons/nailgun/hitbod.wav";
	return "weapons/nailgun/hit.wav";
}

float CProjectileNail::GetDamage() const
{
	return 25.0f;
}
