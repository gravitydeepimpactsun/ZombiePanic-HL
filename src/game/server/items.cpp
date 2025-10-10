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
/*

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"
#include "zp/zp_shared.h"

extern int gmsgItemPickup;

class CWorldItem : public CBaseEntity
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Spawn(void);
	int m_iType;
};

LINK_ENTITY_TO_CLASS(world_items, CWorldItem);

void CWorldItem::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "type"))
	{
		m_iType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CWorldItem::Spawn(void)
{
	CBaseEntity *pEntity = NULL;

	switch (m_iType)
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create("item_battery", pev->origin, pev->angles);
		break;
	case 42: // ITEM_ANTIDOTE:
		pEntity = CBaseEntity::Create("item_antidote", pev->origin, pev->angles);
		break;
	case 43: // ITEM_SECURITY:
		pEntity = CBaseEntity::Create("item_security", pev->origin, pev->angles);
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create("item_suit", pev->origin, pev->angles);
		break;
	}

	if (!pEntity)
	{
		ALERT(at_console, "unable to create world_item %d\n", m_iType);
	}
	else
	{
		pEntity->pev->target = pev->target;
		pEntity->pev->targetname = pev->targetname;
		pEntity->pev->spawnflags = pev->spawnflags;
	}

	REMOVE_ENTITY(edict());
}

void CItem::Spawn(void)
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItem::ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove(this);
		return;
	}
}

extern int gEvilImpulse101;

void CItem::ItemTouch(CBaseEntity *pOther)
{
#if ALLOW_AUTO_PICKUP
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if ( MyTouch( pPlayer ) )
	{
		SUB_UseTargets(pOther, USE_TOGGLE, 0);
		SetTouch(NULL);

		// player grabbed the item.
		g_pGameRules->PlayerGotItem(pPlayer, this);
		SoftRemove();
	}
#endif
}

#if !defined(CLIENT_DLL)
void CItem::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Invisible? Ignore.
	if ( pev->effects == EF_NODRAW ) return;
	// Make sure we have a player
	CBasePlayer *pPlayer = (CBasePlayer *)pActivator;
	if ( !pPlayer || !pPlayer->IsPlayer() ) return;
	if ( pPlayer->pev->team != ZP::TEAM_SURVIVIOR ) return;

	// Give the ammo to the player
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch(pPlayer))
	{
		SUB_UseTargets( pPlayer, USE_TOGGLE, 0 );

		// player grabbed the item.
		g_pGameRules->PlayerGotItem( pPlayer, this );

		if ( SpawnedTroughRandomEntity() )
		{
			SetThink(&CBasePlayerAmmo::SUB_Remove);
			pev->nextthink = gpGlobals->time + .1;
		}
		else
			SoftRemove();
	}
}
#endif

CBaseEntity *CItem::Respawn(void)
{
	SetTouch(NULL);
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin(pev, g_pGameRules->VecItemRespawnSpot(this)); // blip to whereever you should respawn.

	SetThink(&CItem::Materialize);
	pev->nextthink = g_pGameRules->FlItemRespawnTime(this);
	return this;
}

void CItem::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CItem::ItemTouch);
	SetThink(NULL);
}

void CItem::SendItemPickup(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
	WRITE_STRING(STRING(pev->classname));
	MESSAGE_END();

	if (!pPlayer->m_fInitHUD)
	{
		// Send to all spectating players
		CBasePlayer *plr;
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			plr = (CBasePlayer *)UTIL_PlayerByIndex(i);
			if (!plr || plr->pev->iuser1 != OBS_IN_EYE || plr->m_hObserverTarget != pPlayer)
				continue;

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, plr->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();
		}
	}
}

#define SF_SUIT_SHORTLOGON 0x0001

class CItemSuit : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_battery.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_battery.mdl");
	}
	BOOL MyTouch(CBasePlayer *pPlayer)
	{
		// If we write "give item_suit", let's quickly remove it
		// if the player already have said "weapon"
		if (pPlayer->pev->weapons & (1 << WEAPON_SUIT))
		{
			SUB_Remove();
			return TRUE;
		}

		if (pev->spawnflags & SF_SUIT_SHORTLOGON)
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0"); // short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx"); // long version of suit logon

		pPlayer->pev->weapons |= (1 << WEAPON_SUIT);

		// item_suit gets removed, we don't want to keep it for round restarts
		SUB_Remove();
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);

class CItemBattery : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_battery.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_battery.mdl");
		PRECACHE_SOUND("items/armor_pickup01.wav");
		PRECACHE_SOUND("items/armor_pickup02.wav");
	}
	BOOL MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return FALSE;
		}

		if ( pPlayer->pev->team == ZP::TEAM_ZOMBIE )
			return false;

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) && (pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			int pct;
			char szcharge[64];

			pPlayer->pev->armorvalue += gSkillData.batteryCapacity;
			pPlayer->pev->armorvalue = min(pPlayer->pev->armorvalue, (float)MAX_NORMAL_BATTERY);

			if (RANDOM_LONG(0, 1))
				EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/armor_pickup01.wav", 1, ATTN_NORM);
			else
				EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/armor_pickup02.wav", 1, ATTN_NORM);

			CItem::SendItemPickup(pPlayer);

			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)((float)(pPlayer->pev->armorvalue * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;

			sprintf(szcharge, "!HEV_%1dP", pct);

			//EMIT_SOUND_SUIT(ENT(pev), szcharge);
			pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);

class CItemBackpack : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_weaponbox.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_weaponbox.mdl");
		PRECACHE_SOUND("items/backpack_pickup.wav");
	}
	BOOL MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer->pev->deadflag != DEAD_NO) return false;
		if ( pPlayer->pev->team == ZP::TEAM_ZOMBIE ) return false;
		if (!pPlayer->HasBackpack())
		{
			pPlayer->SetBackpackState( true );
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/backpack_pickup.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(item_backpack, CItemBackpack);

class CItemSecurity : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_security.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_security.mdl");
	}
	BOOL MyTouch(CBasePlayer *pPlayer)
	{
		if ( pPlayer->pev->team == ZP::TEAM_ZOMBIE )
			return false;
		pPlayer->m_rgItems[ITEM_SECURITY] += 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);
