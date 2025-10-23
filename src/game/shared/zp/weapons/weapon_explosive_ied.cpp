// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_explosive_ied.h"
#include "throwable_satchel.h"
#ifndef CLIENT_DLL
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_satchel, CWeaponExplosiveIED );
LINK_ENTITY_TO_CLASS( weapon_ied, CWeaponExplosiveIED );

//=========================================================
//=========================================================
int CWeaponExplosiveIED::AddToPlayer(CBasePlayer *pPlayer)
{
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup( pPlayer );
		return TRUE;
	}
	return FALSE;
}

void CWeaponExplosiveIED::Spawn()
{
	pev->classname = MAKE_STRING( "weapon_ied" );

	Precache();
	SET_MODEL(ENT(pev), "models/w_satchel.mdl");

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.

	m_bFirstThrow = false;
	m_bHasThrownSatchel = false;
	m_iPrevClip = m_iDefaultAmmo;
}

void CWeaponExplosiveIED::Precache(void)
{
	PRECACHE_MODEL("models/v_satchel.mdl");
	PRECACHE_MODEL("models/w_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");
	PRECACHE_SOUND("weapons/ied/button_press.wav");

	UTIL_PrecacheOther("monster_satchel");
}

float CWeaponExplosiveIED::Deploy()
{
	m_bHasDetonatedSatchel = false;
	DoDeploy(
		"models/v_satchel.mdl",
		m_bHasThrownSatchel ? "models/p_satchel_radio.mdl" : "models/p_satchel.mdl",
		m_bHasThrownSatchel ? ANIM_SATCHEL_DETONATOR_DRAW : ANIM_SATCHEL_DRAW,
		m_bHasThrownSatchel ? "hive" : "trip"
	);
	return GetAnimationTime( m_bHasThrownSatchel ? 18 : 22, 30 );
}

float CWeaponExplosiveIED::DoHolsterAnimation()
{
	bool bHasSatchel = m_bHasThrownSatchel;
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);

	SendWeaponAnim( bHasSatchel ? ANIM_SATCHEL_DETONATOR_HOLSTER : ANIM_SATCHEL_HOLSTER );

	if ( m_iClip <= 0 && !bHasSatchel )
	{
#ifndef CLIENT_DLL
		m_pPlayer->WeaponSlotSet( this, false );
#endif
		DestroyItem();
	}

	return GetAnimationTime( 9, 30 );
}

void CWeaponExplosiveIED::DeactivateThrow()
{
	m_bHasDetonatedSatchel = false;
}

void CWeaponExplosiveIED::PrimaryAttack( void )
{
	if ( BeginThrowOrPlant() ) return;
	if ( !m_bHasThrownSatchel ) return;
	if ( m_bFirstThrow ) return;

	AddWeaponSound( "weapons/ied/button_press.wav", 1, ATTN_NORM, GetAnimationTime( 4, 30 ) );
	SendWeaponAnim( ANIM_SATCHEL_DETONATOR_USE );

	edict_t *pPlayer = m_pPlayer->edict();

	CThrowableSatchelCharge *pSatchel = NULL;

	while ((pSatchel = (CThrowableSatchelCharge *)UTIL_FindEntityInSphere(pSatchel, m_pPlayer->pev->origin, 4096)) != NULL)
	{
		if (FClassnameIs(pSatchel->pev, "monster_satchel"))
		{
#ifndef CLIENT_DLL
			if (pSatchel->pev->owner == pPlayer || pSatchel->GetThrower() == ENTINDEX(pPlayer))
				pSatchel->IEDExplode();
#endif
		}
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( 14, 30 );
	m_bHasDetonatedSatchel = true;
	m_bHasThrownSatchel = false;
}

bool CWeaponExplosiveIED::HasSatchelCharge() const
{
	return m_bHasThrownSatchel;
}

bool CWeaponExplosiveIED::PlantIED( void )
{
	// We found a wall? Plant the IED. That's it.

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = gpGlobals->v_forward;
	TraceResult tr;
	UTIL_TraceLine( vecSrc, vecSrc + vecAiming * 128, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );

	if ( tr.flFraction < 1.0f )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		if ( pEntity && !(pEntity->pev->flags & FL_CONVEYOR) )
		{
#ifndef CLIENT_DLL
			Vector angles = UTIL_VecToAngles( tr.vecPlaneNormal );
			CThrowableSatchelCharge *pSatchel = (CThrowableSatchelCharge *)Create( "monster_satchel", tr.vecEndPos + tr.vecPlaneNormal * 8, angles, m_pPlayer->edict() );
			pSatchel->PlantIED();
			pSatchel->DisallowPickupFor( 2.0f );
			UTIL_strcpy( m_pPlayer->m_szAnimExtention, "hive" );
#endif
			SendWeaponAnim(ANIM_SATCHEL_DROP);

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);

			m_iClip = 0;
			m_iPrevClip = m_iClip;

			m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime(20, 30);

			m_bHasThrownSatchel = true;

			return true;
		}
	 }

	return false;
}

