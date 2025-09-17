// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_RANDOM_ENTITY_BASE
#define SERVER_RANDOM_ENTITY_BASE
#pragma once

#include "cbase.h"

class CBasePlayer;

enum ItemType
{
	TypeNone = 0,
	TypeItem,
	TypeAmmo,
	TypeWeapon
};

struct SpawnList
{
	char Classname[32];
	int Limit = -1;
	bool Full = false;
	ItemType Type = ItemType::TypeNone;

	// Empty string
	SpawnList()
	{
		Classname[0] = 0;
	}

	// Creation
	SpawnList( const char *szClassname, int iLimit, ItemType nSpawnType )
	{
		UTIL_strcpy( Classname, szClassname );
		Limit = iLimit;
		Full = false;
		Type = nSpawnType;
	}
};

class CRandomItemBase : public CPointEntity
{
	SET_BASECLASS( CPointEntity );
public:
	void SpawnItem( void );
	string_t GetRandomClassname() const;
	virtual ItemType GetType() const = 0;
	virtual bool IsLimited( SpawnList *item ) const;
};

namespace ZP
{
	void OnGameModeRoundStart();
	void CheckHowManySpawnedItems( CBasePlayer *pPlayer );
	void SpawnItems();
    void SetupDefaultSpawnList();
	void IO_CalculatePlayerAmount( KeyValues *pData );
}

#endif