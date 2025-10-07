// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_ZOMBIEPANIC_SPAWNPOINT_ENT
#define SERVER_ZOMBIEPANIC_SPAWNPOINT_ENT
#pragma once

class CBasePlayerSpawnPoint : public CPointEntity
{
public:
	void Spawn();
	void Restart();
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void OnScriptCallBack(KeyValues *pData);

	bool IsEnabled() { return m_bEnabled; }
	bool HasSpawned();

	void DisableSpawn();
	void SetOccupied(bool bOccupied);

private:
	bool m_bEnabled = true;
	bool m_bEnabledRem = true;
	bool m_bOccupied = false;
	float m_flLastOccupied = -1;
	float m_flDisableTime = -1;
};

#endif