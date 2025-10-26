// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "zp/weapons/CWeaponBase.h"

// Ammo box base
class CBaseAmmoItem : public CBasePlayerAmmo
{
	SET_BASECLASS( CBasePlayerAmmo );
	virtual char *GetModel() const { return "models/w_9mmbox.mdl"; }
	virtual const char *GetPickupSound() const { return "items/ammo_pickup.wav"; }
	virtual ZPAmmoTypes GetAmmoType() const { return ZPAmmoTypes::AMMO_NONE; }

public:
	virtual void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT(pev), GetModel() );
		BaseClass::Spawn();
		AmmoData data = GetAmmoByAmmoID( GetAmmoType() );
		m_iAmountLeft = m_iAmmoToGive = data.AmmoBoxGive;
		m_AmmoType = GetAmmoType();
		strncpy( m_szSound, "items/ammo_pickup.wav", 32 );
	}
	void Precache( void )
	{
		PRECACHE_MODEL( GetModel() );
		PRECACHE_SOUND( "items/ammo_pickup.wav" );
	}
};

// A very handy but simple macro to register our ammo boxes
#define REGISTER_AMMO_BOX( className, modelPath, ammoType ) \
class CAmmoItem_##className : public CBaseAmmoItem { \
	SET_BASECLASS( CBaseAmmoItem ); \
	char *GetModel() const override { return modelPath; } \
	ZPAmmoTypes GetAmmoType() const override { return ammoType; } \
	void Spawn( void ) override \
	{ \
		pev->classname = MAKE_STRING( #className ); \
		BaseClass::Spawn(); \
	} \
}; \
LINK_ENTITY_TO_CLASS( className, CAmmoItem_##className ); \
PRECACHE_REGISTER( className )

// A macro to register old ammo box classnames for backward compatibility.
// So we don't break old maps, because that would be bad.
#define REGISTER_BACKWARD_COMPATIBLE_AMMO_BOX( old_class, new_class ) \
LINK_ENTITY_TO_CLASS( old_class, CAmmoItem_##new_class )

// Now we register our ammo boxes

// Pistol ammo box
REGISTER_AMMO_BOX( ammo_9mmbox, "models/w_9mmbox.mdl", ZPAmmoTypes::AMMO_PISTOL );
REGISTER_BACKWARD_COMPATIBLE_AMMO_BOX( ammo_9mmclip, ammo_9mmbox );
REGISTER_BACKWARD_COMPATIBLE_AMMO_BOX( ammo_mp5clip, ammo_9mmbox );
REGISTER_BACKWARD_COMPATIBLE_AMMO_BOX( ammo_9mmAR, ammo_9mmbox );

// Magnum ammo box
REGISTER_AMMO_BOX( ammo_357box, "models/w_357box.mdl", ZPAmmoTypes::AMMO_MAGNUM );
REGISTER_BACKWARD_COMPATIBLE_AMMO_BOX( ammo_357, ammo_357box );

// Shotgun ammo box
REGISTER_AMMO_BOX( ammo_buckshot, "models/w_shotbox.mdl", ZPAmmoTypes::AMMO_SHOTGUN );

// Rifle ammo box
REGISTER_AMMO_BOX( ammo_riflebox, "models/w_riflebbox.mdl", ZPAmmoTypes::AMMO_RIFLE );
REGISTER_BACKWARD_COMPATIBLE_AMMO_BOX( ammo_556AR, ammo_riflebox );

// Long rifle ammo box
REGISTER_AMMO_BOX( ammo_22lrbox, "models/w_22lrbox.mdl", ZPAmmoTypes::AMMO_LONGRIFLE );
