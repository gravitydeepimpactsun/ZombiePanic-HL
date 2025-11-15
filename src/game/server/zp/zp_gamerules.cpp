// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "zp_gamerules.h"
#include "game.h"
#include "shake.h"
#include "zp/info_random_base.h"
#ifdef SCRIPT_SYSTEM
#include "core.h"
#endif

extern DLL_GLOBAL BOOL g_fGameOver;

// Our max rounds! (before we change to another level)
extern cvar_t roundlimit;

extern unsigned short m_usResetDecals;

extern void CleanupBodyQue();

static const char *s_EntitiesRestarts[] = {
	"prop_objective",
	"cycler",
	"cycler_sprite",
	"light",
	"func_breakable",
	"func_door",
	"func_button",
	"func_rot_button",
	"env_fog",
	"env_render",
	"env_spark",
	"trigger_push",
	"func_water",
	"func_door_rotating",
	"func_tracktrain",
	"func_vehicle",
	"func_train",
	"func_pushable",
	"ambient_generic",
	"env_sprite",
	"trigger_once",
	"func_wall",
	"func_wall_toggle",
	"func_illusionary",
	"func_healthcharger",
	"func_recharge",
	"trigger_hurt",
	"multisource",
	"env_beam",
	"env_laser",
	"trigger_relay",
	"trigger_auto",
	"trigger_multiple",
	"trigger_escape",
	"info_objective",
	"info_beacon",
	"multi_manager",
	"multi_relay",
	"game_counter",
	"game_counter_set",
	"game_timer",
	"path_track",
	"info_player_team1",
	"info_player_team2",
	"", // END Marker
};

CZombiePanicGameRules::CZombiePanicGameRules()
{
	m_DisableDeathMessages = FALSE;
	m_DisableDeathPenalty = FALSE;
	m_bHasPickedVolunteer = false;
	m_flRoundRestartDelay = -1;
	m_flRoundJustBegun = -1;
	m_Volunteers.clear();

	m_iRounds = 1;

	m_GameModeType = ZP::IsValidGameModeMap( STRING( gpGlobals->mapname ) );
	switch ( m_GameModeType )
	{
		case ZP::GAMEMODE_SURVIVAL: m_pGameMode = new ZPGameMode_Survival; break;
		case ZP::GAMEMODE_OBJECTIVE: m_pGameMode = new ZPGameMode_Objective; break;
		case ZP::GAMEMODE_HARDCORE: m_pGameMode = new ZPGameMode_HardCore; break;
		default: m_pGameMode = new ZPGameMode_Dev; break;
	}
	ZP::SetCurrentGameMode( m_pGameMode );
}

CZombiePanicGameRules::~CZombiePanicGameRules()
{
	ZP::SetCurrentGameMode( nullptr );
}

#include "voice_gamemgr.h"
extern CVoiceGameMgr g_VoiceGameMgr;
extern cvar_t timeleft;

void CZombiePanicGameRules ::Think(void)
{
	CGameRules::Think();

#ifdef SCRIPT_SYSTEM
	ScriptSystem::OnThink();
#endif

	g_VoiceGameMgr.Update(gpGlobals->frametime);

	if ( g_fGameOver ) // someone else quit the game already
	{
		BaseClass::Think();
		return;
	}

	IGameModeBase::WinState_e eWinState = m_pGameMode->GetWinState();

	static int last_time;

	int time_remaining = 0;

	float flTimeLimit = CVAR_GET_FLOAT( "mp_timelimit" ) * 60;
	time_remaining = (int)(flTimeLimit ? (flTimeLimit - gpGlobals->time) : 0);
	if ( flTimeLimit != 0 && gpGlobals->time >= flTimeLimit && eWinState >= IGameModeBase::WinState_e::State_Draw )
	{
		GoToIntermission();
		return;
	}

	// Updates once per second
	if (timeleft.value != last_time)
	{
		if ( time_remaining <= 0 )
			time_remaining = 0;
		g_engfuncs.pfnCvar_DirectSet(&timeleft, UTIL_VarArgs("%i", time_remaining));
	}

	last_time = time_remaining;

	if ( eWinState >= IGameModeBase::WinState_e::State_Draw || m_pGameMode->GetRoundState() == ZP::RoundState::RoundState_RoundIsOver )
	{
		m_pGameMode->SetRoundState( ZP::RoundState::RoundState_RoundIsOver );
		ResetRound();
		return;
	}

	// We do the gamemode thinking on a seperate class, to make it less
	// of a mess.
	m_pGameMode->OnGameModeThink();

	switch ( m_pGameMode->GetRoundState() )
	{
		case ZP::RoundState::RoundState_PickVolunteers: PickRandomVolunteer(); break;
	}
}

extern int gmsgGameMode;
extern int gmsgSayText;
extern int gmsgTeamInfo;
extern int gmsgTeamNames;
extern int gmsgScoreInfo;
extern int gmsgRounds;
extern int gmsgVGUIMenu;
extern int gmsgRoundState;
extern int gmsgBeaconReset;

