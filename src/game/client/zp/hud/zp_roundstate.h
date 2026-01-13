// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef HUD_ROUNDSTATE_H
#define HUD_ROUNDSTATE_H
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include "../../hud/base.h"
#include "zp/zp_shared.h"

class CHudRoundState : public CHudElemBase<CHudRoundState>, public vgui2::Panel
{
public:
	DECLARE_CLASS_SIMPLE(CHudRoundState, vgui2::Panel);

	CHudRoundState();

	virtual bool IsAllowedToDraw();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

	int MsgFunc_RoundState(const char *pszName, int iSize, void *pbuf);
	void SetRoundState( ZP::RoundState iState, int iWinner );
	void PlayAudio( const char *szAudio );

private:
	CPanelAnimationVarAliasType( int, m_iWaitingForPlayersYPos, "WaitingForPlayers_yPos", "100", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iWaitingForPlayersTall, "WaitingForPlayers_Tall", "30", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextWaitingForPlayersTall, "WaitingForPlayers_TextTall", "20", "proportional_int" );
	CPanelAnimationStringVar( 32, m_szWaitingForPlayersFont, "WaitingForPlayers_Font", "Default" );


	CPanelAnimationVarAliasType( int, m_iRoundIsOverYPos, "RoundIsOver_yPos", "225", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iRoundIsOverTall, "RoundIsOver_Tall", "30", "proportional_int" );
	CPanelAnimationStringVar( 32, m_szRoundIsOverFont, "RoundIsOver_Font", "ZPTitle" );

	vgui2::Label *m_pText;
};

#endif
