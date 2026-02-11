// ============== Copyright (c) 2026 Monochrome Games ============== \\

#include "CProjectileBase.h"
#include "weapons.h"

void CProjectileBase::OnProjectileSpawn()
{
	SET_MODEL( ENT(pev), "models/proj_generic.mdl" );
}

void CProjectileBase::OnProjectileHit( const bool &bHitBody )
{
	const char *pHitSound = GetHitSound( bHitBody );
	if ( !pHitSound ) return;
	EMIT_SOUND_DYN( ENT(pev), CHAN_BODY, pHitSound, RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7) );
}

const char *CProjectileBase::GetHitSound( const bool &bHitBody ) const
{
	if ( bHitBody )
	{
		switch ( RANDOM_LONG(0, 1) )
		{
			case 0: return "weapons/proj/hitbod1.wav";
			case 1: return "weapons/proj/hitbod2.wav";
		}
	}
	return "weapons/proj/hit.wav";
}

void CProjectileBase::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;
	pev->gravity = 0.5;

	OnProjectileSpawn();

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch( &CProjectileBase::OnProjectileTouch );
	SetThink( &CProjectileBase::OnBubbleThink );
	pev->nextthink = gpGlobals->time + 0.2;
}

void CProjectileBase::Precache()
{
	PRECACHE_MODEL( "models/proj_generic.mdl" );
}

void CProjectileBase::OnBubbleThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1;
#if !defined( CLIENT_DLL )
	if ( pev->waterlevel == 0 ) return;
	UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 1 );
#endif
}

void CProjectileBase::OnProjectileTouch( CBaseEntity *pOther )
{
#if !defined( CLIENT_DLL )
	SetTouch(NULL);
	SetThink(NULL);

	if ( pOther->pev->takedamage )
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		entvars_t *pevOwner = VARS( pev->owner );

		ClearMultiDamage();
		pOther->TraceAttack( pevOwner, GetDamage(), pev->velocity.Normalized(), &tr, DMG_NEVERGIB );
		ApplyMultiDamage(pev, pevOwner);

		// play body "thwack" sound
		OnProjectileHit( true );
	}
	else
	{
		OnProjectileHit( false );

		SetThink( &CProjectileBase::SUB_Remove );
		pev->nextthink = gpGlobals->time; // this will get changed below if the bolt is allowed to stick in what it hit.

		if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		{
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = pev->velocity.Normalized();
			UTIL_SetOrigin(pev, pev->origin - vecDir * 12);
			pev->angles = UTIL_VecToAngles(vecDir);
			pev->avelocity.z = 0;
			pev->angles.z = RANDOM_LONG(0, 360);
			pev->nextthink = gpGlobals->time + 10.0;
		}

		if ( UTIL_PointContents(pev->origin) != CONTENTS_WATER )
			UTIL_Sparks(pev->origin);
	}

	pev->solid = SOLID_NOT;
	pev->velocity = Vector( 0, 0, 0 );

	if ( ExplodeOnImpact() )
	{
		SetThink( &CProjectileBase::OnExplodeThink );
		pev->nextthink = gpGlobals->time + 0.1;
	}
#endif
}

void CProjectileBase::OnExplodeThink( void )
{
#if !defined( CLIENT_DLL )
	int iContents = UTIL_PointContents(pev->origin);
	int iScale;

	pev->dmg = GetExplodeDamage();
	iScale = 10;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
	WRITE_BYTE( TE_EXPLOSION );
	WRITE_COORD( pev->origin.x );
	WRITE_COORD( pev->origin.y );
	WRITE_COORD( pev->origin.z );
	if ( iContents != CONTENTS_WATER )
		WRITE_SHORT( g_sModelIndexFireball );
	else
		WRITE_SHORT( g_sModelIndexWExplosion );
	WRITE_BYTE( iScale ); // scale * 10
	WRITE_BYTE( 15 ); // framerate
	WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();

	entvars_t *pevOwner = nullptr;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	pev->owner = nullptr;

	::RadiusDamage( pev->origin, pev, pevOwner, pev->dmg, 128, CLASS_NONE, DMG_BLAST | DMG_ALWAYSGIB );

	UTIL_Remove( this );
#endif
}