void CZombiePanicGameRules::UpdateGameMode(CBasePlayer *pPlayer)
{
	if ( !pPlayer ) return;
	if ( !pPlayer->IsConnected() ) return;
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(m_GameModeType);
	MESSAGE_END();
	MESSAGE_BEGIN(MSG_ONE, gmsgRounds, NULL, pPlayer->edict());
	WRITE_BYTE(m_iRounds);
	MESSAGE_END();
}

const char *CZombiePanicGameRules::SetDefaultPlayerTeam(CBasePlayer *pPlayer)
{
	return pPlayer->TeamID();
}

//=========================================================
// InitHUD
//=========================================================
void CZombiePanicGameRules::InitHUD(CBasePlayer *pPlayer)
{
	if ( !pPlayer ) return;
	if ( !pPlayer->IsConnected() ) return;
	BaseClass::InitHUD(pPlayer);

	// update the current player of the team he is joining
	char text[256];

	// Send down the team names
	MESSAGE_BEGIN(MSG_ONE, gmsgTeamNames, NULL, pPlayer->edict());
	WRITE_BYTE(ZP::MAX_TEAM);
	for (int i = 0; i < ZP::MAX_TEAM; i++)
	{
		WRITE_STRING(ZP::Teams[i]);
	}
	MESSAGE_END();

	ChangePlayerTeam(pPlayer, ZP::Teams[ZP::TEAM_OBSERVER], FALSE, FALSE);

	// loop through all active players and send their team info to the new client
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBaseEntity *plr = UTIL_PlayerByIndex(i);
		if (plr && IsValidTeam(plr->TeamID()))
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pPlayer->edict());
			WRITE_BYTE(plr->entindex());
			WRITE_STRING(plr->pev->iuser1 ? "" : plr->TeamID());
			MESSAGE_END();
		}
	}

	m_pGameMode->OnHUDInit( pPlayer );
}

void CZombiePanicGameRules::ChangePlayerTeam(CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib)
{
	int clientIndex = pPlayer->entindex();

	if (bKill)
	{
		// kill the player,  remove a death,  and let them start on the new team
		m_DisableDeathMessages = TRUE;
		m_DisableDeathPenalty = TRUE;

		int damageFlags = DMG_GENERIC | (bGib ? DMG_ALWAYSGIB : DMG_NEVERGIB);
		pPlayer->TakeDamage(pPlayer->pev, pPlayer->pev, 10000, damageFlags);

		m_DisableDeathMessages = FALSE;
		m_DisableDeathPenalty = FALSE;
	}

	// Set team to player
	pPlayer->pev->team = GetTeamIndex( pTeamName );

	// notify everyone's HUD of the team change
	MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
	WRITE_BYTE(clientIndex);
	WRITE_STRING(pPlayer->pev->iuser1 ? "" : pPlayer->TeamID());
	MESSAGE_END();

	pPlayer->SendScoreInfo();
}

float CZombiePanicGameRules::FlPlayerFallDamage(CBasePlayer *pPlayer)
{
	int iFallDamage = (int)falldamage.value;
	switch ( iFallDamage )
	{
		// progressive
		case 1:
		{
			pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
			float flRes = pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
		    if ( pPlayer->pev->team == ZP::TEAM_ZOMBIE )
			    flRes = clamp( flRes, 0, 15 );
		    return flRes;
		}
		// no damage
		case 2: return 0;
		// fixed
		default: return 10;
	}
}

