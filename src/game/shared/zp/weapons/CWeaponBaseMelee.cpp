// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "CWeaponBaseMelee.h"

#include <tier2/tier2.h>
#include "FileSystem.h"
#include <KeyValues.h>

#ifndef CLIENT_DLL
#include "game.h"
#include "gamerules.h"
#endif

// Loads the melee weapon configuration file.
// Here we read values for the different melee attacks, such as damage, range, animation times and tracers.
void CWeaponBaseMelee::LoadMeleeConfigFile()
{
	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );

	// Read the melee weapon keyvalue file
	std::string szFile( "scripts/" + std::string( slot.Classname ) + ".txt" );
	KeyValues *pWeaponScript = new KeyValues( "WeaponInfo" );
	if ( !pWeaponScript->LoadFromFile( g_pFullFileSystem, szFile.c_str() ) )
	{
		pWeaponScript->deleteThis();
		Warning( "%s is missing the weapon config file !!!!\n", slot.Classname );
		return;
	}

	KeyValues *pPrimaryAttack = pWeaponScript->FindKey( "Primary" );
	if ( pPrimaryAttack )
	{
		m_attackTracers[0].flDamage = pPrimaryAttack->GetFloat( "Damage", 25.0f );
		m_attackTracers[0].flAnimTime = pPrimaryAttack->GetFloat( "AnimTime", 1.0f );
		m_attackTracers[0].flAnimTimeHold = pPrimaryAttack->GetFloat( "AnimTimeHold", 1.0f );
	}

	KeyValues *pSecondaryAttack = pWeaponScript->FindKey( "Secondary" );
	if ( pSecondaryAttack )
	{
		m_attackTracers[1].flDamage = pSecondaryAttack->GetFloat( "Damage", 25.0f );
		m_attackTracers[1].flAnimTime = pSecondaryAttack->GetFloat( "AnimTime", 1.0f );
		m_attackTracers[1].flAnimTimeHold = pSecondaryAttack->GetFloat( "AnimTimeHold", 1.0f );
	}

	KeyValues *pAttackMap = pWeaponScript->FindKey( "AttackMap" );
	if ( pAttackMap )
	{
		pPrimaryAttack = pAttackMap->FindKey( "Primary" );
		if ( pPrimaryAttack )
		{
			m_attackTracers[0].iDmgType = GetMeleeDamageTypeFromString( pPrimaryAttack->GetString( "Type", "BLUNT" ) );
			m_attackTracers[0].iAmount = pPrimaryAttack->GetInt( "Count", 16 );
			m_attackTracers[0].flRange = pPrimaryAttack->GetFloat( "Length", 58 );
			UTIL_StringToVector( m_attackTracers[0].vecStart[0].Base(), pPrimaryAttack->GetString( "Start", "20 -10 8" ) );
			UTIL_StringToVector( m_attackTracers[0].vecEnd[0].Base(), pPrimaryAttack->GetString( "End", "20 10 -10" ) );
			UTIL_StringToVector( m_attackTracers[0].vecStart[1].Base(), pPrimaryAttack->GetString( "Start2", "0 0 0" ) );
			UTIL_StringToVector( m_attackTracers[0].vecEnd[1].Base(), pPrimaryAttack->GetString( "End2", "0 0 0" ) );
		}

		pSecondaryAttack = pAttackMap->FindKey( "Secondary" );
		if ( pSecondaryAttack )
		{
			m_attackTracers[1].iDmgType = GetMeleeDamageTypeFromString( pSecondaryAttack->GetString( "Type", "BLUNT" ) );
			m_attackTracers[1].iAmount = pSecondaryAttack->GetInt( "Count", 16 );
			m_attackTracers[1].flRange = pSecondaryAttack->GetFloat( "Length", 58 );
			UTIL_StringToVector( m_attackTracers[1].vecStart[0].Base(), pSecondaryAttack->GetString( "Start", "20 -10 8" ) );
			UTIL_StringToVector( m_attackTracers[1].vecEnd[0].Base(), pSecondaryAttack->GetString( "End", "20 10 -10" ) );
			UTIL_StringToVector( m_attackTracers[1].vecStart[1].Base(), pPrimaryAttack->GetString( "Start2", "0 0 0" ) );
			UTIL_StringToVector( m_attackTracers[1].vecEnd[1].Base(), pPrimaryAttack->GetString( "End2", "0 0 0" ) );
		}
	}
}

