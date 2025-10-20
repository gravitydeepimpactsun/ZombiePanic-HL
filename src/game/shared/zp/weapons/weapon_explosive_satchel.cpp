// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_explosive_satchel.h"
#include "throwable_satchel.h"
#ifndef CLIENT_DLL
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_satchel, CWeaponExplosiveSatchel );
LINK_ENTITY_TO_CLASS( weapon_ied, CWeaponExplosiveSatchel );

//=========================================================
// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
//=========================================================
int CWeaponExplosiveSatchel::AddDuplicate(CBasePlayerItem *pOriginal)
{
	CWeaponExplosiveSatchel *pSatchel;

#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		pSatchel = (CWeaponExplosiveSatchel *)pOriginal;

		if (!pOriginal->m_pPlayer)
			return TRUE;

		int nSatchelsInPocket = pSatchel->m_iClip;
		int nNumSatchels = 0;
		CThrowableSatchelCharge *pLiveSatchel = NULL;

		while ((pLiveSatchel = (CThrowableSatchelCharge *)UTIL_FindEntityInSphere(pLiveSatchel, pOriginal->m_pPlayer->pev->origin, 4096)) != NULL)
		{
			if (FClassnameIs(pLiveSatchel->pev, "monster_satchel"))
			{
				if (pLiveSatchel->pev->owner == pOriginal->m_pPlayer->edict() || pLiveSatchel->GetThrower() == pOriginal->m_pPlayer->entindex())
				{
					nNumSatchels++;
				}
			}
		}

		if ( pSatchel->HasSatchelCharge() && (nSatchelsInPocket + nNumSatchels) >= pSatchel->iMaxClip() )
		{
			// player has some satchels deployed. Refuse to add more.
			return FALSE;
		}
	}

	CBasePlayerWeapon *pWeapon = dynamic_cast<CBasePlayerWeapon *>( pOriginal );
	if ( pWeapon->m_iClip < pWeapon->iMaxClip() )
	{
		pWeapon->m_iClip += 1;
		EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/ammo_pickup.wav", 1, ATTN_NORM );
		return TRUE;
	}
	return FALSE;
}

//=========================================================
//=========================================================
int CWeaponExplosiveSatchel::AddToPlayer(CBasePlayer *pPlayer)
{
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup( pPlayer );
		return TRUE;
	}
	return FALSE;
}

void CWeaponExplosiveSatchel::Spawn()
{
	pev->classname = MAKE_STRING( "weapon_satchel" );

	Precache();
	SET_MODEL(ENT(pev), "models/w_satchel.mdl");

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.

	m_bFirstThrow = false;
	m_bHasThrownSatchel = false;
}

void CWeaponExplosiveSatchel::Precache(void)
{
	PRECACHE_MODEL("models/v_satchel.mdl");
	PRECACHE_MODEL("models/w_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");
	PRECACHE_SOUND("weapons/ied/button_press.wav");

	UTIL_PrecacheOther("monster_satchel");
}

//=========================================================
//=========================================================
BOOL CWeaponExplosiveSatchel::IsUseable(void)
{
	return CanDeploy();
}

BOOL CWeaponExplosiveSatchel::CanDeploy(void)
{
	if (m_iClip > 0)
	{
		// player is carrying some satchels
		return TRUE;
	}

	if ( HasSatchelCharge() )
	{
		// player isn't carrying any satchels, but has some out
		return TRUE;
	}

	return FALSE;
}

float CWeaponExplosiveSatchel::Deploy()
{
	m_bHasDetonatedSatchel = false;
	DoDeploy(
		"models/v_satchel.mdl",
		HasSatchelCharge() ? "models/p_satchel_radio.mdl" : "models/p_satchel.mdl",
		HasSatchelCharge() ? ANIM_SATCHEL_DETONATOR_DRAW : ANIM_SATCHEL_DRAW,
		HasSatchelCharge() ? "hive" : "trip"
	);
	return GetAnimationTime( HasSatchelCharge() ? 18 : 22, 30 );
}