void CZombiePanicGameRules::ResetRound()
{
	if ( m_flRoundRestartDelay == -1 )
	{
		IGameModeBase::WinState_e winner = m_pGameMode->GetWinState();

		// Fade out to black!
		UTIL_ScreenFadeAll( Vector( 0, 0, 0 ), 1.0f, 5.0f, 255, FFADE_OUT );
		MESSAGE_BEGIN( MSG_ALL, gmsgRoundState );
		WRITE_SHORT( m_pGameMode->GetRoundState() );
		WRITE_SHORT( winner );
		MESSAGE_END();
		m_flRoundRestartDelay = gpGlobals->time + 5.0f;

		int iPlayers = 0;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
			if ( plr && plr->IsConnected() && plr->pev->team == ZP::TEAM_SURVIVIOR )
				iPlayers++;
		}

		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
			if ( plr )
			{
				// No more buttons
				plr->pev->button = 0;

				// For these, we do not care about team etc.
				plr->GiveAchievement( EAchievements::MARATHON );
				plr->GiveAchievement( EAchievements::PLAY_ALL_SURVIVAL );
				plr->GiveAchievement( EAchievements::PLAY_ALL_OBJECTIVE );

				int iTeam = plr->pev->team;
				if ( iTeam == ZP::TEAM_SURVIVIOR && winner == IGameModeBase::WinState_e::State_SurvivorWin )
				{
					// Last survivor?
					if ( iPlayers == 1 )
						plr->GiveAchievement( EAchievements::LASTMANSTAND );
					// 4 or more players?
					if ( iPlayers >= 4 )
						plr->GiveAchievement( EAchievements::THE_ATEAM );
					plr->GiveAchievement( EAchievements::PARTNERINCRIME ); // Only give this if we are actually winning
					switch ( m_pGameMode->GetGameModeType() )
					{
						case ZP::GameModeType_e::GAMEMODE_SURVIVAL:
						{
							if ( m_pGameMode->HasTimeRanOut() )
								plr->GiveAchievement( EAchievements::CLOCKOUT );
							plr->GiveAchievement( EAchievements::FIRST_SURVIVAL );
						}
						break;

						case ZP::GameModeType_e::GAMEMODE_OBJECTIVE: plr->GiveAchievement( EAchievements::FIRST_OBJECTIVE ); break;
					}
				}
				else if ( iTeam == ZP::TEAM_ZOMBIE && winner == IGameModeBase::WinState_e::State_ZombieWin )
				{
					plr->GiveAchievement( EAchievements::PARTNERINCRIME ); // Only give this if we are actually winning
					plr->GiveAchievement( EAchievements::PARTOFHORDE );
				}
			}
		}
		return;
	}
	if ( m_flRoundRestartDelay - gpGlobals->time > 0 ) return;
	m_flRoundRestartDelay = -1;

	// Reset volunteers
	ResetVolunteers();

	// Check round limit. If we reached it, go to intermission
	// instead of restarting the round.
	int iRoundLimit = (int)roundlimit.value;
	if ( iRoundLimit > 0 )
	{
		// We need to change the map
		if ( m_iRounds >= iRoundLimit )
		{
			GoToIntermission();
			return;
		}
	}

	// Reset all map objects
	CleanUpMap();

	// Respawn all players
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
		if ( plr )
		{
			plr->StopObserver();
			ChangePlayerTeam(plr, ZP::Teams[ZP::TEAM_OBSERVER], FALSE, FALSE);
			plr->StartWelcomeCam();
			plr->SetHasEscaped( false );
			// The player is now naked :)
			plr->RemoveAllItems( TRUE );
			// This is being applied in StartWelcomeCam(),
			// but we do it again, just incase if m_bInWelcomeCam
			// was never set back to FALSE.
			plr->pev->solid = SOLID_NOT;
			plr->pev->effects |= EF_NODRAW;
			// Make sure this is off.
			plr->m_bNoLives = false;
			SetPlayerModel( plr );
		}
	}

	m_pGameMode->RestartRound();

	// Show the team menu for all players
	MESSAGE_BEGIN(MSG_ALL, gmsgVGUIMenu);
	WRITE_BYTE(2);
	MESSAGE_END();

	// Increase the round value
	m_iRounds++;

	// Make sure we tell everyone that our rounds have increased
	MESSAGE_BEGIN(MSG_ALL, gmsgRounds, NULL);
	WRITE_BYTE(m_iRounds);
	MESSAGE_END();

	// Make sure we clear our beacons
	MESSAGE_BEGIN(MSG_ALL, gmsgBeaconReset, NULL);
	MESSAGE_END();
}

void CZombiePanicGameRules::CleanUpMap()
{
	// Cleanup player inventories first
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
		if ( plr )
			plr->RemoveAllItems( TRUE );
	}

	// Release or reset everything entities in depending of flags ObjectCaps
	// (FCAP_MUST_RESET / FCAP_MUST_RELEASE)
	UTIL_ResetEntities();

	// Recreate all the map entities from the map data (preserving their indices),
	// then remove everything else except the players.
	for ( int i = 0; i < ARRAYSIZE( s_EntitiesRestarts ); i++ )
		UTIL_RestartOther( s_EntitiesRestarts[i] );

	// Make sure these are removed.
	UTIL_RemoveAll( "monster_satchel" );
	UTIL_RemoveAll( "grenade" );
	UTIL_RemoveAll( "weaponbox" );
	UTIL_RemoveAll( "bodyque" ); // We don't want any bodies on next round

	// Fix the body que on the next round
	CleanupBodyQue();

	// Spawn our static stuff
	ZP::SpawnStaticSpawns();

	// Make sure we reset our decals.
	PLAYBACK_EVENT((FEV_GLOBAL | FEV_RELIABLE), 0, m_usResetDecals);
}

void CZombiePanicGameRules::ResetVolunteers()
{
	m_bHasPickedVolunteer = false;
	m_Volunteers.clear();
	// Check if we should clear it before hand.
	m_pGameMode->ShouldClearChoosenZombies();
}

