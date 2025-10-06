// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "zp/zp_shared.h"

#include "zp_gamemodebase.h"
#include "zp_dev.h"


ZPGameMode_Dev::ZPGameMode_Dev()
{
	SetRoundState( ZP::RoundState_RoundHasBegun );
}

void ZPGameMode_Dev::OnHUDInit(CBasePlayer *pPlayer)
{
	if ( !pPlayer ) return;
	if ( !pPlayer->IsConnected() ) return;
	MESSAGE_BEGIN(MSG_ONE, gmsgRoundState, NULL, pPlayer->edict());
	WRITE_SHORT(GetRoundState());
	MESSAGE_END();
}
