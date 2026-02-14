// ============== Copyright (c) 2026 Monochrome Games ============== \\

#include "CProjectileNail.h"

LINK_ENTITY_TO_CLASS( proj_nail, CProjectileNail );
DECLARE_PROJECTILE_FUNCTION( CProjectileNail*, CreateNailProjectile );

void CProjectileNail::OnProjectileSpawn()
{
	SET_MODEL( ENT(pev), "models/proj_nail.mdl" );
	// Make sure the base class gets called (we don't want an empty string on the kill feed)
	pev->classname = MAKE_STRING( "proj_nail" );
}

void CProjectileNail::Precache( void )
{
	PRECACHE_MODEL( "models/proj_nail.mdl" );
}

const char *CProjectileNail::GetHitSound( const bool &bHitBody ) const
{
	if ( bHitBody ) return "weapons/nailgun/proj_hitbody.wav";
	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0: return "weapons/nailgun/proj_hit.wav";
		case 1: return "weapons/nailgun/proj_hit2.wav";
		case 2: return "weapons/nailgun/proj_hit3.wav";
	}
	return "weapons/nailgun/proj_hit.wav";
}

float CProjectileNail::GetDamage() const
{
	return 25.0f;
}