void CZombiePanicGameRules::PickRandomVolunteer()
{
	if ( m_bHasPickedVolunteer ) return;
	m_bHasPickedVolunteer = true;
	// Punish players who join late
	m_flRoundJustBegun = gpGlobals->time + 60;
	if ( m_pGameMode->IsTestModeActive() ) return;
	int iMoreRequired = 0;
	bool bFirstTry = false;

add_one_more_zombie:

	// We have no volunteers, so lets pick some random survivors
	if ( m_Volunteers.size() == 0 )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex(i);
			if ( plr && plr->IsAlive() && !m_pGameMode->WasAlreadyChoosenPreviously( plr ) && plr->pev->team == ZP::TEAM_SURVIVIOR )
				m_Volunteers.push_back( i );
		}
	}

	// Still zero? Reset the choosen list and try again.
	// This happens if a player is still a spectator.
	if ( m_Volunteers.size() == 0 )
	{
		if ( !bFirstTry )
		{
			bFirstTry = true;
			m_pGameMode->ClearChoosenZombies();
			// Try again.
			goto add_one_more_zombie;
		}
		return;
	}

	int iVolunteers = m_Volunteers.size() - 1;
	int iPlayerIndex = 0;
	int iVolunteerIndex = 0;
	if ( iVolunteers > 0 )
		iVolunteerIndex = RANDOM_LONG( 0, iVolunteers );
	iPlayerIndex = m_Volunteers[iVolunteerIndex];

	if ( iPlayerIndex == 0 ) return;
	m_Volunteers.erase( m_Volunteers.begin() + iVolunteerIndex );
	CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( iPlayerIndex );
	if ( plr )
	{
		ChangePlayerTeam( plr, ZP::Teams[ZP::TEAM_ZOMBIE], FALSE, FALSE );
		plr->Spawn();
		m_pGameMode->AddToChoosenList( plr );
		iMoreRequired--;
	}

	// Count the survivors. If we have 8 survivors, add 1 more.
	// and if we got more than 15, make sure we always have 3 zombies.
	if ( iMoreRequired == -1 )
	{
		int iSurvivors = 0;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBaseEntity *plr = UTIL_PlayerByIndex(i);
			if ( plr && plr->IsAlive() && plr->pev->team == ZP::TEAM_SURVIVIOR )
				iSurvivors++;
		}

		if ( iSurvivors > 15 )
			iMoreRequired = 2;
		else if ( iSurvivors >= 8 )
			iMoreRequired = 1;
	}

	// If it's higher than 0, then add the next one.
	if ( iMoreRequired > 0 )
		goto add_one_more_zombie;
}

void CZombiePanicGameRules::PlayerSpawn(CBasePlayer *pPlayer)
{
	// Start welcome cam for new players
	if (!pPlayer->m_bPutInServer && mp_welcomecam.GetBool() != 0)
	{
		// don't let him spawn as soon as he enters the server
		// give enough time to plugins to send the player to spectator mode
		pPlayer->m_flNextAttack = mp_welcomecam_delay.GetFloat();

		pPlayer->StartWelcomeCam();

		// We just joined.
		pPlayer->m_bNoLives = false;
		SetPlayerModel( pPlayer );
		return;
	}

	SetPlayerModel( pPlayer );

	int iTeamNumber = pPlayer->pev->team;
	if ( iTeamNumber == ZP::TEAM_OBSERVER ) return;

	int aws = pPlayer->m_iAutoWepSwitch;
	pPlayer->m_iAutoWepSwitch = 1;

	pPlayer->pev->weapons |= (1 << WEAPON_SUIT);

	// Zombies
	if ( iTeamNumber == ZP::TEAM_ZOMBIE )
	{
		// Zombie arms!
		// That's all that the zombies get!
		pPlayer->GiveNamedItem( "weapon_swipe" );

		// Zombies can't choose weapons, they only got their arms.
		pPlayer->m_iHideHUD |= HIDEHUD_WEAPONS;
	}

	FireTargets("game_playerspawn", pPlayer, pPlayer, USE_TOGGLE, 0);

	pPlayer->m_iAutoWepSwitch = aws;

	m_pGameMode->OnPlayerSpawned( pPlayer );
}

