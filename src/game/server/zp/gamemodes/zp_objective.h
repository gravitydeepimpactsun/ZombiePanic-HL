// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_GAMEMODE_OBJECTIVE
#define SERVER_GAMEMODE_OBJECTIVE
#pragma once

class ZPGameMode_Objective : public CBaseGameMode
{
	SET_BASECLASS( CBaseGameMode );
public:
	ZPGameMode_Objective();

protected:
	virtual GameModeType_e GetGameModeType() { return GameModeType_e::GM_OBJECTIVE; }
	virtual void OnHUDInit(CBasePlayer *pPlayer);
	virtual WinState_e GetWinState();
	virtual void SetWinState(WinState_e state) override
	{
		switch ( state )
		{
			case IGameModeBase::State_Draw:
			case IGameModeBase::State_ZombieWin: m_bTimeRanOut = true; break;
			case IGameModeBase::State_SurvivorWin: m_bHasPlayersReachedEnd = true; break;
		}
	}
	virtual void GiveWeaponsOnRoundStart() override;
	virtual void RestartRound();

	bool HasAnySurvivorEscaped();

private:
	bool m_bHasPlayersReachedEnd;
};

#endif