/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"
#include "zp/zp_shared.h"

class CHealthItemBase : public CItem
{
	SET_BASECLASS( CItem );
public:
	void Spawn();
	void Precache();
	BOOL MyTouch( CBasePlayer *pPlayer );
	enum HealthItemType
	{
		HEALTHKIT_LARGE,
		HEALTHKIT_MEDIUM,
		HEALTHKIT_SMALL,
		BANDAGE,
		PAINKILLER
	};
	virtual HealthItemType GetHealthItemType() const { return HEALTHKIT_LARGE; }
	bool OnHealthKitUsed( CBasePlayer *pPlayer );
};

void CHealthItemBase::Spawn()
{
	Precache();
	switch ( GetHealthItemType() )
	{
		case HEALTHKIT_LARGE: SET_MODEL( ENT( pev ), "models/w_medkit.mdl" ); break;
		case HEALTHKIT_MEDIUM: SET_MODEL( ENT( pev ), "models/w_medkit_medium.mdl" ); break;
		case HEALTHKIT_SMALL: SET_MODEL( ENT( pev ), "models/w_medkit_small.mdl" ); break;
		case BANDAGE: SET_MODEL( ENT( pev ), "models/w_bandage.mdl" ); break;
	    case PAINKILLER: SET_MODEL( ENT( pev ), "models/w_painkiller.mdl" ); break;
		default: SET_MODEL( ENT( pev ), "models/w_medkit.mdl" ); break;
	}
	BaseClass::Spawn();
}

void CHealthItemBase::Precache()
{
	// Precache all the models
	PRECACHE_MODEL( "models/w_medkit.mdl" );
	PRECACHE_MODEL( "models/w_medkit_medium.mdl" );
	PRECACHE_MODEL( "models/w_medkit_small.mdl" );
	PRECACHE_MODEL( "models/w_bandage.mdl" );
	PRECACHE_MODEL( "models/w_painkiller.mdl" );

	// Precache all the sounds
	PRECACHE_SOUND( "items/medkit_use.wav" );
	PRECACHE_SOUND( "items/bandage_use.wav" );
	PRECACHE_SOUND( "items/pills_use.wav" );
}

BOOL CHealthItemBase::MyTouch( CBasePlayer *pPlayer )
{
	if ( pPlayer->pev->deadflag != DEAD_NO ) return FALSE;
	if ( pPlayer->pev->team == ZP::TEAM_ZOMBIE ) return FALSE;

	// Handle based on type
	switch( GetHealthItemType() )
	{
		case HEALTHKIT_LARGE:
		case HEALTHKIT_MEDIUM:
		case HEALTHKIT_SMALL:
		{
			if ( OnHealthKitUsed( pPlayer ) )
				return TRUE;
		}
		break;
		case BANDAGE:
		{
			if ( pPlayer->GotBandage( true ) )
			{
				BaseClass::SendItemPickup( pPlayer );
				EMIT_SOUND( ENT( pPlayer->pev ), CHAN_ITEM, "items/bandage_use.wav", 1, ATTN_NORM );
				SoftRemove();
				return TRUE;
			}
		}
	    break;
		case PAINKILLER:
		{
			if ( pPlayer->GotPainKiller() )
			{
				BaseClass::SendItemPickup( pPlayer );
				EMIT_SOUND( ENT( pPlayer->pev ), CHAN_ITEM, "items/pills_use.wav", 1, ATTN_NORM );
				SoftRemove();
				return TRUE;
			}
	    }
	    break;
	}

	return FALSE;
}

bool CHealthItemBase::OnHealthKitUsed( CBasePlayer *pPlayer )
{
	// Health to give
	int healthToGive = 0;
	switch ( GetHealthItemType() )
	{
		case HEALTHKIT_LARGE: healthToGive = gSkillData.healthkitCapacity; break;
		case HEALTHKIT_MEDIUM: healthToGive = gSkillData.healthkitMediumCapacity; break;
		case HEALTHKIT_SMALL: healthToGive = gSkillData.healthkitSmallCapacity; break;
	}
	if ( healthToGive <= 0 )
		return false;

	if ( pPlayer->TakeHealth( healthToGive, DMG_GENERIC ) )
	{
		BaseClass::SendItemPickup( pPlayer );
		EMIT_SOUND( ENT( pPlayer->pev ), CHAN_ITEM, "items/medkit_use.wav", 1, ATTN_NORM );
		pPlayer->GotBandage( false );
		SoftRemove();
		return true;
	}

	return false;
}