bool CWeaponExplosiveIED::BeginThrowOrPlant(void)
{
	if ( m_iClip > 0 )
	{
		if ( PlantIED() ) return true;

		if ( !m_bHasThrownSatchel )
			m_bFirstThrow = true;
		Vector vecSrc = m_pPlayer->pev->origin;
		Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;
		bool bHasSatchel = HasSatchelCharge();

#ifndef CLIENT_DLL
		CThrowableSatchelCharge *pSatchel = (CThrowableSatchelCharge *)Create( "monster_satchel", vecSrc, Vector(0, 0, 0), m_pPlayer->edict() );
		pSatchel->pev->velocity = vecThrow;
		pSatchel->pev->avelocity.y = 400;
		pSatchel->DisallowPickupFor( 2.0f );
		m_pPlayer->pev->weaponmodel = MAKE_STRING( "models/p_satchel_radio.mdl" );
		UTIL_strcpy( m_pPlayer->m_szAnimExtention, "hive" );
#endif
		SendWeaponAnim( bHasSatchel ? ANIM_SATCHEL_DETONATOR_DROP : ANIM_SATCHEL_DROP );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_iClip = 0;
		m_iPrevClip = m_iClip;

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( bHasSatchel ? 21 : 20, 30 );

		m_bHasThrownSatchel = true;
		return true;
	}
	return false;
}

void CWeaponExplosiveIED::WeaponIdle( void )
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() ) return;
	bool bHasSatchel = HasSatchelCharge();

	// If we have picked up more satchels,
	// reset the throw state.
	if ( m_iClip > m_iPrevClip )
	{
		m_iPrevClip = m_iClip;
		if ( m_bHasThrownSatchel )
		{
			m_bHasThrownSatchel = false;
			DoDeployAnimation();
		}
		return;
	}

	// Retire if we are out of ammo,
	// and have no satchels deployed.
	if ( !bHasSatchel && m_iClip <= 0 )
	{
		DoHolsterAnimation();
		return;
	}

	if ( m_bFirstThrow )
	{
		m_bFirstThrow = false;
		DoDeployAnimation();
		return;
	}

	if ( m_bHasDetonatedSatchel )
	{
		DoDeployAnimation();
		return;
	}

	int iAnimation;
	float flDelay;

	switch ( RANDOM_LONG( 0, 8 ) )
	{
		default:
			iAnimation = bHasSatchel ? ANIM_SATCHEL_DETONATOR_IDLE : ANIM_SATCHEL_IDLE;
			flDelay = GetAnimationTime( bHasSatchel ? 36 : 39, 30 );
		break;

		case 4:
		    iAnimation = bHasSatchel ? ANIM_SATCHEL_DETONATOR_FIDGET : ANIM_SATCHEL_FIDGET;
			flDelay = GetAnimationTime( bHasSatchel ? 36 : 50, 30 );
		break;
	}

	SendWeaponAnim( iAnimation );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flDelay;
}
