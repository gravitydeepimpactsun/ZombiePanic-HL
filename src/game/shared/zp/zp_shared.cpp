// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#if defined(CLIENT_DLL)
#include "hud.h"
#include "strtools.h"
#include "steam_achievements.h"
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

// =========================================================
// Achievements Data Class
// =========================================================

std::vector<RequiredStepsTable> m_MarathonSteps = {
	RequiredStepsTable( MAP_ZP_SANTERIA, 1 ),
	RequiredStepsTable( MAP_ZP_CLUBZOMBO, 1 ),
	RequiredStepsTable( MAP_ZP_THELABS, 1 ),
	RequiredStepsTable( MAP_ZP_EASTSIDE, 1 ),
	RequiredStepsTable( MAP_ZP_INDUSTRY, 1 ),
	RequiredStepsTable( MAP_ZP_CONTINGENCY, 1 ),
	RequiredStepsTable( MAP_ZP_HAUNTED, 1 ),
	RequiredStepsTable( MAP_ZP_RUINS, 1 ),
	RequiredStepsTable( MAP_ZP_HOTEL, 1 ),
	RequiredStepsTable( MAP_ZP_TOWN, 1 ),
	RequiredStepsTable( MAP_ZP_MANSION, 1 ),
	RequiredStepsTable( MAP_ZPO_CONTINGENCY, 1 ),
	RequiredStepsTable( MAP_ZPH_EASTSIDE, 1 ),
	RequiredStepsTable( MAP_ZPH_INDUSTRY, 1 ),
	RequiredStepsTable( MAP_ZPH_HAUNTED, 1 ),
	RequiredStepsTable( MAP_ZPH_SANTERIA, 1 ),
	RequiredStepsTable( MAP_ZPH_THELABS, 1 ),
	RequiredStepsTable( MAP_ZPH_CONTINGENCY, 1 )
};

std::vector<RequiredStepsTable> m_MapsSurvivalSteps = {
	RequiredStepsTable( MAP_ZP_SANTERIA, 1 ),
	RequiredStepsTable( MAP_ZP_CLUBZOMBO, 1 ),
	RequiredStepsTable( MAP_ZP_THELABS, 1 ),
	RequiredStepsTable( MAP_ZP_EASTSIDE, 1 ),
	RequiredStepsTable( MAP_ZP_INDUSTRY, 1 ),
	RequiredStepsTable( MAP_ZP_CONTINGENCY, 1 ),
	RequiredStepsTable( MAP_ZP_HAUNTED, 1 ),
	RequiredStepsTable( MAP_ZP_RUINS, 1 ),
	RequiredStepsTable( MAP_ZP_HOTEL, 1 ),
	RequiredStepsTable( MAP_ZP_TOWN, 1 ),
	RequiredStepsTable( MAP_ZP_MANSION, 1 )
};

std::vector<RequiredStepsTable> m_MapsObjectiveSteps = {
	RequiredStepsTable( MAP_ZPO_CONTINGENCY, 1 )
};