#define CREATE_HEALTHKIT_CLASS( className, entityName, healthItemType ) \
class className : public CHealthItemBase \
{ \
public: \
	HealthItemType GetHealthItemType() const override { return healthItemType; } \
}; \
LINK_ENTITY_TO_CLASS( entityName, className ); \
PRECACHE_REGISTER( entityName )

CREATE_HEALTHKIT_CLASS( CHealthKitLarge, item_healthkit, CHealthItemBase::HEALTHKIT_LARGE )
CREATE_HEALTHKIT_CLASS( CHealthKitMedium, item_healthkit_medium, CHealthItemBase::HEALTHKIT_MEDIUM )
CREATE_HEALTHKIT_CLASS( CHealthKitSmall, item_healthkit_small, CHealthItemBase::HEALTHKIT_SMALL )
CREATE_HEALTHKIT_CLASS( CBandageItem, item_bandage, CHealthItemBase::BANDAGE )
CREATE_HEALTHKIT_CLASS( CPainKiller, item_painkiller, CHealthItemBase::PAINKILLER )

// Not used in ZP
#if 0
//-------------------------------------------------------------
// Wall mounted health kit
//-------------------------------------------------------------
class CWallHealth : public CBaseToggle
{
public:
	void Spawn();
	void Restart();
	void Precache(void);
	void EXPORT Off(void);
	void EXPORT Recharge(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int ObjectCaps(void) { return (CBaseToggle ::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);

	static TYPEDESCRIPTION m_SaveData[];

	float m_flNextCharge;
	int m_iReactivate; // DeathMatch Delay until reactvated
	int m_iJuice;
	int m_iOn; // 0 = off, 1 = startup, 2 = going
	float m_flSoundTime;
};

TYPEDESCRIPTION CWallHealth::m_SaveData[] = {
	DEFINE_FIELD(CWallHealth, m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD(CWallHealth, m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD(CWallHealth, m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD(CWallHealth, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD(CWallHealth, m_flSoundTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CWallHealth, CBaseEntity);

LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);

void CWallHealth::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style") || FStrEq(pkvd->szKeyName, "height") || FStrEq(pkvd->szKeyName, "value1") || FStrEq(pkvd->szKeyName, "value2") || FStrEq(pkvd->szKeyName, "value3"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CWallHealth::Spawn()
{
	Precache();

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin); // set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model));
	m_iJuice = gSkillData.healthchargerCapacity;
	pev->frame = 0;
}

void CWallHealth::Restart()
{
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	// set size and link into world
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, pev->mins, pev->maxs);

	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->nextthink = pev->ltime + 0.1f;
	SetThink(&CWallHealth::Recharge);
}

void CWallHealth::Precache()
{
	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
	PRECACHE_SOUND("items/medcharge4.wav");
}

void CWallHealth::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Make sure that we have a caller
	if (!pActivator)
		return;
	// if it's not a player, ignore
	if (!pActivator->IsPlayer())
		return;

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		pev->frame = 1;
		Off();
	}

	CBasePlayer *pPlayer = dynamic_cast< CBasePlayer* >( pActivator );
	if ( !pPlayer ) return;

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ( (m_iJuice <= 0) || pPlayer->GetWeaponOwn(WEAPON_SUIT) )
	{
		if (m_flSoundTime <= gpGlobals->time)
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshotno1.wav", 1.0, ATTN_NORM);
		}
		return;
	}

	pev->nextthink = pev->ltime + 0.25;
	SetThink(&CWallHealth::Off);

	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->time)
		return;

	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM);
		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->time))
	{
		m_iOn++;
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "items/medcharge4.wav", 1.0, ATTN_NORM);
	}

	// charge the player
	if (pActivator->TakeHealth(1, DMG_GENERIC))
	{
		m_iJuice--;
	}

	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1;
}

void CWallHealth::Recharge(void)
{
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM);
	m_iJuice = gSkillData.healthchargerCapacity;
	pev->frame = 0;
	SetThink(&CWallHealth::SUB_DoNothing);
}

void CWallHealth::Off(void)
{
	// Stop looping sound.
	if (m_iOn > 1)
		STOP_SOUND(ENT(pev), CHAN_STATIC, "items/medcharge4.wav");

	m_iOn = 0;

	if ((!m_iJuice) && ((m_iReactivate = g_pGameRules->FlHealthChargerRechargeTime()) > 0))
	{
		pev->nextthink = pev->ltime + m_iReactivate;
		SetThink(&CWallHealth::Recharge);
	}
	else
		SetThink(&CWallHealth::SUB_DoNothing);
}
#endif