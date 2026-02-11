// ============== Copyright (c) 2026 Monochrome Games ============== \\

#ifndef SHARED_WEAPON_PROJECTILES_BASE_H
#define SHARED_WEAPON_PROJECTILES_BASE_H

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#ifndef DECLARE_CLASS_SIMPLE
#define DECLARE_CLASS_SIMPLE( className, baseClassName ) typedef baseClassName BaseClass;
#endif

class CProjectileBase : public CBaseEntity
{
	DECLARE_CLASS_SIMPLE( CProjectileBase, CBaseEntity );

public:
	virtual bool ExplodeOnImpact() const { return false; }
	virtual void OnProjectileSpawn();
	virtual const char *GetHitSound( const bool &bHitBody ) const;
	virtual float GetDamage() const { return 0; }
	virtual float GetExplodeDamage() const { return 0; }
	void OnProjectileHit( const bool &bHitBody );
	void Spawn( void );
	void Precache( void );
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() | FCAP_MUST_RELEASE; }
	void EXPORT OnBubbleThink( void );
	void EXPORT OnProjectileTouch( CBaseEntity *pOther );
	void EXPORT OnExplodeThink( void );
};

// Create our projectile, using the given class as it's template.
#define CREATE_PROJECTILE_CLASS( projectileClass, projFuncName ) projectileClass projFuncName()
#define DECLARE_PROJECTILE_FUNCTION( projectileClass, projFuncName ) projectileClass projFuncName()		\
{																								\
	projectileClass pBolt = GetClassPtr( (projectileClass)NULL );								\
	if ( !pBolt ) return nullptr;																\
	pBolt->Spawn();																				\
	return pBolt;																				\
}

#endif