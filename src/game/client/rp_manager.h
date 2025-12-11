// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef RICH_PRESENCE_MANAGER_H
#define RICH_PRESENCE_MANAGER_H

class CRichPresenceManager
{
public:
	CRichPresenceManager();

	static CRichPresenceManager *GetInstance();
	void UpdatePresence();
	void Reset();

private:
	char m_szLevelName[64];
	int m_iCurrentRounds;
};

#endif