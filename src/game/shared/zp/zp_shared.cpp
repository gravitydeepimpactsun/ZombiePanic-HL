// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#if defined(CLIENT_DLL)
#include "strtools.h"
#else
#include "decals.h"
#include "func_break.h"
#endif

#include <tier2/tier2.h>
#include "FileSystem.h"
#include <KeyValues.h>

#include "weapons.h"
#include "player.h"

#include "zp_shared.h"

#define WEAPON_SCRIPT_PATH "scripts/"
#define WEAPON_SCRIPT_FILE ".txt"
static std::vector<WeaponData> sWeaponDataList;
static AmmoData sAmmoDataList[] = {
	/* { ZPAmmoTypes AmmoType, const char *AmmoName, int MaxCarry, float WeightPerBullet } */
	{ AMMO_PISTOL, "9mm", 150, 0.21f },
	{ AMMO_MAGNUM, "357", 30, 0.65f },
	{ AMMO_SHOTGUN, "buckshot", 24, 1.35f },
	{ AMMO_RIFLE, "556ar", 150, 0.35f },

	{ AMMO_GRENADE, "Hand Grenade", 3, 0.1f },
	{ AMMO_SATCHEL, "Satchel Charge", 5, 0.5f },

	// MUST BE LAST, DO NOT CHANGE THIS.
	// This is used by Crowbar and Swipe (or any other weapon that has no ammo)
	{ AMMO_NONE, "", -1, 0.0f }
};

static WeaponInfo sWeaponInfoList[] = {
	// { const char *szWeapon, ZPWeaponID WeaponID, bool Hidden }
	{ "crowbar", WEAPON_CROWBAR, false },
	{ "swipe", WEAPON_SWIPE, true },
	{ "sig", WEAPON_SIG, false },
	{ "357", WEAPON_PYTHON, false },
	{ "mp5", WEAPON_MP5, false },
	{ "556ar", WEAPON_556AR, false },
	{ "shotgun", WEAPON_SHOTGUN, false },
	{ "handgrenade", WEAPON_HANDGRENADE, false },
	{ "satchel", WEAPON_SATCHEL, false },
};

