// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "triggers.h"
#include "player.h"
#include "func_break.h"

extern void SetBodygroup( void *pmodel, entvars_t *pev, int iGroup, int iValue );

#define BGROUP_BODY 0
#define BGROUP_SUB_HEALTHY 0
#define BGROUP_SUB_DAMAGED 1
#define BGROUP_SUB_DESTROYED 2

class CPropBarricade : public CBreakable
{
	SET_BASECLASS( CBreakable );

public:
	void Spawn() override;
	void Precache() override;
	void SoftRemove() override;
	void Restart();
	void KeyValue( KeyValueData *pkvd ) override;
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps(void) { return CBaseEntity::ObjectCaps() | FCAP_MUST_RESET | FCAP_CONTINUOUS_USE; }
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

protected:
	void SetSequenceBox();
	int ExtractBbox(int sequence, float *mins, float *maxs);

private:
	bool m_bIsBuilt;
};
LINK_ENTITY_TO_CLASS( prop_barricade, CPropBarricade );

void CPropBarricade::Precache()
{
	PRECACHE_MODEL( (char *)STRING(pev->model) );
	for (int i = 0; i < 3; i++)
		PRECACHE_SOUND( CPushable::m_soundNames[i] );
	BaseClass::Precache();
}

void CPropBarricade::SoftRemove()
{
	BaseClass::SoftRemove();
	pev->effects = 0;
	SetBodygroup( GET_MODEL_PTR(ENT(pev)), pev, BGROUP_BODY, BGROUP_SUB_DESTROYED );
}


void CPropBarricade::Spawn()
{
	Precache();

	SET_MODEL( ENT(pev), STRING(pev->model) );

	SetBodygroup( GET_MODEL_PTR(ENT(pev)), pev, BGROUP_BODY, BGROUP_SUB_HEALTHY );
	pev->sequence = 0;
	SetSequenceBox(); // Copied from CBaseAnimating

	// Just add extra 26 if Z is 1...
	if ( pev->maxs.z == 1 )
		pev->maxs.z += 26;

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_TRIGGER;
	pev->takedamage = DAMAGE_YES;

	if (pev->friction > 399)
		pev->friction = 399;

	UTIL_SetOrigin(pev, pev->origin);

	m_bIsDestroyed = false;
	m_bIsBuilt = false;
}


void CPropBarricade::Restart()
{
	SET_MODEL(ENT(pev), STRING(pev->model));

	SetBodygroup( GET_MODEL_PTR(ENT(pev)), pev, BGROUP_BODY, BGROUP_SUB_HEALTHY );
	pev->sequence = 0;
	SetSequenceBox(); // Copied from CBaseAnimating

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_TRIGGER;
	pev->takedamage = DAMAGE_YES;

	m_bIsDestroyed = false;
	m_bIsBuilt = false;
}


void CPropBarricade::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "model" ) )
		pev->model = ALLOC_STRING( pkvd->szValue );
	else
		BaseClass::KeyValue( pkvd );
}


void CPropBarricade::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
		return;
	if ( m_bIsBuilt ) return;
	// TODO: Build the barricade, and check if anyone is blocking it (somebody inside our BBOX)
}


int CPropBarricade::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if ( m_bIsDestroyed ) return 0;
	int nPrevHealth = pev->health;
	int nRet = BaseClass::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
	if ( pev->health > 0 && nPrevHealth < pev->health )
		SetBodygroup( GET_MODEL_PTR(ENT(pev)), pev, BGROUP_BODY, BGROUP_SUB_DAMAGED );
	return nRet;
}


//=========================================================
//=========================================================

extern int ExtractBbox( void *pmodel, int sequence, float *mins, float *maxs );

int CPropBarricade::ExtractBbox( int sequence, float *mins, float *maxs )
{
	return ::ExtractBbox( GET_MODEL_PTR(ENT(pev)), sequence, mins, maxs );
}

void CPropBarricade::SetSequenceBox()
{
	Vector mins, maxs;

	// Get sequence bbox
	if ( ExtractBbox( pev->sequence, mins, maxs ) )
	{
		// expand box for rotation
		// find min / max for rotations
		float yaw = pev->angles.y * (M_PI / 180.0);

		Vector xvector, yvector;
		xvector.x = cos(yaw);
		xvector.y = sin(yaw);
		yvector.x = -sin(yaw);
		yvector.y = cos(yaw);
		Vector bounds[2];

		bounds[0] = mins;
		bounds[1] = maxs;

		Vector rmin(9999, 9999, 9999);
		Vector rmax(-9999, -9999, -9999);
		Vector base, transformed;

		for (int i = 0; i <= 1; i++)
		{
			base.x = bounds[i].x;
			for (int j = 0; j <= 1; j++)
			{
				base.y = bounds[j].y;
				for (int k = 0; k <= 1; k++)
				{
					base.z = bounds[k].z;

					// transform the point
					transformed.x = xvector.x * base.x + yvector.x * base.y;
					transformed.y = xvector.y * base.x + yvector.y * base.y;
					transformed.z = base.z;

					if (transformed.x < rmin.x)
						rmin.x = transformed.x;
					if (transformed.x > rmax.x)
						rmax.x = transformed.x;
					if (transformed.y < rmin.y)
						rmin.y = transformed.y;
					if (transformed.y > rmax.y)
						rmax.y = transformed.y;
					if (transformed.z < rmin.z)
						rmin.z = transformed.z;
					if (transformed.z > rmax.z)
						rmax.z = transformed.z;
				}
			}
		}
		UTIL_SetSize(pev, rmin, rmax);
	}
}
