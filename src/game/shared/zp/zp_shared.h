// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SHARED_ZOMBIEPANIC
#define SHARED_ZOMBIEPANIC
#pragma once

#include "Color.h"
#include <vector>

// Auto include em.
#include "zp_achievements.h"

/// <summary>
/// Objective states, used by info_objective.cpp & zp_objective.cpp
/// </summary>
enum ObjectiveState
{
	State_Normal = 0,
	State_InProgress,
	State_Completed,
	State_Failed
};

/// <summary>
/// The WeaponID's used by Zombie Panic!
/// </summary>
enum ZPWeaponID
{
	WEAPON_NONE = 0,
	WEAPON_CROWBAR,
	WEAPON_SWIPE,
	WEAPON_SIG,
	WEAPON_PYTHON,
	WEAPON_MP5,
	WEAPON_556AR,
	WEAPON_SHOTGUN,
	WEAPON_HANDGRENADE,
	WEAPON_SATCHEL,

	LAST_WEAPON_ID
};

/// <summary>
/// The available ammo types, used by Zombie Panic!.
/// This is also used by the new ammo table, since
/// the GoldSrc version sucked ass.
/// </summary>
enum ZPAmmoTypes
{
	AMMO_NONE = 0,
	AMMO_PISTOL,
	AMMO_MAGNUM,
	AMMO_SHOTGUN,
	AMMO_RIFLE,
	AMMO_GRENADE,
	AMMO_SATCHEL,

	AMMO_MAX
};

/// <summary>
/// The required data for our ammo. That's about it.
/// </summary>
struct AmmoData
{
	ZPAmmoTypes AmmoType;
	const char *AmmoName;
	int MaxCarry;
	float WeightPerBullet;
};
AmmoData GetAmmoByName( const char *szAmmoType );
AmmoData GetAmmoByTableIndex( int id );
AmmoData GetAmmoByAmmoID( int id );
AmmoData GetAmmoByWeaponID( ZPWeaponID id, bool bPrimary = true );

/// <summary>
/// The static weapon list
/// Used by CreateWeaponSlotData and 'give' command.
/// </summary>
struct WeaponInfo
{
	const char *szWeapon = nullptr;
	ZPWeaponID WeaponID = WEAPON_NONE;
	bool Hidden = false;
};
WeaponInfo GetWeaponInfo( const char *szClassname );
WeaponInfo GetWeaponInfo( ZPWeaponID id );
WeaponInfo GetRandomWeaponInfo();

enum WeaponDataIcons
{
	ICON_WEAPON = 0,
	ICON_WEAPON_SELECTED,
	ICON_AMMO1,
	ICON_AMMO2,
	ICON_CROSSHAIR,
	ICON_CROSSHAIR_AUTO,
	ICON_CROSSHAIR_ZOOM,
	ICON_CROSSHAIR_ZOOMAUTO,
	MAX_ICONS,
};

struct WeaponData
{
	ZPWeaponID WeaponID;
	int Slot;
	int Position;
	char Ammo1[16]; // ammo 1 type
	char Ammo2[16]; // ammo 2 type
	char Icons[WeaponDataIcons::MAX_ICONS][32]; // HUD Icons
	int DefaultAmmo;
	int AmmoBox; // How much should the ammo item give?
	int MaxClip;
	int Bullets = 1;
	int Flags;
	float Weight;
	float WeaponSpread[2]; // 0 - Primary, 1 - Secondary
	float FireRate[2]; // 0 - Primary, 1 - Secondary
};

/// <summary>
/// Grabs the weapon information by using the WeaponID.
/// </summary>
/// <param name="WeaponID">The WeaponID we want to look for</param>
/// <returns>Returns the data from our weapon script file</returns>
WeaponData GetWeaponSlotInfo( ZPWeaponID WeaponID );
WeaponData GetWeaponSlotInfo( const char *szClassname );

// Player Death flags
#define PLR_DEATH_FLAG_NONE				0					// No flag found
#define PLR_DEATH_FLAG_HEADSHOT			(1 << 0)			// Player now have a permanent headache
#define PLR_DEATH_FLAG_GIBBED			(1 << 1)			// Player got gibbed. Mmmm, free food
#define PLR_DEATH_FLAG_FELL				(1 << 2)			// Player fell to their death. Good job
#define PLR_DEATH_FLAG_BEYOND_GRAVE		(1 << 3)			// Player killed someone while already dead
#define PLR_DEATH_FLAG_TEAMKILLER		(1 << 4)			// Player was a friend :(


// Beacon Types (used by info_beacon entity)
enum BeaconTypes
{
	BEACON_BUTTON = 0,
	BEACON_DEFEND,
	BEACON_DESTROY,
	BEACON_WAYPOINT,
	BEACON_CAPTUREPOINT,
};

// Beacon Draw Types (used by info_beacon entity)
enum BeaconDrawTypes
{
	BEACON_DRAW_ALWAYS = 0, // Always draw, no matter what
	BEACON_DRAW_UNCOLLUDED, // Only draw when uncolluded
	BEACON_DRAW_COLLUDED, // Only draw when colluded
};


