// Martin Webrant (BulliT)

#ifndef HUD_AG_PLAYERID_H
#define HUD_AG_PLAYERID_H
#include "hud/base.h"

class AgHudPlayerId : public CHudElemBase<AgHudPlayerId>
{
public:
	AgHudPlayerId()
	    : m_iPlayer(0)
	    , m_iHealth(0)
	    , m_iArmour(0)
	{
	}

	void Init() override;
	void VidInit() override;
	void Draw(float flTime) override;
	void Reset() override;

	void DrawSurvivorID( CPlayerInfo *pInfo );
	void DrawZombieID( CPlayerInfo *pInfo );
	void DrawText( const char *pszText, const int &ypos, const Color &rgb );
	void SetPlayerID( int iPlayerID, int iHealth, int iArmor );

private:
	int m_iPlayer;
	int m_iHealth;
	int m_iArmour;
};

#endif
