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

	const char *szEntToCheck = nullptr;
	if ( FStrEq( item->Classname, "ammo_9mmclip" ) ) szEntToCheck = "weapon_sig";
	else if ( FStrEq( item->Classname, "ammo_mp5clip" ) ) szEntToCheck = "weapon_mp5";
	else if ( FStrEq( item->Classname, "ammo_556AR" ) ) szEntToCheck = "weapon_556ar";
	else if ( FStrEq( item->Classname, "ammo_buckshot" ) ) szEntToCheck = "weapon_shotgun";
	else if ( FStrEq( item->Classname, "ammo_357" ) ) szEntToCheck = "weapon_357";
	else if ( FStrEq( item->Classname, "ammo_22lrbox" ) ) szEntToCheck = "weapon_ppk";

	if ( !szEntToCheck ) return true;
	// We only need to find one instance of this classname
	CBaseEntity *pFind = UTIL_FindEntityByClassname( nullptr, szEntToCheck );
	return pFind ? BaseClass::IsLimited( item ) : true;
}