BOOL CZombiePanicGameRules::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	if (FStrEq(pcmd, "joingame"))
	{
		if ( pPlayer->pev->team == ZP::TEAM_OBSERVER || pPlayer->m_bInWelcomeCam )
		{
			// Go no more lives? stop.
			// This only applies if the player is on TEAM_OBSERVER
			if ( pPlayer->m_bNoLives )
			{
				if ( pPlayer->pev->team == ZP::TEAM_OBSERVER && !pPlayer->IsObserver() )
					pPlayer->StartObserver();
				return TRUE;
			}

			// Stop being spec. Thanks.
			if ( pPlayer->IsObserver() )
				pPlayer->StopObserver();

			const char *pVolunteer = CMD_ARGV(1);
			if ( pVolunteer && pVolunteer[0] )
			{
				if (FStrEq(pVolunteer, "volunteer"))
					m_Volunteers.push_back( pPlayer->entindex() );
			}
			bool bLateJoin = ( m_pGameMode->GetRoundState() == ZP::RoundState::RoundState_RoundHasBegun ) ? true : false;
			if ( bLateJoin
				&& m_flRoundJustBegun - gpGlobals->time > 0
				&& !m_pGameMode->HasLeftMidRound( pPlayer ) )
			{
				bLateJoin = false;
				pPlayer->m_bPunishLateJoiner = true;
			}

			ChangePlayerTeam(pPlayer, ZP::Teams[ bLateJoin ? ZP::TEAM_ZOMBIE : ZP::TEAM_SURVIVIOR], FALSE, FALSE);
			pPlayer->RemoveAllItems( TRUE );
			pPlayer->StopWelcomeCam();
			if ( pPlayer->m_bPunishLateJoiner )
				m_pGameMode->GiveWeapons( pPlayer );
			if ( m_pGameMode->GetRoundState() < ZP::RoundState::RoundState_RoundHasBegun )
				pPlayer->m_iHideHUD = HIDEHUD_WEAPONS | HIDEHUD_HEALTH | HIDEHUD_FLASHLIGHT;
		}
		return TRUE;
	}
	else if (FStrEq(pcmd, "dropcurrentweapon"))
	{
		if ( pPlayer->pev->team == ZP::TEAM_SURVIVIOR )
			pPlayer->DropActiveWeapon();
		return TRUE;
	}
	else if (FStrEq(pcmd, "dropammo"))
	{
		if ( pPlayer->pev->team == ZP::TEAM_SURVIVIOR )
			pPlayer->DropSelectedAmmo();
		return TRUE;
	}
	else if (FStrEq(pcmd, "changeammotype"))
	{
		if ( pPlayer->pev->team == ZP::TEAM_SURVIVIOR )
			pPlayer->ChangeAmmoTypeToDrop();
		return TRUE;
	}
	else if (FStrEq(pcmd, "dua"))
	{
		if ( pPlayer->pev->team == ZP::TEAM_SURVIVIOR )
			pPlayer->DropUnuseableAmmo();
		return TRUE;
	}
	else if (FStrEq(pcmd, "panic"))
	{
		if ( pPlayer->pev->team == ZP::TEAM_SURVIVIOR )
			pPlayer->DoPanic();
		return TRUE;
	}
	else if (FStrEq(pcmd, "achearn"))
	{
		const char *pAchivement = CMD_ARGV(1);
		if (pAchivement && pAchivement[0])
			pPlayer->NotifyOfEarnedAchivement( atoi( pAchivement ) );
		return TRUE;
	}
	else if (FStrEq(pcmd, "give"))
	{
		int iszItem = ALLOC_STRING( CMD_ARGV(1) ); // Make a copy of the classname
		OnWeaponGive( pPlayer, STRING(iszItem) );
		return TRUE;
	}
	else if (FStrEq(pcmd, "report_entities"))
	{
		ZP::CheckHowManySpawnedItems( pPlayer );
		return TRUE;
	}
	else if (FStrEq(pcmd, "slot"))
	{
		const char *pSelectedWeaponSlot = CMD_ARGV(1);
		if ( pSelectedWeaponSlot && pSelectedWeaponSlot[0] )
			pPlayer->SelectWeaponFromSlot( atoi( pSelectedWeaponSlot ) );
		return TRUE;
	}
	else if (FStrEq(pcmd, "invnext"))
	{
		pPlayer->SelectNextSlot();
		return TRUE;
	}
	else if (FStrEq(pcmd, "invprev"))
	{
		pPlayer->SelectPreviousSlot();
		return TRUE;
	}
	else if (FStrEq(pcmd, "ent_fire"))
	{
		const char *pSetCommand = CMD_ARGV(1);
		bool bIsCheatsEnabled = CVAR_GET_FLOAT("sv_cheats") >= 1 ? true : false;
		if ( bIsCheatsEnabled && pSetCommand && pSetCommand[0] )
			FireTargets( pSetCommand, pPlayer, pPlayer, USE_TOGGLE, 0);
		return TRUE;
	}
	else if (FStrEq(pcmd, "_set"))
	{
		const char *pSetCommand = CMD_ARGV(1);
		bool bIsCheatsEnabled = CVAR_GET_FLOAT("sv_cheats") >= 1 ? true : false;
		if ( bIsCheatsEnabled
			&& pSetCommand
			&& pSetCommand[0]
			&& ( pPlayer->pev->team == ZP::TEAM_SURVIVIOR || pPlayer->pev->team == ZP::TEAM_ZOMBIE ) )
		{
			if ( FStrEq( pSetCommand, "god" ) )
			{
				// If we have the godmode flag, remove it
				if ( (pPlayer->pev->flags & FL_GODMODE) )
					pPlayer->pev->flags &= ~FL_GODMODE;
				else
					pPlayer->pev->flags |= FL_GODMODE;
				UTIL_PrintConsole( UTIL_VarArgs( "God mode has been turned %s\n", (pPlayer->pev->flags & FL_GODMODE) ? "on" : "off" ), pPlayer );
			}
			else if ( FStrEq( pSetCommand, "buddha" ) )
			{
				// If we have the godmode flag, remove it
				pPlayer->m_bBuddhaMode = !pPlayer->m_bBuddhaMode;
				UTIL_PrintConsole( UTIL_VarArgs( "Buddha mode has been turned %s\n", (pPlayer->m_bBuddhaMode) ? "on" : "off" ), pPlayer );
			}
			else if ( FStrEq( pSetCommand, "noclip" ) )
			{
				// If we have the godmode flag, remove it
				if ( pPlayer->pev->movetype == MOVETYPE_NOCLIP )
					pPlayer->pev->movetype = MOVETYPE_WALK;
				else
					pPlayer->pev->movetype = MOVETYPE_NOCLIP;
				UTIL_PrintConsole( UTIL_VarArgs( "Noclip has been turned %s\n", (pPlayer->pev->movetype == MOVETYPE_NOCLIP) ? "on" : "off" ), pPlayer );
			}
			else if ( FStrEq( pSetCommand, "explode" ) )
			{
				pPlayer->TakeDamage(pPlayer->pev, pPlayer->pev, 10000, DMG_ALWAYSGIB);
			}
			else if ( FStrEq( pSetCommand, "die_h" ) )
			{
				pPlayer->m_LastHitGroup = 1; // HITGROUP_HEAD
				pPlayer->TakeDamage(pPlayer->pev, pPlayer->pev, 10000, DMG_NEVERGIB);
				pPlayer->m_LastHitGroup = 0;
			}
		}
		return TRUE;
	}
	else if (FStrEq(pcmd, "vocalize"))
	{
		const char *szVoice = CMD_ARGV(1);
		PlayerVocalizeType nType = VOCALIZE_NONE;
		if ( szVoice && szVoice[0] )
		{
			if ( FStrEq( szVoice, "agree" ) ) nType = VOCALIZE_AGREE;
			else if ( FStrEq( szVoice, "decline" ) ) nType = VOCALIZE_DECLINE;
			else if ( FStrEq( szVoice, "cover" ) ) nType = VOCALIZE_COVER;
			else if ( FStrEq( szVoice, "need_ammo" ) ) nType = VOCALIZE_NEED_AMMO;
			else if ( FStrEq( szVoice, "need_weapon" ) ) nType = VOCALIZE_NEED_WEAPON;
			else if ( FStrEq( szVoice, "hold" ) ) nType = VOCALIZE_HOLD_HERE;
			else if ( FStrEq( szVoice, "fire" ) ) nType = VOCALIZE_OPEN_FIRE;
			else if ( FStrEq( szVoice, "taunt" ) ) nType = VOCALIZE_TAUNT;
		}
		if ( pPlayer->IsAlive() )
			pPlayer->DoVocalize( nType );
		return TRUE;
	}
