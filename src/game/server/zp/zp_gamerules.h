// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_ZOMBIEPANIC_GAMERULES
#define SERVER_ZOMBIEPANIC_GAMERULES
#pragma once

#include "cdll_dll.h"
#include "zp/zp_shared.h"
#include "zp/zp_apicallback.h"
#include "zp/gamemodes/zp_gamemodebase.h"
#include "zp/gamemodes/zp_dev.h"
#include "zp/gamemodes/zp_survival.h"
#include "zp/gamemodes/zp_objective.h"
#include "zp/gamemodes/zp_hardcore.h"
#include "steam/steam_api.h"

class CZombiePanicGameRules : public CHalfLifeMultiplay
{
	SET_BASECLASS( CHalfLifeMultiplay );
public:
	CZombiePanicGameRules();
	~CZombiePanicGameRules();

	virtual void ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer);
	virtual BOOL IsTeamplay(void);
	virtual BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pInflictor, CBaseEntity *pAttacker);
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget);
	virtual const char *GetTeamID(CBaseEntity *pEntity);
	virtual BOOL ShouldAutoAim(CBasePlayer *pPlayer, edict_t *target);
	virtual int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);
	virtual void InitHUD(CBasePlayer *pl);
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor);
	virtual const char *GetGameDescription(void) { return "Zombie Panic! v1.1"; } // this is the game name that gets seen in the server browser
	int WeaponShouldRespawn(CBasePlayerItem *pWeapon) override { return GR_WEAPON_RESPAWN_NO; }
	int ItemShouldRespawn(CItem *pItem) override { return GR_ITEM_RESPAWN_NO; }
	int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo) override { return GR_AMMO_RESPAWN_NO; }
	virtual void UpdateGameMode(CBasePlayer *pPlayer); // the client needs to be informed of the current game mode
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual void Think(void);
	virtual int GetTeamIndex(const char *pTeamName);
	virtual const char *GetIndexedTeamName(int teamIndex);
	virtual BOOL IsValidTeam(const char *pTeamName);
	const char *SetDefaultPlayerTeam(CBasePlayer *pPlayer);
	virtual void ChangePlayerTeam(CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib);
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer);

	virtual void ResetRound();
	virtual void CleanUpMap();

	virtual void ResetVolunteers();
	virtual void PickRandomVolunteer();

	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual BOOL ClientCommand(CBasePlayer *pPlayer, const char *pcmd);

	void OnWeaponGive( CBasePlayer *pPlayer, const char *szItem );

	bool DownloadMissingWorkshopItem( edict_t *pClient );
	BOOL ClientConnected(edict_t *pClient, const char *pszName, const char *pszAddress, char szRejectReason[128]) override;
	void ClientDisconnected(edict_t *pClient) override;

	BOOL FAllowMonsters( void ) override { return FALSE; }

	bool WasCheatsOnThisSession() const;
	inline void SetCheatsOnThisSession( bool bCheats ) { m_bCheatsOnThisSession = bCheats; }
	void EndMultiplayerGame( void ) override;

	void DoAPICallBack( CBasePlayer *pPlayer );

protected:
	void ProcessAPICalls();
	std::vector<int> m_vecPendingAPICalls;
	float m_flNextAPICallTime;

private:
	void CheckCheats();
	void SetPlayerModel(CBasePlayer *pPlayer);

	BOOL m_DisableDeathMessages;
	BOOL m_DisableDeathPenalty;

	bool m_bHasPickedVolunteer;
	ZP::GameModeType_e m_GameModeType;
	IGameModeBase *m_pGameMode;
	std::vector<int> m_Volunteers;
	float m_flRoundRestartDelay;
	float m_flRoundJustBegun;
	int m_iRounds;
	bool m_bCheatsOnThisSession;
	bool m_bHostHasJoined; // Peer-2-Peer only

	ClientAPIData_t m_ClientsData[ MAX_PLAYERS ] = {};
};
CZombiePanicGameRules *ZPGameRules();

#endif