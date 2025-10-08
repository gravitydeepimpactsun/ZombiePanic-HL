// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "weapon_explosive_satchel.h"
#include "throwable_satchel.h"
#ifndef CLIENT_DLL
#include "gamerules.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_satchel, CWeaponExplosiveSatchel );

enum
{
	// Satchel states
	SATCHEL_IDLE = 0,
	SATCHEL_READY,
	SATCHEL_RELOAD,

	// It's animations
	SATCHEL_IDLE1 = 0,
	SATCHEL_FIDGET1,
	SATCHEL_DRAW,
	SATCHEL_DROP
};

enum
{
	SATCHEL_RADIO_IDLE1 = 0,
	SATCHEL_RADIO_FIDGET1,
	SATCHEL_RADIO_DRAW,
	SATCHEL_RADIO_FIRE,
	SATCHEL_RADIO_HOLSTER
};

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

		if (pSatchel->m_chargeReady != SATCHEL_IDLE && (nSatchelsInPocket + nNumSatchels) >= SATCHEL_MAX_CARRY)
		{
			// player has some satchels deployed. Refuse to add more.
			return FALSE;
		}
	}

	CBasePlayerWeapon *pWeapon = dynamic_cast<CBasePlayerWeapon *>( pOriginal );
	if ( pWeapon->m_iClip < pWeapon->iMaxClip() )
	{
		pWeapon->m_iClip += 1;
		EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
		return TRUE;
	}
	return FALSE;
}

//=========================================================
//=========================================================
int CWeaponExplosiveSatchel::AddToPlayer(CBasePlayer *pPlayer)
{
	m_chargeReady = SATCHEL_IDLE; // this satchel charge weapon now forgets that any satchels are deployed by it.
	if ( BaseClass::AddToPlayer( pPlayer ) )
	{
		BaseClass::SendWeaponPickup( pPlayer );
		return TRUE;
	}
	return FALSE;
}

void CWeaponExplosiveSatchel::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_satchel.mdl");

	WeaponData slot = GetWeaponSlotInfo( GetWeaponID() );
	m_iDefaultAmmo = slot.DefaultAmmo;

	FallInit(); // get ready to fall down.
}

void CWeaponExplosiveSatchel::Precache(void)
{
	PRECACHE_MODEL("models/v_satchel.mdl");
	PRECACHE_MODEL("models/v_satchel_radio.mdl");
	PRECACHE_MODEL("models/w_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");

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

	if (m_chargeReady != SATCHEL_IDLE)
	{
		// player isn't carrying any satchels, but has some out
		return TRUE;
	}

	return FALSE;
}

BOOL CWeaponExplosiveSatchel::Deploy()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);

	if (m_chargeReady)
		return DefaultDeploy("models/v_satchel_radio.mdl", "models/p_satchel_radio.mdl", SATCHEL_RADIO_DRAW, "hive");
	else
		return DefaultDeploy("models/v_satchel.mdl", "models/p_satchel.mdl", SATCHEL_DRAW, "trip");

	return TRUE;
}

void CWeaponExplosiveSatchel::Holster(int skiplocal /* = 0 */)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);

	if (m_chargeReady)
	{
		SendWeaponAnim(SATCHEL_RADIO_HOLSTER);
	}
	else
	{
		SendWeaponAnim(SATCHEL_DROP);
	}

	if (m_iClip <= 0 && m_chargeReady != SATCHEL_READY)
	{
#ifndef CLIENT_DLL
		m_pPlayer->WeaponSlotSet( this, false );
#endif
		DestroyItem();
	}
}

void CWeaponExplosiveSatchel::PrimaryAttack(void)
{
	if (m_chargeReady != SATCHEL_RELOAD)
	{
		Throw();
	}
}

void CWeaponExplosiveSatchel::SecondaryAttack()
{
	switch (m_chargeReady)
	{
	case SATCHEL_IDLE:
	{
		Throw();
	}
	break;
	case SATCHEL_READY:
	{
		SendWeaponAnim(SATCHEL_RADIO_FIRE);

		edict_t *pPlayer = m_pPlayer->edict();

		CThrowableSatchelCharge *pSatchel = NULL;

		while ((pSatchel = (CThrowableSatchelCharge *)UTIL_FindEntityInSphere(pSatchel, m_pPlayer->pev->origin, 4096)) != NULL)
		{
			if (FClassnameIs(pSatchel->pev, "monster_satchel"))
			{
				if (pSatchel->pev->owner == pPlayer || pSatchel->GetThrower() == ENTINDEX(pPlayer))
				{
					pSatchel->Use(m_pPlayer, m_pPlayer, USE_SET, 0);
				}
			}
		}

		m_chargeReady = SATCHEL_RELOAD;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + SecondaryFireRate();
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + SecondaryFireRate();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		break;
	}

	case SATCHEL_RELOAD:
		// we're reloading, don't allow fire
		{
		}
		break;
	}
}

void CWeaponExplosiveSatchel::Throw(void)
{
	if (m_iClip > 0)
	{
		Vector vecSrc = m_pPlayer->pev->origin;

		Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CThrowableSatchelCharge *pSatchel = (CThrowableSatchelCharge *)Create("monster_satchel", vecSrc, Vector(0, 0, 0), m_pPlayer->edict());
		pSatchel->pev->velocity = vecThrow;
		pSatchel->pev->avelocity.y = 400;
		pSatchel->DisallowPickupFor( 2.5f );

		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel_radio.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel_radio.mdl");
#else
		LoadVModel("models/v_satchel_radio.mdl", m_pPlayer);
#endif

		SendWeaponAnim(SATCHEL_RADIO_DRAW);

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		m_chargeReady = SATCHEL_READY;

		m_iClip--;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + PrimaryFireRate();
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + SecondaryFireRate();
	}
}

void CWeaponExplosiveSatchel::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	switch (m_chargeReady)
	{
	case SATCHEL_IDLE:
		SendWeaponAnim(SATCHEL_FIDGET1);
		// use tripmine animations
		UTIL_strcpy(m_pPlayer->m_szAnimExtention, "trip");
		break;
	case SATCHEL_READY:
		SendWeaponAnim(SATCHEL_RADIO_FIDGET1);
		// use hivehand animations
		UTIL_strcpy(m_pPlayer->m_szAnimExtention, "hive");
		break;
	case SATCHEL_RELOAD:
		if (m_iClip <= 0)
		{
			RetireWeapon();
			m_chargeReady = SATCHEL_IDLE;
			return;
		}

#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_satchel.mdl");
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_satchel.mdl");
#else
		LoadVModel("models/v_satchel.mdl", m_pPlayer);
#endif

		SendWeaponAnim(SATCHEL_DRAW);

		// use tripmine animations
		UTIL_strcpy(m_pPlayer->m_szAnimExtention, "trip");

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_chargeReady = SATCHEL_IDLE;
		break;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
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