#ifdef SCRIPT_SYSTEM
	else if (FStrEq(pcmd, "ss") || FStrEq(pcmd, "scriptsystem") || FStrEq(pcmd, "scall"))
	{
		const char *pSetCommand = CMD_ARGV(1);
		if ( FStrEq( pSetCommand, "func" ) )
		{
			UTIL_PrintConsole( "Available functions:\n", pPlayer );
			for ( int i = 0; i < IOFunctions_t::IO_ON_MAX; i++ )
				UTIL_PrintConsole( UTIL_VarArgs( "%s\n", IO_GetAvailableFunctions( (IOFunctions_t)i ) ), pPlayer );
			return TRUE;
		}
		else if ( FStrEq( pSetCommand, "cmds" ) )
		{
			UTIL_PrintConsole( "Available commands:\n", pPlayer );
			for ( int i = 0; i < IOFunctionCommands_t::IO_MAX; i++ )
				UTIL_PrintConsole( UTIL_VarArgs( "%s\n", IO_GetAvailableFunctionCommands( (IOFunctionCommands_t)i ) ), pPlayer );
			return TRUE;
		}
		else
		{
			UTIL_PrintConsole( "Available commands:\n", pPlayer );
			UTIL_PrintConsole( "scall func - Print all available functions\n", pPlayer );
			UTIL_PrintConsole( "scall cmds - Print all available commands\n", pPlayer );
		}
		return TRUE;
	}
