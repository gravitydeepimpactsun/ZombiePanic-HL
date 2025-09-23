#ifndef SERVER_TRIGGERS
#define SERVER_TRIGGERS
#pragma once

class CBaseTrigger : public CBaseToggle
{
public:
	void EXPORT TeleportTouch(CBaseEntity *pOther);
	void KeyValue(KeyValueData *pkvd);
	void EXPORT MultiTouch(CBaseEntity *pOther);
	void EXPORT HurtTouch(CBaseEntity *pOther);
	void EXPORT CDAudioTouch(CBaseEntity *pOther);
	void ActivateMultiTrigger(CBaseEntity *pActivator);
	void EXPORT MultiWaitOver(void);
	void EXPORT CounterUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT ToggleUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void InitTrigger(void);
	void OnScriptCallBack( KeyValues *pData );

	virtual int ObjectCaps(void) { return CBaseToggle ::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

#endif