// We only need to check for one single kill for each
std::vector<RequiredStepsTable> m_JackOfTradesSteps = {
	RequiredStepsTable( ZP_KILLS_CROWBAR, 1 ),
	RequiredStepsTable( ZP_KILLS_LEADPIPE, 1 ),
//	RequiredStepsTable( ZP_KILLS_PLANK, 1 ),
//	RequiredStepsTable( ZP_KILLS_SLEDGEHAMMER, 1 ),
//	RequiredStepsTable( ZP_KILLS_AXE, 1 ),
//	RequiredStepsTable( ZP_KILLS_MACHETE, 1 ),
//	RequiredStepsTable( ZP_KILLS_SHOVEL, 1 ),
//	RequiredStepsTable( ZP_KILLS_GOLFCLUB, 1 ),
//	RequiredStepsTable( ZP_KILLS_GUITAR, 1 ),
//	RequiredStepsTable( ZP_KILLS_GUITAR_BASE, 1 ),
//	RequiredStepsTable( ZP_KILLS_GUITAR_ELECTRIC, 1 ),
//	RequiredStepsTable( ZP_KILLS_WINEBOTTLE, 1 ),
//	RequiredStepsTable( ZP_KILLS_CHAIR, 1 ),
//	RequiredStepsTable( ZP_KILLS_KEYBOARD, 1 ),
//	RequiredStepsTable( ZP_KILLS_BASEBALLBAT, 1 ),
//	RequiredStepsTable( ZP_KILLS_BASEBALLBAT_METAL, 1 ),
	RequiredStepsTable( ZP_KILLS_PISTOL, 1 ),
//	RequiredStepsTable( ZP_KILLS_PPK, 1 ),
	RequiredStepsTable( ZP_KILLS_REVOLVER, 1 ),
	RequiredStepsTable( ZP_KILLS_RIFLE, 1 ),
	RequiredStepsTable( ZP_KILLS_MP5, 1 ),
	RequiredStepsTable( ZP_KILLS_SHOTGUN, 1 ),
	RequiredStepsTable( ZP_KILLS_DBARREL, 1 ),
	RequiredStepsTable( ZP_KILLS_SATCHEL, 1 ),
	RequiredStepsTable( ZP_KILLS_TNT, 1 )
};