#endif

	if ( m_pGameMode->IsTestModeActive() )
	{
		if (FStrEq(pcmd, "dev_spec"))
		{
			pPlayer->RemoveAllItems( FALSE );
			ChangePlayerTeam(pPlayer, ZP::Teams[ZP::TEAM_OBSERVER], FALSE, FALSE);
			pPlayer->StartObserver();
			return TRUE;
		}
		else if (FStrEq(pcmd, "dev_zombie"))
		{
			pPlayer->RemoveAllItems( FALSE );
			ChangePlayerTeam(pPlayer, ZP::Teams[ZP::TEAM_ZOMBIE], FALSE, FALSE);
			if ( pPlayer->IsObserver() )
				pPlayer->StopObserver();
			else
				pPlayer->StopWelcomeCam();
			// Give the player their arms.
			OnWeaponGive( pPlayer, "weapon_swipe" );
			return TRUE;
		}
		else if (FStrEq(pcmd, "dev_human"))
		{
			pPlayer->RemoveAllItems( FALSE );
			ChangePlayerTeam(pPlayer, ZP::Teams[ZP::TEAM_SURVIVIOR], FALSE, FALSE);
			if ( pPlayer->IsObserver() )
				pPlayer->StopObserver();
			else
				pPlayer->StopWelcomeCam();
			// No longer a late joiner since we forced to spawn as a human.
			pPlayer->m_bPunishLateJoiner = false;
			// Give the player their default weapons.
			m_pGameMode->GiveWeapons( pPlayer );
			return TRUE;
		}
		else if (FStrEq(pcmd, "dev_nolives"))
		{
			// Toggle the no lives.
			pPlayer->m_bNoLives = !pPlayer->m_bNoLives;
			return TRUE;
		}
		else if (FStrEq(pcmd, "test_ammoid"))
		{
			for ( int i = 0; i < ZPAmmoTypes::AMMO_MAX; i++ )
			{
				AmmoData ammo = GetAmmoByAmmoID( i );
				if ( !ammo.AmmoName ) continue;
				UTIL_SayText(UTIL_VarArgs( "%s ID: [%i]", ammo.AmmoName, ammo.AmmoType ), pPlayer);
			}
			return TRUE;
		}
	}
	return BaseClass::ClientCommand(pPlayer, pcmd);
}

void CZombiePanicGameRules::OnWeaponGive( CBasePlayer *pPlayer, const char *szItem )
{
	bool bIsCheatsEnabled = CVAR_GET_FLOAT("sv_cheats") >= 1 ? true : false;
	if ( !bIsCheatsEnabled ) return;

	bool bValid = false;
	if ( szItem && szItem[0] )
		bValid = true;

	if ( !bValid )
	{
		std::vector<WeaponInfo> found;

		// Iterate all weapons
		{
			for ( size_t i = 0; i < LAST_WEAPON_ID; i++ )
			{
				WeaponInfo weapon = GetWeaponInfo( (ZPWeaponID)i );
				if ( !weapon.szWeapon ) continue;
				if ( weapon.Hidden ) continue;
				found.push_back( weapon );
			}
		}

		// Sort array
		qsort(found.data(), found.size(), sizeof(WeaponInfo), [](const void *i, const void *j) -> int
		{
			const WeaponInfo *lhs = (const WeaponInfo *)i;
			const WeaponInfo *rhs = (const WeaponInfo *)j;
			return strcmp(lhs->szWeapon, rhs->szWeapon);
		});

		// Display results
		UTIL_PrintConsole( "Available weapons:\n", pPlayer );
		for (WeaponInfo& i : found)
			UTIL_PrintConsole( UTIL_VarArgs( "weapon_%s\n", i.szWeapon ), pPlayer );
		UTIL_PrintConsole( "--------------------\n", pPlayer );
		UTIL_PrintConsole( "Usage: give <string>\n", pPlayer );
		return;
	}

	pPlayer->GiveNamedItem( szItem );
}

extern void RemovePlayerLastSpawnPointData( CBasePlayer *pPlayer );

void CZombiePanicGameRules::ClientDisconnected(edict_t *pClient)
{
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pClient);
	if ( pPlayer )
	{
		RemovePlayerLastSpawnPointData( pPlayer );
		m_pGameMode->OnPlayerDisconnected( pPlayer );
	}

	BaseClass::ClientDisconnected( pClient );
}

void CZombiePanicGameRules::SetPlayerModel(CBasePlayer *pPlayer)
{
	// Moved to this function instead.
	pPlayer->SetTheCorrectPlayerModel();
}

//=========================================================
// ClientUserInfoChanged
//=========================================================
void CZombiePanicGameRules::ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer)
{
}

extern int gmsgDeathMsg;

//=========================================================
// Deathnotice.
//=========================================================
void CZombiePanicGameRules::DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor)
{
	if (m_DisableDeathMessages)
		return;

	if (pVictim && pKiller && pKiller->flags & FL_CLIENT)
	{
		CBasePlayer *pk = (CBasePlayer *)CBaseEntity::Instance(pKiller);
		if (pk && (pk != pVictim))
		{
			if ( (PlayerRelationship(pVictim, pk) == GR_TEAMMATE) )
				pVictim->m_iDeathFlags |= PLR_DEATH_FLAG_TEAMKILLER;
			else if ( pVictim->pev->team == ZP::TEAM_SURVIVIOR && pk->pev->team == ZP::TEAM_ZOMBIE )
			{
				int iPlayers = 0;
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );
					if ( plr && plr->IsConnected() && plr->pev->team == ZP::TEAM_SURVIVIOR )
						iPlayers++;
				}
				if ( iPlayers == 1 )
					pk->GiveAchievement( EAchievements::ZOMBIEDESSERT );
			}
		}
	}

	BaseClass::DeathNotice(pVictim, pKiller, pevInflictor);
}