WeaponData CreateWeaponSlotData( const char *szClassname )
{
	if ( !szClassname ) return WeaponData();

	std::string szClassnameStr( szClassname );
	// Make sure our classname is starting with "weapon_"
	// if not, add it.
	if ( szClassnameStr.find( "weapon_" ) != 0 )
		szClassnameStr = "weapon_" + szClassnameStr;

	std::string szFile( WEAPON_SCRIPT_PATH + szClassnameStr + WEAPON_SCRIPT_FILE );
	KeyValues *pWeaponScript = new KeyValues( "WeaponInfo" );
	if ( !pWeaponScript->LoadFromFile( g_pFullFileSystem, szFile.c_str() ) )
	{
		pWeaponScript->deleteThis();
		return WeaponData();
	}

	WeaponData slot;
	slot.WeaponID = GetWeaponInfo( szClassnameStr.c_str() ).WeaponID;
	slot.Ammo1[0] = 0;
	slot.Ammo2[0] = 0;

	// Default
	slot.FireRate[0] = 0.1f;
	slot.FireRate[1] = 0.1f;
	slot.WeaponSpread[0] = 0.1f;
	slot.WeaponSpread[1] = 0.1f;
	for ( int i = 0; i < WeaponDataIcons::MAX_ICONS; i++ )
		slot.Icons[i][0] = 0;

	slot.Slot = pWeaponScript->GetInt( "Slot", 0 );
	slot.Position = pWeaponScript->GetInt( "Position", 0 );

	const char *szAmmoType = pWeaponScript->GetString( "AmmoType1", NULL );
	if ( szAmmoType && szAmmoType[0] )
	{
		AmmoData ammo = GetAmmoByName( szAmmoType );
		if ( ammo.MaxCarry != -1 )
		{
			UTIL_strcpy( slot.Ammo1, szAmmoType );
			UTIL_strcpy( slot.Icons[WeaponDataIcons::ICON_AMMO1], szAmmoType );
		}
	}

	szAmmoType = pWeaponScript->GetString( "AmmoType2", NULL );
	if ( szAmmoType && szAmmoType[0] )
	{
		AmmoData ammo = GetAmmoByName( szAmmoType );
		if ( ammo.MaxCarry != -1 )
		{
			UTIL_strcpy( slot.Ammo2, szAmmoType );
			UTIL_strcpy( slot.Icons[WeaponDataIcons::ICON_AMMO2], szAmmoType );
		}
	}

	slot.DefaultAmmo = pWeaponScript->GetInt( "DefaultGive", 7 );
	slot.AmmoBox = pWeaponScript->GetInt( "AmmoBox", 7 );
	slot.MaxClip = pWeaponScript->GetInt( "MaxClip", 7 );
	slot.Bullets = pWeaponScript->GetInt( "Bullets", 1 );
	slot.Flags = 0;

	// Go trough our flags
	KeyValues *pWeaponFlags = pWeaponScript->FindKey( "Flags" );
	if ( pWeaponFlags )
	{
		if ( pWeaponFlags->GetBool( "SELECTONEMPTY" ) ) slot.Flags |= ITEM_FLAG_SELECTONEMPTY;
		if ( pWeaponFlags->GetBool( "NOAUTORELOAD" ) ) slot.Flags |= ITEM_FLAG_NOAUTORELOAD;
		if ( pWeaponFlags->GetBool( "NOAUTOSWITCHEMPTY" ) ) slot.Flags |= ITEM_FLAG_NOAUTOSWITCHEMPTY;
		if ( pWeaponFlags->GetBool( "LIMITINWORLD" ) ) slot.Flags |= ITEM_FLAG_LIMITINWORLD;
		if ( pWeaponFlags->GetBool( "EXHAUSTIBLE" ) ) slot.Flags |= ITEM_FLAG_EXHAUSTIBLE;
		if ( pWeaponFlags->GetBool( "NOAUTOSWITCHTO" ) ) slot.Flags |= ITEM_FLAG_NOAUTOSWITCHTO;
	}

	slot.Weight = pWeaponScript->GetFloat( "Weight", 0 );

	// Primary Attack stuff
	KeyValues *pPrimaryAttack = pWeaponScript->FindKey( "Primary" );
	if ( pPrimaryAttack )
	{
		slot.FireRate[0] = pPrimaryAttack->GetFloat( "FireRate", 0.1f );
		slot.WeaponSpread[0] = pPrimaryAttack->GetFloat( "Spread", 0.1f );
	}

	// Secondary Attack stuff
	KeyValues *pSecondaryAttack = pWeaponScript->FindKey( "Secondary" );
	if ( pSecondaryAttack )
	{
		slot.FireRate[1] = pSecondaryAttack->GetFloat( "FireRate", 0.1f );
		slot.WeaponSpread[1] = pSecondaryAttack->GetFloat( "Spread", 0.1f );
	}

	KeyValues *pWeaponSprites = pWeaponScript->FindKey( "HUD" );
	if ( pWeaponSprites )
	{
		const char *szIcon = pWeaponSprites->GetString( "Weapon", NULL );
		if ( szIcon && szIcon[0] )
		{
			UTIL_strcpy( slot.Icons[WeaponDataIcons::ICON_WEAPON], szIcon );
			char buffer[32];
			Q_snprintf( buffer, sizeof( buffer ), "%s_s", szIcon );
			UTIL_strcpy( slot.Icons[WeaponDataIcons::ICON_WEAPON_SELECTED], buffer );
		}
		szIcon = pWeaponSprites->GetString( "Crosshair", NULL );
		if ( szIcon && szIcon[0] )
			UTIL_strcpy( slot.Icons[WeaponDataIcons::ICON_CROSSHAIR], szIcon );
		szIcon = pWeaponSprites->GetString( "CrosshairAuto", NULL );
		if ( szIcon && szIcon[0] )
			UTIL_strcpy( slot.Icons[WeaponDataIcons::ICON_CROSSHAIR_AUTO], szIcon );
		szIcon = pWeaponSprites->GetString( "CrosshairZoom", NULL );
		if ( szIcon && szIcon[0] )
			UTIL_strcpy( slot.Icons[WeaponDataIcons::ICON_CROSSHAIR_ZOOM], szIcon );
		szIcon = pWeaponSprites->GetString( "CrosshairZoomAuto", NULL );
		if ( szIcon && szIcon[0] )
			UTIL_strcpy( slot.Icons[WeaponDataIcons::ICON_CROSSHAIR_ZOOMAUTO], szIcon );
	}

	pWeaponScript->deleteThis();

#if defined( CLIENT_DLL )
	UTIL_LogPrintf("[CLIENT] Loaded weapon script \"%s\" [%i]\n", szFile.c_str(), slot.WeaponID );
#else
	UTIL_LogPrintf("[SERVER] Loaded weapon script \"%s\" [%i]\n", szFile.c_str(), slot.WeaponID );
#endif

	sWeaponDataList.push_back( slot );
	return slot;
}