static DialogAchievementData g_DAchievements[] =
{
	_ACH_ADD_ID(KILLS_CROWBAR,					CATEGORY_KILLS,			ZP_KILLS_CROWBAR),
	_ACH_ADD_ID(KILLS_LEADPIPE,					CATEGORY_KILLS,			ZP_KILLS_LEADPIPE),
	_ACH_ADD_ID(KILLS_PISTOL,					CATEGORY_KILLS,			ZP_KILLS_PISTOL),
	_ACH_ADD_ID(KILLS_REVOLVER,					CATEGORY_KILLS,			ZP_KILLS_REVOLVER),
	_ACH_ADD_ID(KILLS_RIFLE,					CATEGORY_KILLS,			ZP_KILLS_RIFLE),
	_ACH_ADD_ID(KILLS_MP5,						CATEGORY_KILLS,			ZP_KILLS_MP5),
	_ACH_ADD_ID(KILLS_SHOTGUN,					CATEGORY_KILLS,			ZP_KILLS_SHOTGUN),
	_ACH_ADD_ID(KILLS_DBARREL,					CATEGORY_KILLS,			ZP_KILLS_DBARREL),
	_ACH_ADD_ID(KILLS_SATCHEL,					CATEGORY_KILLS,			ZP_KILLS_SATCHEL),
	_ACH_ADD_ID(KILLS_TNT,						CATEGORY_KILLS,			ZP_KILLS_TNT),
	_ACH_ADD_ID(KILLS_ZOMBIE,					CATEGORY_KILLS,			ZP_KILLS_ZOMBIE),
	_ACH_ADD_ID(YOU_WILL_DIE_WITH_ME,			CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID(UNSAFE_HANDLING,				CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID_LIST(JACKOFTRADES,				CATEGORY_KILLS,			INVALID_STAT, m_JackOfTradesSteps),
	_ACH_ADD_ID(PANICRUSH,						CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID(FLEEESH,						CATEGORY_KILLS,			ZP_FLEEESH),
	_ACH_ADD_ID(ZOMBIEDESSERT,					CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID(SCREAM4ME,						CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID(INLINEP2,						CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID(CUTYOUDOWN,						CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID(RABBITBEAST,					CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID(ITS_A_MASSACRE,					CATEGORY_KILLS,			ZP_ITS_A_MASSACRE),
	_ACH_ADD_ID(HC_SNACKTIME,					CATEGORY_KILLS,			ZP_HC_SNACKTIME),
	_ACH_ADD_ID(HC_OVERWEIGHTKILLER,			CATEGORY_KILLS,			INVALID_STAT),
	_ACH_ADD_ID(GENOCIDESTEP3,					CATEGORY_KILLS,			ZP_GENOCIDESTEP3),
	_ACH_ADD_ID(DANCE_FLOOR,					CATEGORY_KILLS,			INVALID_STAT),

	_ACH_ADD_ID(FIRST_SURVIVAL,					CATEGORY_MAPS,			INVALID_STAT),
	_ACH_ADD_ID(FIRST_OBJECTIVE,				CATEGORY_MAPS,			INVALID_STAT),
	_ACH_ADD_ID(PARTOFHORDE,					CATEGORY_MAPS,			INVALID_STAT),
	_ACH_ADD_ID(CLOCKOUT,						CATEGORY_MAPS,			INVALID_STAT),
	_ACH_ADD_ID(THE_ATEAM,						CATEGORY_MAPS,			INVALID_STAT),
	_ACH_ADD_ID(PARTNERINCRIME,					CATEGORY_MAPS,			INVALID_STAT),
	_ACH_ADD_ID(LASTMANSTAND,					CATEGORY_MAPS,			INVALID_STAT),

	_ACH_ADD_ID_LIST(MARATHON,					CATEGORY_MAPS,			INVALID_STAT, m_MarathonSteps),
	_ACH_ADD_ID_LIST(PLAY_ALL_SURVIVAL,			CATEGORY_MAPS,			INVALID_STAT, m_MapsSurvivalSteps),
	_ACH_ADD_ID_LIST(PLAY_ALL_OBJECTIVE,		CATEGORY_MAPS,			INVALID_STAT, m_MapsObjectiveSteps),

	_ACH_ADD_ID(FIRST_ESCAPE,					CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(ESCAPE_ARTIST,					CATEGORY_GENERAL,		ZP_ESCAPE_ARTIST),
	_ACH_ADD_ID(PANIC_ATTACK,					CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(PANIC_100,						CATEGORY_GENERAL,		ZP_PANIC_100),
	_ACH_ADD_ID(MMMWAP,							CATEGORY_GENERAL,		ZP_MMMWAP),
	_ACH_ADD_ID(I_FELL,							CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(ONE_OF_US,						CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(FIRST_TO_DIE,					CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(TABLE_FLIP,						CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(ZMASH,							CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(DIE_BY_DOOR,					CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(PUMPUPSHOTGUN,					CATEGORY_GENERAL,		ZP_PUMPUPSHOTGUN),
	_ACH_ADD_ID(CHILDOFGRAVE,					CATEGORY_GENERAL,		ZP_CHILDOFGRAVE),
	_ACH_ADD_ID(REGEN_10K,						CATEGORY_GENERAL,		ZP_REGEN_10K),
	_ACH_ADD_ID(ILIVEAGAIN,						CATEGORY_GENERAL,		ZP_ILIVEAGAIN),
	_ACH_ADD_ID(HOUSEOFHORRORS,					CATEGORY_GENERAL,		INVALID_STAT),
	_ACH_ADD_ID(KILLYOSELF,						CATEGORY_GENERAL,		ZP_KILLYOSELF),
};

// =========================================================
// Weapon Script
// =========================================================

AmmoData::AmmoData( ZPAmmoTypes nType, const char *szName, int iAmmoBox, int iMaxCarry, float flWeight )
{
	AmmoType = nType;
	UTIL_strcpy( AmmoName, szName );
	AmmoBoxGive = iAmmoBox;
	MaxCarry = iMaxCarry;
	WeightPerBullet = flWeight;
}

#define WEAPON_SCRIPT_PATH "scripts/"
#define WEAPON_SCRIPT_FILE ".txt"
static std::vector<WeaponData> sWeaponDataList;
static AmmoData sAmmoDataList[] = {
	AmmoData( AMMO_PISTOL, "9mm", 15, 150, 0.21f ),
	AmmoData( AMMO_MAGNUM, "357", 6, 30, 0.65f ),
	AmmoData( AMMO_SHOTGUN, "buckshot", 6, 24, 1.35f ),
	AmmoData( AMMO_RIFLE, "556ar", 20, 150, 0.35f ),
	AmmoData( AMMO_LONGRIFLE, "longrifle", 10, 100, 0.48f ),

	AmmoData( AMMO_GRENADE, "explosive_tnt", 0, 3, 0.1f ),
	AmmoData( AMMO_SATCHEL, "explosive_ied", 0, 2, 0.5f ),

	// MUST BE LAST, DO NOT CHANGE THIS.
	// This is used by Crowbar and Swipe (or any other weapon that has no ammo)
	AmmoData( AMMO_NONE, "", 0, -1, 0.0f )
};

static WeaponInfo sWeaponInfoList[] = {
	// { const char *szWeapon, ZPWeaponID WeaponID, bool Hidden }
	{ "crowbar", WEAPON_CROWBAR, false },
	{ "leadpipe", WEAPON_LEADPIPE, false },
	{ "swipe", WEAPON_SWIPE, true },
	{ "sig", WEAPON_SIG, false },
	{ "ppk", WEAPON_PPK, false },
	{ "357", WEAPON_PYTHON, false },
	{ "mp5", WEAPON_MP5, false },
	{ "556ar", WEAPON_556AR, false },
	{ "shotgun", WEAPON_SHOTGUN, false },
	{ "doublebarrel", WEAPON_DOUBLEBARREL, false },
	{ "tnt", WEAPON_TNT, false },
	{ "ied", WEAPON_SATCHEL, false },
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
		//if ( pWeaponFlags->GetBool( "NOAUTOSWITCHEMPTY" ) ) slot.Flags |= ITEM_FLAG_NOAUTOSWITCHEMPTY; // Unused
		if ( pWeaponFlags->GetBool( "LIMITINWORLD" ) ) slot.Flags |= ITEM_FLAG_LIMITINWORLD;
		if ( pWeaponFlags->GetBool( "EXHAUSTIBLE" ) ) slot.Flags |= ITEM_FLAG_EXHAUSTIBLE;
		if ( pWeaponFlags->GetBool( "NOAUTOSWITCHTO" ) ) slot.Flags |= ITEM_FLAG_NOAUTOSWITCHTO;
		if ( pWeaponFlags->GetBool( "ALLOWDUPLICATE" ) ) slot.Flags |= ITEM_FLAG_ALLOWDUPLICATE;
	}

	slot.Weight = pWeaponScript->GetFloat( "Weight", 0 );
	slot.DoubleSlots = pWeaponScript->GetBool( "DoubleSlots", false );

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
		case WEAPON_LEADPIPE: szWeaponScriptFile = "weapon_leadpipe"; break;
		case WEAPON_SWIPE: szWeaponScriptFile = "weapon_swipe"; break;
		case WEAPON_SIG: szWeaponScriptFile = "weapon_sig"; break;
		case WEAPON_PYTHON: szWeaponScriptFile = "weapon_357"; break;
		case WEAPON_MP5: szWeaponScriptFile = "weapon_mp5"; break;
		case WEAPON_556AR: szWeaponScriptFile = "weapon_556ar"; break;
		case WEAPON_SHOTGUN: szWeaponScriptFile = "weapon_shotgun"; break;
		case WEAPON_TNT: szWeaponScriptFile = "weapon_tnt"; break;
		case WEAPON_SATCHEL: szWeaponScriptFile = "weapon_ied"; break;
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

float CBasePlayer::GetTotalWeight()
{
	int i;
	float flWeight = 0.0f;
	// Count our ammo
	flWeight += GetAmmoWeight( "9mm" );
	flWeight += GetAmmoWeight( "357" );
	flWeight += GetAmmoWeight( "buckshot" );
	flWeight += GetAmmoWeight( "556ar" );
	for ( i = 0; i < MAX_ITEM_TYPES; i++ )
	{
		if ( m_rgpPlayerItems[i] )
		{
			CBasePlayerWeapon *pWeapon = (CBasePlayerWeapon *)m_rgpPlayerItems[i];
			while (pWeapon)
			{
				flWeight += pWeapon->iWeight();
				pWeapon = (CBasePlayerWeapon *)pWeapon->m_pNext;
			}
		}
	}
#if defined( CLIENT_DLL )
	if ( (gHUD.m_iWeaponBits & (1 << WEAPON_BACKPACK)) )
#else
	if ( HasBackpack() )
#endif
		flWeight += 10.0f; // Backpack adds 10 pounds of weight
	return flWeight;
}

void CBasePlayer::UpdatePlayerMaxSpeed()
{
	int iHowFatAmI = GetTotalWeight();

#if !defined( CLIENT_DLL )
	// If we are in panic, ignore our weight
	if ( IsInPanic() )
		iHowFatAmI = 0;
#endif

	// Now check the weapons we got.
	bool bUseZombieSpeed = ( pev->team == ZP::TEAM_ZOMBIE );
	float flNewSpeed = ZP::MaxSpeeds[bUseZombieSpeed ? 1 : 0] - iHowFatAmI - pev->fuser4;
	if ( flNewSpeed < 50 )
		flNewSpeed = 50;

#if !defined( CLIENT_DLL )
	// If we are in panic, ignore our weight
	if ( IsInPanic() )
		flNewSpeed += ZP::ExtraPanicSpeed;
#endif

	pev->maxspeed = flNewSpeed;
}

// =========================================================
// Achievements Data Class
// =========================================================

DialogAchievementData GetAchievementByID( int eAchievement )
{
	for ( int i = 0; i < ARRAYSIZE( g_DAchievements ); i++ )
	{
		DialogAchievementData item = g_DAchievements[ i ];
		if ( item.GetAchievementID() == eAchievement )
			return item;
	}
	return g_DAchievements[0];
}

DialogAchievementData GetAchievementByID( const char *szAchievementID )
{
	if ( !szAchievementID || !szAchievementID[0] ) return g_DAchievements[0];
	for ( int i = 0; i < ARRAYSIZE( g_DAchievements ); i++ )
	{
		DialogAchievementData item = g_DAchievements[ i ];
		if ( FStrEq( szAchievementID, item.GetAchievementName() ) )
			return item;
	}
	return g_DAchievements[0];
}

void SetAchievementCompletedByID( DialogAchievementData nAch, bool bState )
{
	for ( int i = 0; i < ARRAYSIZE( g_DAchievements ); i++ )
	{
		DialogAchievementData &item = g_DAchievements[ i ];
		if ( item.GetAchievementID() == nAch.GetAchievementID() )
		{
			item.SetAchieved( bState );
			break;
		}
	}
}

DialogAchievementData::DialogAchievementData( EAchievements nID, const char *szName, int nCategory, EStats nStatID )
{
	Q_snprintf( m_pchAchievementID, sizeof(m_pchAchievementID), "%s", szName );
	m_bAchieved = false;
	m_iIconImage = 0;
	m_eCategory = nCategory;
	m_eAchievementID = nID;
	m_nStat = nStatID;
}

DialogAchievementData::DialogAchievementData( EAchievements nID, const char *szName, int nCategory, EStats nStatID, std::vector<RequiredStepsTable> nRequiredSteps )
{
	Q_snprintf( m_pchAchievementID, sizeof(m_pchAchievementID), "%s", szName );
	m_bAchieved = false;
	m_iIconImage = 0;
	m_eCategory = nCategory;
	m_eAchievementID = nID;
	m_nStat = nStatID;
	m_RequiredSteps = nRequiredSteps;
}

StatData_t DialogAchievementData::GetData()
{
#if defined( CLIENT_DLL )
	return GrabStat( m_nStat );
#else
	return StatData_t();
#endif
}
