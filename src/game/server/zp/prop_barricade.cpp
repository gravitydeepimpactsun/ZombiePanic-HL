// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "triggers.h"
#include "player.h"
#include "func_break.h"

extern void SetBodygroup( void *pmodel, entvars_t *pev, int iGroup, int iValue );
extern int gmsgBarricadeBuildProgress;

#define BGROUP_BODY 0
#define BGROUP_SUB_HEALTHY 0
#define BGROUP_SUB_DAMAGED 1
#define BGROUP_SUB_DESTROYED 2

#define SF_BARRICADE_START_BUILT (1<<0)

static char *s_blockClasses[] =
{
	"player",
	"prop_pushable",
	"func_pushable",
	"" // End of list
};

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
	EXPORT void OnBarricading();
	void StopBuilding();

protected:
	void SetSequenceBox();
	int ExtractBbox(int sequence, float *mins, float *maxs);
	void OnBarricadeBuilt();

private:
	int m_nHealthRem;

	enum BuildingState
	{
		NOT_BUILT = 0,
		BUILDING,
		BUILT
	};
	BuildingState m_eIsBuilt;
	int m_nBuilder;
	float m_flBuildTime;
	float m_flBuildStartTime;
};
LINK_ENTITY_TO_CLASS( prop_barricade, CPropBarricade );


void CPropBarricade::Precache()
{
	PRECACHE_SOUND( "barricade/built.wav" );
	PRECACHE_SOUND( "barricade/building.wav" );

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
	pev->skin = CONTENTS_BARRICADE_NOT_BUILT;
	pev->rendermode = kRenderTransAlpha;

	// Default build time
	m_flBuildTime = 1.6f;

	if (pev->friction > 399)
		pev->friction = 399;

	UTIL_SetOrigin(pev, pev->origin);

	m_bIsDestroyed = false;
	m_eIsBuilt = NOT_BUILT;
	m_nBuilder = 0;

	m_nHealthRem = pev->health;

	// Pre-Built?
	if ( pev->spawnflags & SF_BARRICADE_START_BUILT )
		OnBarricadeBuilt();
}


void CPropBarricade::Restart()
{
	SET_MODEL(ENT(pev), STRING(pev->model));

	SetBodygroup( GET_MODEL_PTR(ENT(pev)), pev, BGROUP_BODY, BGROUP_SUB_HEALTHY );
	pev->sequence = 0;
	SetSequenceBox(); // Copied from CBaseAnimating

	// Remember our previous health.
	pev->health = m_nHealthRem;

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_TRIGGER;
	pev->takedamage = DAMAGE_YES;
	pev->rendermode = kRenderTransAlpha;

	// Make sure skin is set to not built, so it appears invisible for zombies.
	// But if we have some woodenboards "ammo" we will be able to see and build it.
	pev->skin = CONTENTS_BARRICADE_NOT_BUILT;

	m_bIsDestroyed = false;
	m_eIsBuilt = NOT_BUILT;
	m_nBuilder = 0;

	// Pre-Built?
	if ( pev->spawnflags & SF_BARRICADE_START_BUILT )
		OnBarricadeBuilt();
}


void CPropBarricade::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "model" ) )
		pev->model = ALLOC_STRING( pkvd->szValue );
	else if ( FStrEq( pkvd->szKeyName, "health" ) )
		pev->health = atof( pkvd->szValue );
	else
		BaseClass::KeyValue( pkvd );
}


