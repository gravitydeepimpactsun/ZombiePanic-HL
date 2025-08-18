// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SCRIPT_SYSTEM_SCRIPT_IOSYSTEM
#define SCRIPT_SYSTEM_SCRIPT_IOSYSTEM
#pragma once

#include "script_interface.h"

class IOScriptFile
{
public:
	IOScriptFile( const std::string &szFile );
	~IOScriptFile();

	void OnCalled( const std::string &szFunction, KeyValues *pData );

private:
	void OnOutput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue, const float &szDelay );
	void OnInput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue );

	std::string m_szFileName;
	// TODO: Add data
};

class IOSystem : public IBaseScriptClass
{
	IOSystem();

public:
	AvailableScripts_t GetScriptType() { return AvailableScripts_t::InputOutput; }
	void OnInit();
	void OnCalled(pOnScriptCallbackReturn pfnCallback, KeyValues *pData, const std::string &szFunctionName);
	void OnLevelInit(bool bPostLoad);
	void OnLevelShutdown();
	void OnRegisterFunction(pOnScriptCallback pCallback, const std::string &szFunctionName);
	void OnRegisterFunction(CBaseEntity *pEntity, const std::string &szFunctionName);
	void OnLoadMapScriptFile();

private:
	inline bool FunctionAlreadyExist(const std::string &szFunctionName)
	{
		for (size_t i = 0; i < m_Functions.size(); i++)
		{
			IScriptFunctions ScriptFunction = m_Functions[i];
			if ( ScriptFunction.Function == szFunctionName )
				return true;
		}
		return false;
	}
	std::vector<IScriptFunctions> m_Functions;
	bool bAvailableToCall = false;
	IOScriptFile *m_szScript = nullptr;
};

#endif