WeaponData CreateWeaponSlotData( ZPWeaponID WeaponID )
{
	const char *szWeaponScriptFile = nullptr;
	switch ( WeaponID )
	{
		case WEAPON_CROWBAR: szWeaponScriptFile = "weapon_crowbar"; break;
		case WEAPON_SWIPE: szWeaponScriptFile = "weapon_swipe"; break;
		case WEAPON_SIG: szWeaponScriptFile = "weapon_sig"; break;
		case WEAPON_PYTHON: szWeaponScriptFile = "weapon_357"; break;
		case WEAPON_MP5: szWeaponScriptFile = "weapon_mp5"; break;
		case WEAPON_556AR: szWeaponScriptFile = "weapon_556ar"; break;
		case WEAPON_SHOTGUN: szWeaponScriptFile = "weapon_shotgun"; break;
		case WEAPON_HANDGRENADE: szWeaponScriptFile = "weapon_handgrenade"; break;
		case WEAPON_SATCHEL: szWeaponScriptFile = "weapon_satchel"; break;
		case WEAPON_DOUBLEBARREL: szWeaponScriptFile = "weapon_doublebarrel"; break;
		case WEAPON_PPK: szWeaponScriptFile = "weapon_ppk"; break;
		case WEAPON_GLOCK17: szWeaponScriptFile = "weapon_glock17"; break;
		default: szWeaponScriptFile = "weapon_example"; break;
	}
#if defined( CLIENT_DLL )
	UTIL_LogPrintf("[CLIENT] Loading weapon script \"%s\" [i]\n", szWeaponScriptFile, WeaponID );
#else
	UTIL_LogPrintf("[SERVER] Loading weapon script \"%s\" [i]\n", szWeaponScriptFile, WeaponID );
#endif
	return CreateWeaponSlotData( szWeaponScriptFile );
}

AmmoData GetAmmoByName( const char *szAmmoType )
{
	if ( szAmmoType && szAmmoType[0] )
	{
		for ( int i = 0; i < ARRAYSIZE( sAmmoDataList ); i++ )
		{
			AmmoData ammo = sAmmoDataList[i];
			if ( !ammo.AmmoName ) continue;
			if ( FStrEq( ammo.AmmoName, szAmmoType ) )
				return ammo;
		}
	}
	// If we find nothing, grab the last item in the list.
	return sAmmoDataList[ ARRAYSIZE( sAmmoDataList ) - 1 ];
}

AmmoData GetAmmoByTableIndex(int id)
{
	int iFind = (int)clamp( id, 0, ARRAYSIZE( sAmmoDataList ) - 1 );
	return sAmmoDataList[ iFind ];
}

AmmoData GetAmmoByAmmoID( int id )
{
	for ( int i = 0; i < ARRAYSIZE( sAmmoDataList ); i++ )
	{
		AmmoData ammo = sAmmoDataList[i];
		if ( ammo.AmmoType == id )
			return ammo;
	}
	// If we find nothing, grab the last item in the list.
	return sAmmoDataList[ ARRAYSIZE( sAmmoDataList ) - 1 ];
}

AmmoData GetAmmoByWeaponID(ZPWeaponID id, bool bPrimary)
{
	WeaponData slot = GetWeaponSlotInfo( id );
	return GetAmmoByName( bPrimary ? slot.Ammo1 : slot.Ammo2 );
}

WeaponInfo GetWeaponInfo( const char *szClassname )
{
	if ( szClassname && szClassname[0] )
	{
		// Check if we have "weapon_", if we do, get rid of it.
		if ( StringHasPrefix( szClassname, "weapon_" ) )
			szClassname += 7;

		for ( int i = 0; i < ARRAYSIZE( sWeaponInfoList ); i++ )
		{
			WeaponInfo weapon = sWeaponInfoList[i];
			if ( FStrEq( weapon.szWeapon, szClassname ) )
				return weapon;
		}
	}
	return WeaponInfo();
}

WeaponInfo GetWeaponInfo( ZPWeaponID id )
{
	for ( int i = 0; i < ARRAYSIZE( sWeaponInfoList ); i++ )
	{
		WeaponInfo weapon = sWeaponInfoList[i];
		if ( weapon.WeaponID == id )
			return weapon;
	}
	return WeaponInfo();
}

WeaponInfo GetRandomWeaponInfo()
{
	WeaponInfo weapon = WeaponInfo();
	do
	{
		weapon = sWeaponInfoList[ RandomInt( 0, ARRAYSIZE( sWeaponInfoList ) - 1 ) ];
	}
	while ( !weapon.Hidden );
	// Grab a random item
	return weapon;
}

WeaponData GetWeaponSlotInfo( ZPWeaponID WeaponID )
{
	for ( int i = 0; i < sWeaponDataList.size(); i++ )
	{
		WeaponData slot = sWeaponDataList[i];
		if ( slot.WeaponID == WeaponID )
			return slot;
	}
	return CreateWeaponSlotData( WeaponID );
}

WeaponData GetWeaponSlotInfo( const char *szClassname )
{
	return GetWeaponSlotInfo( GetWeaponInfo( szClassname ).WeaponID );
}

#ifdef SERVER_DLL
struct StaticSpawn
{
	char Classname[32];
	int SpawnFlags;
	Vector Origin;
	Vector Angle;
};
static std::vector<StaticSpawn> sStaticSpawnList;
void ZP::ClearStaticSpawnList() { sStaticSpawnList.clear(); }