int CWeaponBaseMelee::GetMeleeDamageTypeFromString( const char *szDmgType ) const
{
	int iDmgType = 0;
	if ( Q_stristr( szDmgType, "CRUSH" ) != 0 )			iDmgType |= DMG_CRUSH;
	if ( Q_stristr( szDmgType, "BLUNT" ) != 0 )			iDmgType |= DMG_CLUB;
	if ( Q_stristr( szDmgType, "SHARP" ) != 0 )			iDmgType |= DMG_SLASH;
	if ( Q_stristr( szDmgType, "ALWAYSGIB" ) != 0 )		iDmgType |= DMG_ALWAYSGIB;
	return iDmgType;
}

void CWeaponBaseMelee::Spawn()
{
	Precache();
	SET_MODEL( ENT(pev), GetMeleeWorldModel() );
	m_iClip = -1;
	n_meleeAttackType = MELEE_ATTACK_NONE;
	DefaultSpawn();
	LoadMeleeConfigFile();
}

int CWeaponBaseMelee::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup(pPlayer);
		return TRUE;
	}
	return FALSE;
}

float CWeaponBaseMelee::Deploy()
{
	DoDeploy( GetMeleeViewModel(), GetMeleePlayerModel(), ANIM_MELEE_DRAW, GetMeleePlayerExt() );
	return GetAnimationTime( 24, 30 );
}

float CWeaponBaseMelee::DoHolsterAnimation()
{
	SendWeaponAnim( ANIM_MELEE_HOLSTER );
	return GetAnimationTime( 24, 30 );
}

void CWeaponBaseMelee::PrimaryAttack( void )
{
	if ( IsAttackInProgress() ) return;
	n_meleeAttackType = MELEE_ATTACK_LIGHT;
	DoMeleeAttack();
	// So we can whack again after the primary fire rate
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + PrimaryFireRate();
}

void CWeaponBaseMelee::SecondaryAttack( void )
{
	if ( !CanUseHeavyAttack() ) return;
	if ( IsInHeavyAttack() )
	{
		m_pPlayer->SetAnimation( PLAYER_ATTACK2_HOLD );
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
		return;
	}
	// Already attacking?
	if ( IsAttackInProgress() ) return;
	SendWeaponAnim( ANIM_MELEE_HEAVY_HOLD );
	m_pPlayer->SetAnimation( PLAYER_ATTACK2_PRE );
	n_meleeAttackType = MELEE_ATTACK_HEAVY;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

float CWeaponBaseMelee::DoWeaponIdleAnimation( int iAnim )
{
	float flTime = 1.0f;
	switch ( RANDOM_LONG(0, 4) )
	{
		default:
		case ANIM_MELEE_IDLE1: flTime = 2.75f; break;
		case ANIM_MELEE_IDLE2: flTime = 2.5f; break;
	    case ANIM_MELEE_IDLE3: flTime = 3.33f; break;
	}
	return flTime;
}

float CWeaponBaseMelee::GetMeleeAttackRange( MeleeAttackType attackType ) const
{
	return m_attackTracers[ attackType == MELEE_ATTACK_HEAVY ? 1 : 0 ].flRange;
}

float CWeaponBaseMelee::GetMeleeAttackDamage( MeleeAttackType attackType ) const
{
	return m_attackTracers[ attackType == MELEE_ATTACK_HEAVY ? 1 : 0 ].flDamage;
}

float CWeaponBaseMelee::GetMeleeAttackAnimationTime( MeleeAttackType attackType ) const
{
	return m_attackTracers[ attackType == MELEE_ATTACK_HEAVY ? 1 : 0 ].flAnimTime;
}

int CWeaponBaseMelee::GetMeleeDamageType(MeleeAttackType attackType) const
{
	return m_attackTracers[ attackType == MELEE_ATTACK_HEAVY ? 1 : 0 ].iDmgType;
}

bool CWeaponBaseMelee::IsEntityAlreadyHit( edict_t *pEntity, const std::vector<edict_t *> &hitEntities )
{
	for ( size_t i = 0; i < hitEntities.size(); ++i )
	{
		if ( hitEntities[i] == pEntity )
			return true;
	}
	return false;
}

void CWeaponBaseMelee::WeaponIdle()
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ) return;
	if ( IsAttackInProgress() )
	{
		DoMeleeAttack();
		return;
	}
	int iAnim;
	switch ( RANDOM_LONG(0, 4) )
	{
		default:
		case 0: iAnim = ANIM_MELEE_IDLE1; break;
		case 2: iAnim = ANIM_MELEE_IDLE2; break;
		case 4: iAnim = ANIM_MELEE_IDLE3; break;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + DoWeaponIdleAnimation( iAnim );
	SendWeaponAnim( iAnim );
}

