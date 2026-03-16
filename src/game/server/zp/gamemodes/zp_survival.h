// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_GAMEMODE_SURVIVAL
#define SERVER_GAMEMODE_SURVIVAL
#pragma once

class ZPGameMode_Survival : public CBaseGameMode
{
	SET_BASECLASS( CBaseGameMode );
public:
	ZPGameMode_Survival();

protected:
	virtual ZP::GameModeType_e GetGameModeType() { return ZP::GameModeType_e::GAMEMODE_SURVIVAL; }
	virtual void OnHUDInit(CBasePlayer *pPlayer);
	virtual void GetZombieLifeData( int &current, int &max ) override;
	virtual void OnZombieLifeUpdated( bool bIncreased ) override;
	virtual void OnPlayerDied( CBasePlayer *pPlayer, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void OnPlayerSpawned( CBasePlayer *pPlayer );
	virtual WinState_e GetWinState();
	virtual void RestartRound();
	bool HasNoRemainingZombies() const;
	void CalculateZombieLives();
	void OnRoundStateThink( ZP::RoundState state ) override;

	virtual void UpdateZombieLifesForClient();
	virtual void UpdateZombieLifesForClient( CBasePlayer *pPlayer );
	virtual void GiveWeaponsOnRoundStart();

	int m_iZombieLives;
};

#endif
