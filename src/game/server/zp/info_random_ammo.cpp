// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "zp/info_random_base.h"

class CRandomItemAmmo : public CRandomItemBase
{
	SET_BASECLASS( CRandomItemBase );
public:
	ItemType GetType() const { return ItemType::TypeAmmo; }
	bool IsLimited( SpawnList *item ) const;
};
LINK_ENTITY_TO_CLASS( info_random_ammo, CRandomItemAmmo );

bool CRandomItemAmmo::IsLimited( SpawnList *item ) const
{
	// Not the correct type, return true.
	if ( item->Type != GetType() ) return true;

	std::vector<std::string> entitiesToCheck;

	if ( FStrEq( item->Classname, "ammo_9mmclip" ) ) entitiesToCheck.push_back( "weapon_sig" );
	else if ( FStrEq( item->Classname, "ammo_mp5clip" ) ) entitiesToCheck.push_back( "weapon_mp5" );
	else if ( FStrEq( item->Classname, "ammo_556AR" ) ) entitiesToCheck.push_back( "weapon_556ar" );
	else if ( FStrEq( item->Classname, "ammo_buckshot" ) )
	{
		entitiesToCheck.push_back( "weapon_shotgun" );
		entitiesToCheck.push_back( "weapon_doublebarrel" );
	}
	else if ( FStrEq( item->Classname, "ammo_357" ) ) entitiesToCheck.push_back( "weapon_357" );
	else if ( FStrEq( item->Classname, "ammo_22lrbox" ) ) entitiesToCheck.push_back( "weapon_ppk" );

	// Make sure we have something to check
	if ( entitiesToCheck.size() <= 0 ) return true;

	// Check if any of the related weapons exist in the map
	for ( const std::string& szEntToCheck : entitiesToCheck )
	{
		CBaseEntity *pFind = UTIL_FindEntityByClassname( nullptr, szEntToCheck.c_str() );
		if ( pFind ) return BaseClass::IsLimited( item );
	}

	// Found nothing
	return true;
}
