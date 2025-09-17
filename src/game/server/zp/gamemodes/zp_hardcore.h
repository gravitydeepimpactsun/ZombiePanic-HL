// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_GAMEMODE_HARDCORE
#define SERVER_GAMEMODE_HARDCORE
#pragma once

#include "zp_survival.h"

class ZPGameMode_HardCore : public ZPGameMode_Survival
{
	SET_BASECLASS( ZPGameMode_Survival );
public:
	ZPGameMode_HardCore();

protected:
	virtual void RestartRound();
	virtual GameModeType_e GetGameModeType() { return GameModeType_e::GM_HARDCORE; }
	virtual void GiveWeapons(CBasePlayer *pPlayer);
	virtual void GiveWeaponsOnRoundStart();
};

#endif