//=========================================================
//=========================================================
void CZombiePanicGameRules ::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	if (!m_DisableDeathPenalty)
	{
		m_pGameMode->OnPlayerDied( pVictim, pKiller, pInflictor );
		BaseClass::PlayerKilled(pVictim, pKiller, pInflictor);

		CBasePlayer *pk = (CBasePlayer *)CBaseEntity::Instance(pKiller);
		if (pk && (pk != pVictim))
			pVictim->GiveAchievement( EAchievements::FIRST_TO_DIE );
	}
}

void CZombiePanicGameRules::PlayerThink(CBasePlayer *pPlayer)
{
	BaseClass::PlayerThink( pPlayer );
}

//=========================================================
// IsTeamplay
//=========================================================
BOOL CZombiePanicGameRules::IsTeamplay(void)
{
	return TRUE;
}

BOOL CZombiePanicGameRules::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pInflictor, CBaseEntity *pAttacker)
{
	// TEAM_NONE and TEAM_OBSERVER will never be damaged.
	if ( pPlayer->pev->team <= ZP::TEAM_OBSERVER ) return FALSE;

	// If valid attacker
	if ( pAttacker )
	{
		// We got ourselves, always return true.
		if ( pAttacker == pPlayer ) return TRUE;

		// If the attack was from an explosive,
		// and the one who threw it is dead,
		// don't cause any dmg to other survivors.
		if ( pInflictor && pInflictor->pev->team == ZP::TEAM_SURVIVIOR )
		{
			// If this is true, if we are a zombie player, always return true.
			bool bThrowerIsDeadOrNotSurvivor = !pAttacker->IsAlive();
			if ( pAttacker->pev->team != ZP::TEAM_SURVIVIOR )
				bThrowerIsDeadOrNotSurvivor = true;
			if ( bThrowerIsDeadOrNotSurvivor )
			{
				if ( pPlayer->pev->team == ZP::TEAM_SURVIVIOR ) return FALSE;
				return TRUE;
			}
		}

		// Player relationship stuff
		if ( PlayerRelationship(pPlayer, pAttacker) == GR_TEAMMATE )
		{
			// If friendly fire is off, and this hit came from someone other than myself, then don't get hurt.
			if ((friendlyfire.value == 0))
				return FALSE;
		}
	}

	// True by default.
	return TRUE;
}

//=========================================================
//=========================================================
int CZombiePanicGameRules::PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget)
{
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if (!pPlayer || !pTarget || !pPlayer->IsPlayer() || !pTarget->IsPlayer())
		return GR_NOTTEAMMATE;
	// Spectators are teammates, but not players in welcomecam mode
	if (((CBasePlayer *)pPlayer)->IsObserver() && !((CBasePlayer *)pPlayer)->m_bInWelcomeCam && ((CBasePlayer *)pTarget)->IsObserver() && !((CBasePlayer *)pTarget)->m_bInWelcomeCam)
		return GR_TEAMMATE;

	if ((*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !_stricmp(GetTeamID(pPlayer), GetTeamID(pTarget)))
	{
		return GR_TEAMMATE;
	}

	return GR_NOTTEAMMATE;
}

//=========================================================
//=========================================================
BOOL CZombiePanicGameRules::ShouldAutoAim(CBasePlayer *pPlayer, edict_t *target)
{
	// always autoaim, unless target is a teammate
	CBaseEntity *pTgt = CBaseEntity::Instance(target);
	if (pTgt && pTgt->IsPlayer())
	{
		if (PlayerRelationship(pPlayer, pTgt) == GR_TEAMMATE)
			return FALSE; // don't autoaim at teammates
	}

	return BaseClass::ShouldAutoAim(pPlayer, target);
}

//=========================================================
//=========================================================
int CZombiePanicGameRules::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	if (!pKilled)
		return 0;

	if (!pAttacker)
		return 1;

	if (pAttacker != pKilled && PlayerRelationship(pAttacker, pKilled) == GR_TEAMMATE)
		return -1;

	return 1;
}

//=========================================================
//=========================================================
const char *CZombiePanicGameRules::GetTeamID(CBaseEntity *pEntity)
{
	if (pEntity == NULL || pEntity->pev == NULL)
		return "";

	// return their team name
	return pEntity->TeamID();
}

int CZombiePanicGameRules::GetTeamIndex(const char *pTeamName)
{
	if (pTeamName && *pTeamName != 0)
	{
		// try to find existing team
		for (int tm = 0; tm < ZP::MAX_TEAM; tm++)
		{
			if (!_stricmp(ZP::Teams[tm], pTeamName))
				return tm;
		}
	}

	return -1; // No match
}

const char *CZombiePanicGameRules::GetIndexedTeamName(int teamIndex)
{
	if (teamIndex < 0 || teamIndex >= ZP::MAX_TEAM)
		return ZP::Teams[0];
	return ZP::Teams[teamIndex];
}

BOOL CZombiePanicGameRules::IsValidTeam(const char *pTeamName)
{
	return (GetTeamIndex(pTeamName) != -1) ? TRUE : FALSE;
}
