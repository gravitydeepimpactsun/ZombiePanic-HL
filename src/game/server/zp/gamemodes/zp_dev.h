// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_GAMEMODE_DEVMODE
#define SERVER_GAMEMODE_DEVMODE
#pragma once

class ZPGameMode_Dev : public IGameModeBase
{
public:
	ZPGameMode_Dev();

protected:
	virtual ZP::GameModeType_e GetGameModeType() { return ZP::GameModeType_e::GAMEMODE_NONE; }
	virtual bool IsTestModeActive() const { return true; };
	virtual void OnHUDInit(CBasePlayer *pPlayer);
	virtual void OnGameModeThink() {}
	virtual void RestartRound() {}
	virtual void OnPlayerDied( CBasePlayer *pPlayer, entvars_t *pKiller, entvars_t *pInflictor ) {}
	virtual void OnPlayerSpawned( CBasePlayer *pPlayer ) {}
	virtual void OnPlayerDisconnected( CBasePlayer *pPlayer ) {}
	virtual WinState_e GetWinState() { return WinState_e::State_None; }
	virtual bool WasAlreadyChoosenPreviously( CBasePlayer *pPlayer ) { return false; }
	virtual void ShouldClearChoosenZombies() {}
	virtual void ClearChoosenZombies() { }
	virtual void GiveWeapons(CBasePlayer *pPlayer) {}
};

#endif