#ifndef CLIENT_DLL
static ConVar sv_melee_debug_traces( "sv_melee_debug_traces", "0", FCVAR_CHEATS, "Enable to visualize melee attack traces." );
#endif

bool CWeaponBaseMelee::DidMeleeAttackHit( MeleeAttackType attackType )
{
	// Check for hits with the 1st tracer set.
	// First we do line tracer, then hull tracer (if nothing was hit).
	WhatDidWeHit eWhatDidWeHit1 = DoAttackTrace( attackType, 0, false );
	if ( eWhatDidWeHit1 == HIT_NOTHING ) eWhatDidWeHit1 = DoAttackTrace( attackType, 0, true );

	// Now do 2nd tracer set, and do the same thing.
	WhatDidWeHit eWhatDidWeHit2 = DoAttackTrace( attackType, 1, false );
	if ( eWhatDidWeHit2 == HIT_NOTHING ) eWhatDidWeHit2 = DoAttackTrace( attackType, 1, true );

	// If we didn't hit anything with the 1st tracer, use the result from the 2nd tracer.
	if ( eWhatDidWeHit1 == HIT_NOTHING ) eWhatDidWeHit1 = eWhatDidWeHit2;

	// What did we hit?
	bool bHitSomething = false;

	// We managed to hit something
	if ( eWhatDidWeHit1 > HIT_NOTHING )
	{
		bool bHitWorld = ( eWhatDidWeHit1 == HIT_WORLD );
		DoWeaponSoundFromAttack( attackType, bHitWorld );
		m_pPlayer->m_iWeaponVolume = bHitWorld ? MELEE_SND_WALLHIT_VOLUME : MELEE_SND_BODYHIT_VOLUME;
		bHitSomething = true;
	}
	else
		DoWeaponSoundFromMiss( attackType );

	return bHitSomething;
}