void ZP::AddToStaticSpawnList( string_t classname, int spawnflags, float flOrigin[3], float flAngle[3] )
{
	StaticSpawn item;
	UTIL_strcpy( item.Classname, STRING( classname ) );
	item.SpawnFlags = spawnflags;
	item.Origin.x = flOrigin[0];
	item.Origin.y = flOrigin[1];
	item.Origin.z = flOrigin[2];
	item.Angle.x = flAngle[0];
	item.Angle.y = flAngle[1];
	item.Angle.z = flAngle[2];

	sStaticSpawnList.push_back( item );
}

void ZP::SpawnStaticSpawns()
{
	for ( int i = 0; i < sStaticSpawnList.size(); i++ )
	{
		StaticSpawn slot = sStaticSpawnList[i];
		CBaseEntity::Create( (char *)slot.Classname, slot.Origin + Vector( 0, 0, 2 ), slot.Angle, nullptr );
	}
}

int ZP::GrabCorrectDecal( int iDamageFlag )
{
	// We don't use them as bits here, since only a single flag is sent here.
	switch ( iDamageFlag )
	{
		case DMG_CLUB: return DECAL_CROWBAR1 + RANDOM_LONG(0, 4);
		case DMG_SLASH: return DECAL_ZOMBIE_SWIPE1 + RANDOM_LONG(0, 4);
	}
	// Default, if we found nothing.
	return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}

void ZP::CheckIfBreakableGlass( TraceResult *pTrace, CBaseEntity *pEnt, const Vector &vDir, int iDamageFlag )
{
	// Check if this is a breakable window. If so, add a decal on the other side.
	CBreakable *pBreakable = dynamic_cast<CBreakable*>( pEnt );
	if ( pBreakable && pBreakable->m_Material == matGlass )
	{
		// Point backwards
		TraceResult tr;
		Vector vecSrc = pTrace->vecEndPos + vDir * 10;
		Vector vecEnd = pTrace->vecEndPos;
		UTIL_TraceLine( vecSrc, vecEnd, ignore_monsters, nullptr, &tr );
		if ( tr.flFraction <= 1.0 )
			UTIL_DecalTrace( &tr, iDamageFlag );
	}
}
#endif

ZP::GameModeType_e ZP::IsValidGameModeMap(const char *szLevel)
{
	if ( !szLevel ) return GameModeType_e::GAMEMODE_INVALID;
	char sz[4];
#if defined( CLIENT_DLL )
	char buf[64];
	V_FileBase( szLevel, buf, sizeof(buf) );
	V_strcpy_safe( sz, buf );
#else
	UTIL_strcpy( sz, szLevel );
#endif
	if ( !stricmp( sz, "zp_" ) ) return GameModeType_e::GAMEMODE_SURVIVAL;
	else if ( !stricmp( sz, "zpo" ) ) return GameModeType_e::GAMEMODE_OBJECTIVE;
	else if ( !stricmp( sz, "zph" ) ) return GameModeType_e::GAMEMODE_HARDCORE;
	return GameModeType_e::GAMEMODE_NONE;
}

float CBasePlayer::GetAmmoWeight( const char *szAmmo )
{
	// We only do this for survivors!!!
	if ( pev->team != ZP::TEAM_SURVIVIOR ) return 0.0f;
	AmmoData ammo = GetAmmoByName( szAmmo );
	int amount = m_rgAmmo[ ammo.AmmoType ];
	return amount * ammo.WeightPerBullet;
}

void CBasePlayer::UpdatePlayerMaxSpeed()
{
	int i;
	int iHowFatAmI = 0;

	// Count our ammo
	iHowFatAmI += GetAmmoWeight( "9mm" );
	iHowFatAmI += GetAmmoWeight( "357" );
	iHowFatAmI += GetAmmoWeight( "buckshot" );
	iHowFatAmI += GetAmmoWeight( "556ar" );

	for ( i = 0; i < MAX_ITEM_TYPES; i++ )
	{
		if ( m_rgpPlayerItems[i] )
			iHowFatAmI += m_rgpPlayerItems[i]->iWeight();
	}

#if !defined( CLIENT_DLL )
	// If we are in panic, ignore our weight
	if ( IsInPanic() )
		iHowFatAmI = 0;
#endif

	// Now check the weapons we got.
	float flNewSpeed = ZP::MaxSpeeds[0] - iHowFatAmI - pev->fuser4;
	if ( flNewSpeed < 50 )
		flNewSpeed = 50;

#if !defined( CLIENT_DLL )
	// If we are in panic, ignore our weight
	if ( IsInPanic() )
		flNewSpeed += ZP::ExtraPanicSpeed;
#endif

	pev->maxspeed = flNewSpeed;
}