namespace ZP
{
#ifdef SERVER_DLL
    void ClearStaticSpawnList();
	void AddToStaticSpawnList( string_t classname, int spawnflags, float flOrigin[3], float flAngle[3] );
	void SpawnStaticSpawns();
    int GrabCorrectDecal( int iDamageFlag );
    void CheckIfBreakableGlass( TraceResult *pTrace, CBaseEntity *pEnt, const Vector &vDir, int iDamageFlag );
#endif

	enum RoundState
	{
		RoundState_Invalid = -1,
		RoundState_WaitingForPlayers = 0,
		RoundState_RoundIsStarting,
		RoundState_PickVolunteers,
		RoundState_RoundHasBegunPost,
		RoundState_RoundHasBegun,
		RoundState_RoundIsOver
	};
	
	enum Teams_e
	{
		TEAM_NONE,
		TEAM_OBSERVER,
		TEAM_SURVIVIOR,
		TEAM_ZOMBIE,
	
		MAX_TEAM
	};

	static int ExtraPanicSpeed = 60;

	static int MaxSpeeds[2] = {
		230,	// Human Max Speed
		180		// Zombie Max Speed
	};

	static int MaxHealth[2] = {
		100,	// Human Max Health
		200		// Zombie Max Health
	};

	static const char *Teams[MAX_TEAM] = {
		"",
		"Observer",
		"Survivor",
		"Zombie"
	};

	enum GameModeType_e
	{
		GAMEMODE_INVALID = -1,
		GAMEMODE_NONE,
		GAMEMODE_SURVIVAL,
		GAMEMODE_OBJECTIVE,
	
		MAX_GAMEMODES
	};
	GameModeType_e IsValidGameModeMap(const char *szLevel);
	static const char *GameModes[MAX_GAMEMODES] = {
		"Development",
		"Last Man Standing",
		"Objective"
	};
	
	namespace ColorGradient
	{
		class ColorBase
		{
		public:
			int r;
			int g;
			int b;
			int a;
			float flValue;
			ColorBase( Color cColor, float flPercent )
			{
				r = cColor.r();
				g = cColor.g();
				b = cColor.b();
				a = cColor.a();
				flValue = flPercent;
			}
		};
	
		class Base
		{
		public:
			void AddColor( ColorBase *value )
			{
				m_Colors.push_back( value );
			}
	
			Color GetColorForValue( float flValue, float flAlpha = 255 )
			{
				if ( m_Colors.size() < 1 )
					return Color( 255, 0, 255, flAlpha );
	
				for ( int i = 0; i < int(m_Colors.size()); i++ )
				{
					ColorBase *sCurrent = m_Colors[i];
	
					if ( flValue <= sCurrent->flValue )
					{
						int y = i-1;
						if ( y < 0 ) y = 0;
	
						ColorBase *sPrevious = m_Colors[y];
	
						float flDiff = (sPrevious->flValue - sCurrent->flValue);
						float flFrac = flDiff == 0.0f ? 0.0f : (flValue - sCurrent->flValue) / flDiff;
						int iRed     = Float2Int( (sPrevious->r - sCurrent->r) * flFrac + sCurrent->r );
						int iGreen   = Float2Int( (sPrevious->g - sCurrent->g) * flFrac + sCurrent->g );
						int iBlue    = Float2Int( (sPrevious->b - sCurrent->b) * flFrac + sCurrent->b );
	
						return Color( iRed, iGreen, iBlue, flAlpha );
					}
				}
				return Color( 255, 255, 255, flAlpha );
			}
	
			std::vector<ColorBase*> m_Colors;
		};
	
		class WhiteGreen : public Base
	    {
		public:
			WhiteGreen()
			{
				AddColor( new ColorBase( Color( 255, 255, 255 ), 0.099999999999999f ) );
				AddColor( new ColorBase( Color( 0, 255, 0 ), 1.000000f ) );
			}
		};
	
		class WhiteYellow : public Base
	    {
		public:
			WhiteYellow()
			{
				AddColor( new ColorBase( Color( 255, 255, 255 ), 0.099999999999999f ) );
				AddColor( new ColorBase( Color( 255, 255, 0 ), 1.000000f ) );
			}
		};
	
		class RedYellowGreen : public Base
		{
		public:
			RedYellowGreen()
			{
				AddColor( new ColorBase( Color( 255, 0, 0 ), 0.333333f ) ); // full red
				AddColor( new ColorBase( Color( 255, 255, 0 ), 0.666666f ) ); // full yellow
				AddColor( new ColorBase( Color( 0, 255, 0 ), 1.000000f ) ); // full green
			}
		};
	
		class RainBow : public Base
	    {
		public:
			RainBow()
			{
				AddColor( new ColorBase( Color( 255, 0, 0 ), 0.099999999999999f ) );
				AddColor( new ColorBase( Color( 255, 255, 0 ), 0.1666666666666667f ) );
				AddColor( new ColorBase( Color( 0, 255, 0 ), 0.5f ) );
				AddColor( new ColorBase( Color( 0, 255, 255 ), 0.6666666666666667f ) );
				AddColor( new ColorBase( Color( 0, 0, 255 ), 0.8333333333333333f ) );
				AddColor( new ColorBase( Color( 255, 0, 255 ), 1.000000f ) );
			}
		};
	}
}

#endif