CWeaponBaseMelee::WhatDidWeHit CWeaponBaseMelee::DoAttackTrace( MeleeAttackType attackType, int iTracerSet, bool bDoHullTrace )
{
	std::vector<edict_t *> hitEntities;
	MeleeAttackTrace *pMelee = &m_attackTracers[ attackType == MELEE_ATTACK_HEAVY ? 1 : 0 ];
	WhatDidWeHit eWhatDidWeHit = HIT_NOTHING;
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );

	Vector vPos, vForward, vRight, vUp;
	vPos = m_pPlayer->GetGunPosition();
	vForward = gpGlobals->v_forward;
	vRight = gpGlobals->v_right;
	vUp = gpGlobals->v_up;

	// Get the front, right, and up movement vectors.
	Vector vStartFRU = pMelee->vecStart[ iTracerSet ];
	Vector vEndFRU = pMelee->vecEnd[ iTracerSet ];

	// Calculate the FRU start and end vectors.
	Vector vStart = vPos
	    + vForward * vStartFRU.x
	    + vRight * vStartFRU.y
	    + vUp * vStartFRU.z;

	Vector vEnd = vPos
	    + vForward * vEndFRU.x
	    + vRight * vEndFRU.y
	    + vUp * vEndFRU.z;

	Vector vTraceTargetDir = vEnd - vStart;
	VectorNormalize( vTraceTargetDir );

	int nMeleeTraceCount = pMelee->iAmount;
	float flMeleeMaxTraceDist = pMelee->flRange;

	float flMeleeDaamge = GetMeleeAttackDamage( attackType );
	int bitsDamageType = GetMeleeDamageType( attackType );

	// Clear multi damage
	ClearMultiDamage();

	// Calculate the distance to move per trace.
	float flMeleeTraceDist = vStart.DistTo( vEnd ) / max( nMeleeTraceCount, 1 );
	for ( int n = 0; n <= nMeleeTraceCount; ++n )
	{
		// Move the trace target from the start towards the end position.
		Vector vTraceTarget = vStart + (vTraceTargetDir * (flMeleeTraceDist * n));

		// Get the direction from the eye to the trace target from start to end.
		Vector vTraceTargetDir = vTraceTarget - vPos;
		VectorNormalize( vTraceTargetDir );

		// Do hull trace, if the line trace didn't hit anything from the previous call.
		// But we only care about this on the server side.
#ifndef CLIENT_DLL
		if ( bDoHullTrace )
			UTIL_TraceHull( vPos, vPos + (vTraceTargetDir * flMeleeMaxTraceDist), dont_ignore_monsters, point_hull, ENT(m_pPlayer->pev), &m_trHit );
		else
#endif
			UTIL_TraceLine( vPos, vPos + (vTraceTargetDir * flMeleeMaxTraceDist), dont_ignore_monsters, ENT(m_pPlayer->pev), &m_trHit );

		if ( m_trHit.flFraction < 1.0 )
		{
#ifndef CLIENT_DLL
			// TODO: Create a proper user message to the player, that visualizes the melee traces.
			// As GoldSrc does not have debug overlays like Source.
			if ( sv_melee_debug_traces.GetBool() )
				DecalGunshot( &m_trHit, vForward, DMG_BULLET );
#endif

			if ( IsEntityAlreadyHit( m_trHit.pHit, hitEntities ) ) continue;
			CBaseEntity *pHitEntity = CBaseEntity::Instance( m_trHit.pHit );
			if ( pHitEntity )
			{
				if ( pHitEntity->Classify() != CLASS_NONE && pHitEntity->Classify() != CLASS_MACHINE )
					eWhatDidWeHit = HIT_ENTITY;
				else
				{
#ifndef CLIENT_DLL
					Vector vecEnd = vPos + (vTraceTargetDir * flMeleeMaxTraceDist);
					TEXTURETYPE_PlaySound( &m_trHit, vPos, vPos + (vecEnd - vPos) * 2, GetBulletType() );
#endif
					eWhatDidWeHit = HIT_WORLD;
				}
				pHitEntity->TraceAttack( m_pPlayer->pev, flMeleeDaamge, vTraceTargetDir, &m_trHit, bitsDamageType );
			}
			else
				eWhatDidWeHit = HIT_WORLD;

			DecalGunshot( &m_trHit, vForward, GetBulletType() );
			hitEntities.push_back( m_trHit.pHit );
		}
	}

	// Apply all the damage we traced this frame
	ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

	// Clear it after use
	hitEntities.clear();

	return eWhatDidWeHit;
}

void CWeaponBaseMelee::DoMeleeAttack()
{
	bool bHitSomething = DidMeleeAttackHit( n_meleeAttackType );
	int iAnim = bHitSomething ? ANIM_MELEE_HEAVY_HIT : ANIM_MELEE_HEAVY_MISS;
	if ( IsInLightAttack() )
	{
		switch ( RANDOM_LONG( 0, 2 ) )
		{
			case 0: iAnim = bHitSomething ? ANIM_MELEE_ATTACK1HIT : ANIM_MELEE_ATTACK1MISS; break;
			case 1: iAnim = bHitSomething ? ANIM_MELEE_ATTACK2HIT : ANIM_MELEE_ATTACK2MISS; break;
			case 2: iAnim = bHitSomething ? ANIM_MELEE_ATTACK3HIT : ANIM_MELEE_ATTACK3MISS; break;
		}
	}
	SendWeaponAnim( iAnim );

	// Set player animation
	m_pPlayer->SetAnimation( IsInHeavyAttack() ? PLAYER_ATTACK2_POST : PLAYER_ATTACK1 );

	float flDelay = GetMeleeAttackAnimationTime( n_meleeAttackType );
	if ( flDelay <= 0.0f ) flDelay = 0.1f;
	else flDelay /= 2.0f;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flDelay;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flDelay;

	n_meleeAttackType = MELEE_ATTACK_NONE;
}