void CPropBarricade::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() )
		return;
	if ( m_eIsBuilt > NOT_BUILT ) return;

	// Make sure the activator has wooden boards to build with, aka ZPAmmoTypes::AMMO_BARRICADE.
	CBasePlayer *pPlayer = static_cast<CBasePlayer *>( pActivator );
	if ( !pPlayer ) return;
	if ( pPlayer->AmmoInventory( ZPAmmoTypes::AMMO_BARRICADE ) <= 0 )
		return;

	// Let's use UTIL_EntitiesInBox to check for entities in our bbox, since GoldSrc UTIL_TraceHull
	// does not have custom min/max size support...
	Vector mins = pev->mins;
	Vector maxs = pev->maxs;
	CBaseEntity *pList[32];
	int nCount = UTIL_EntitiesInBox( pList, 32, pev->origin + mins, pev->origin + maxs, 0 );
	for ( int i = 0; i < nCount; i++ )
	{
		// Ignore ourselves.
		if ( pList[i] == this ) continue;
		// Go through block classes.
		for ( int j = 0; s_blockClasses[j][0] != '\0'; j++ )
		{
			if ( FClassnameIs( pList[i]->pev, s_blockClasses[j] ) )
				return;
		}
	}

	m_eIsBuilt = BUILDING;
	m_nBuilder = pPlayer->entindex();
	m_flBuildStartTime = gpGlobals->time + m_flBuildTime;
	pPlayer->SetAnimation( PLAYER_BARRICADE );

	// Send build progress message to the player.
	MESSAGE_BEGIN( MSG_ONE, gmsgBarricadeBuildProgress, NULL, pPlayer->pev );
		WRITE_FLOAT( m_flBuildTime );
	MESSAGE_END();

	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "barricade/building.wav", 1.0, ATTN_NORM );

	SetThink( &CPropBarricade::OnBarricading );
	pev->nextthink = gpGlobals->time + 0.1f;
}


void CPropBarricade::OnBarricadeBuilt()
{
	CBasePlayer *pPlayer = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( m_nBuilder ) );
	if ( pPlayer )
	{
		pPlayer->SetAnimation( PLAYER_IDLE );
		pPlayer->m_rgAmmo[ ZPAmmoTypes::AMMO_BARRICADE ] = 0; // Consume the wooden boards.
	}

	// No more thinking!
	SetThink( NULL );

	// Set bodygroup to healthy state.
	SetBodygroup( GET_MODEL_PTR(ENT(pev)), pev, BGROUP_BODY, BGROUP_SUB_HEALTHY );

	// Now, let's make sure we set the barricade to be solid.
	pev->solid = SOLID_BBOX;
	pev->skin = 0;
	pev->rendermode = kRenderNormal; // Make sure it's fully visible now.
	m_eIsBuilt = BUILT;
	m_nBuilder = 0;

	// Let's play a simple button sound to indicate building is done.
	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "barricade/built.wav", 1.0, ATTN_NORM );
}


int CPropBarricade::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if ( m_bIsDestroyed ) return 0;
	int nPrevHealth = pev->health;
	int nRet = BaseClass::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
	if ( nRet > 0 )
		SetBodygroup( GET_MODEL_PTR(ENT(pev)), pev, BGROUP_BODY, BGROUP_SUB_DAMAGED );
	return nRet;
}


void CPropBarricade::StopBuilding()
{
	CBasePlayer *pPlayer = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( m_nBuilder ) );
	if ( pPlayer )
	{
		// Reset build progress.
		MESSAGE_BEGIN( MSG_ONE, gmsgBarricadeBuildProgress, NULL, pPlayer->pev );
			WRITE_FLOAT( 0.0f );
		MESSAGE_END();
		pPlayer->SetAnimation( PLAYER_IDLE );
	}

	// Stop the damn sound
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "common/null.wav", 0, ATTN_NORM, 0, 100 );

	m_eIsBuilt = NOT_BUILT;
	m_nBuilder = 0;
	SetThink( NULL );
}


void CPropBarricade::OnBarricading()
{
	bool bStopThinking = false;
	CBasePlayer *pPlayer = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( m_nBuilder ) );
	if ( !pPlayer )
		bStopThinking = true;
	else
	{
		// Make sure we can't move
		pPlayer->pev->velocity = pPlayer->pev->basevelocity = g_vecZero;

		// Still holding use?
		if ( !( pPlayer->pev->button & IN_USE ) )
			bStopThinking = true;
		// Out of ammo for some reason?
		else if ( pPlayer->AmmoInventory( ZPAmmoTypes::AMMO_BARRICADE ) <= 0 )
			bStopThinking = true;
	}

	// Stop building?
	if ( bStopThinking )
	{
		StopBuilding();
		return;
	}

	// Fully built?
	if ( m_flBuildStartTime - gpGlobals->time <= 0.0f )
	{
		OnBarricadeBuilt();
		return;
	}

	pev->nextthink = gpGlobals->time;
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
