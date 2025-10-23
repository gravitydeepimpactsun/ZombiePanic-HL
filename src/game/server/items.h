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
#ifndef ITEMS_H
#define ITEMS_H

class CItem : public CBaseEntity
{
public:
	void EXPORT Spawn(void);
	CBaseEntity *Respawn(void);
	void EXPORT ItemTouch(CBaseEntity *pOther);
	void EXPORT Materialize(void);
	virtual BOOL MyTouch(CBasePlayer *pPlayer) { return FALSE; };
	virtual int ObjectCaps(void) { return CBaseEntity::ObjectCaps() | FCAP_MUST_RELEASE | FCAP_IMPULSE_USE; }
	bool IsUseableItem() const override { return true; }
#if !defined( CLIENT_DLL )
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
#endif

protected:
	void SendItemPickup(CBasePlayer *pPlayer);
};

#endif // ITEMS_H