float CWeaponExplosiveSatchel::DoHolsterAnimation()
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);

	SendWeaponAnim( HasSatchelCharge() ? ANIM_SATCHEL_DETONATOR_HOLSTER : ANIM_SATCHEL_HOLSTER );

	if ( m_iClip <= 0 && !HasSatchelCharge() )
	{
#ifndef CLIENT_DLL
		m_pPlayer->WeaponSlotSet( this, false );
#endif
		DestroyItem();
	}

	return GetAnimationTime( 9, 30 );
}

void CWeaponExplosiveSatchel::DeactivateThrow()
{
	m_bHasDetonatedSatchel = false;
	m_bHasThrownSatchel = false;
}

void CWeaponExplosiveSatchel::PrimaryAttack(void)
{
	Throw();
}

void CWeaponExplosiveSatchel::SecondaryAttack()
{
	if ( !HasSatchelCharge() ) return;
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

void CWeaponExplosiveSatchel::Throw(void)
{
	if ( m_iClip > 0 )
	{
		if ( !m_bHasThrownSatchel )
			m_bFirstThrow = true;
		Vector vecSrc = m_pPlayer->pev->origin;
		Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CThrowableSatchelCharge *pSatchel = (CThrowableSatchelCharge *)Create( "monster_satchel", vecSrc, Vector(0, 0, 0), m_pPlayer->edict() );
		pSatchel->pev->velocity = vecThrow;
		pSatchel->pev->avelocity.y = 400;
		pSatchel->DisallowPickupFor( 2.0f );
		m_pPlayer->pev->weaponmodel = MAKE_STRING( "models/p_satchel_radio.mdl" );
		UTIL_strcpy( m_pPlayer->m_szAnimExtention, "hive" );
#endif

		SendWeaponAnim( HasSatchelCharge() ? ANIM_SATCHEL_DETONATOR_DROP : ANIM_SATCHEL_DROP );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_iClip--;

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetAnimationTime( HasSatchelCharge() ? 21 : 20, 30 );

		m_bHasThrownSatchel = true;
	}
}

void CWeaponExplosiveSatchel::WeaponIdle( void )
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// Retire if we are out of ammo,
	// and have no satchels deployed.
	if ( !HasSatchelCharge() && m_iClip <= 0 )
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
			iAnimation = HasSatchelCharge() ? ANIM_SATCHEL_DETONATOR_IDLE : ANIM_SATCHEL_IDLE;
			flDelay = GetAnimationTime( HasSatchelCharge() ? 36 : 39, 30 );
		break;

		case 4:
		    iAnimation = HasSatchelCharge() ? ANIM_SATCHEL_DETONATOR_FIDGET : ANIM_SATCHEL_FIDGET;
			flDelay = GetAnimationTime( HasSatchelCharge() ? 36 : 50, 30 );
		break;
	}

	SendWeaponAnim( iAnimation );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flDelay;
}

//=========================================================
// DeactivateSatchels - removes all satchels owned by
// the provided player. Should only be used upon death.
//
// Made this global on purpose.
//=========================================================
void DeactivateSatchels( CBasePlayer *pOwner )
{
	edict_t *pFind;

	pFind = FIND_ENTITY_BY_CLASSNAME(NULL, "monster_satchel");

	while (!FNullEnt(pFind))
	{
		CBaseEntity *pEnt = CBaseEntity::Instance(pFind);
		CThrowableSatchelCharge *pSatchel = (CThrowableSatchelCharge *)pEnt;

		if (pSatchel)
		{
			if (pSatchel->pev->owner == pOwner->edict() || pSatchel->GetThrower() == pOwner->entindex())
			{
				pSatchel->Deactivate();
			}
		}

		pFind = FIND_ENTITY_BY_CLASSNAME(pFind, "monster_satchel");
	}
}