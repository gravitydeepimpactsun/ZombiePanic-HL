// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SERVER_RANDOM_ENTITY_BASE
#define SERVER_RANDOM_ENTITY_BASE
#pragma once

#include "cbase.h"
#include "zp/zp_shared.h"

#define OBJECTIVE_MESSAGE_FIRST (1<<0)
#define OBJECTIVE_DONT_USE_ALT (1<<1)

class CObjectiveMessage : public CPointEntity
{
	SET_BASECLASS( CPointEntity );

public:
	void Spawn( void );
	void Restart();
	void KeyValue( KeyValueData *pkvd ) override;
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void UpdateMessageState();
	void CallNewObjective();
	bool IsFirstObjective();
	virtual int ObjectCaps(void) { return CBaseEntity::ObjectCaps() | FCAP_MUST_RESET; }
	void OnScriptCallBack( KeyValues *pData );

private:
	ObjectiveState m_State;
	string_t m_NextObj = 0;
	string_t m_AltObj = 0;
	string_t m_ZombieText = 0;
};

class CObjectiveMessageSet : public CPointEntity
{
	SET_BASECLASS( CPointEntity );

public:
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

private:
	ObjectiveState m_State = ObjectiveState::State_Normal;
};

#endif