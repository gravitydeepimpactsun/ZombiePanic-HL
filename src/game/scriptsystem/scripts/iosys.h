// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SCRIPT_SYSTEM_SCRIPT_IOSYSTEM
#define SCRIPT_SYSTEM_SCRIPT_IOSYSTEM
#pragma once

#include "script_interface.h"

enum IOFunctionCommands_t
{
	IO_INVALID = -1,
	IO_FIRE_OUTPUT = 0,
	IO_EXEC_AS,
	IO_WAIT,
	IO_IF,
	IO_END,
	IO_PRINT_TO_CONSOLE,
	IO_PRINT_TO_CHAT,

	IO_MAX
};

enum IOFunctions_t
{
	IO_ON_MAP_START = 0,
	IO_ON_ROUND_RESTART,
	IO_ON_MAP_END,
	IO_ON_PLAYER_SPAWNED,
	IO_ON_INPUT_RECEIVED,
	IO_ON_OUTPUT_SENT,

	IO_ON_MAX
};
const char *IO_GetAvailableFunctions( IOFunctions_t nFunc );
const char *IO_GetAvailableFunctionCommands( IOFunctionCommands_t nCommand );

struct IOFunctionCommand
{
	IOFunctionCommands_t Type; // The type of this command
	std::string Message; // The message of our command, used by PRINT_TO values and IO_FIRE_OUTPUT after our first argument value
	float Delay; // How long until we fire this? used by IO_WAIT
	std::string EntFire; // The entity we want to call
	std::string Input; // The input we want to call
	std::string Require; // The value we require to execute this
};

struct IOFunctionData
{
	std::string FunctionName; // The function name
	std::vector<std::string> Parameters; // The parameters it takes
	std::vector<IOFunctionCommand> Commands; // The commands it will execute
};

struct IOFunctionCall
{
	uint ID; // Our ID for this function call
	std::string InputCall; // Our function
	std::vector<std::string> Arguments; // Our arguments
	std::vector<IOFunctionCommand> Commands; // The commands it will execute
};

class IOScriptFile
{
public:
	IOScriptFile( const std::string &szFile );
	~IOScriptFile();

	void OnCalled( const std::string &szFunction, KeyValues *pData );
	void OnThink();
	void OnRoundRestart();

private:
	void OnOutput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue, const float &szDelay );
	void OnInput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue );

	// Let's call our function with no arguments.
	void CallData( const std::string &szFunction );

	// Let's call our function with variable arguments.
	void CallData( const std::string &szFunction, const std::string &szArgs );

	// Let's call specific functions
	void CallData( const IOFunctions_t &nFunction, const std::string &szArgs );

	// Runs the commands
	void RunCommands( int nID );

	std::string m_szFileName;
	// Our available functions
	std::vector<IOFunctionData> m_Functions;
	std::vector<IOFunctionCall> m_Commands;

	uint GetCurrentID() const;
};

class IOSystem : public IBaseScriptClass
{
public:
	IOSystem();

	AvailableScripts_t GetScriptType() { return AvailableScripts_t::InputOutput; }
	void OnInit();
	void OnThink();
	void OnCalled(pOnScriptCallbackReturn pfnCallback, KeyValues *pData, const std::string &szFunctionName);
	void OnLevelInit(bool bPostLoad);
	void OnLevelShutdown();
	void OnRoundRestart();
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
	inline bool FunctionAlreadyExist(CBaseEntity *pEnt, const std::string &szFunctionName)
	{
		for (size_t i = 0; i < m_Functions.size(); i++)
		{
			IScriptFunctions ScriptFunction = m_Functions[i];
			if ( ScriptFunction.Function == szFunctionName
				&& FClassnameIs( CBaseEntity::Instance( ScriptFunction.Entity )->pev, STRING( pEnt->pev->classname ) ) )
				return true;
		}
		return false;
	}
	std::vector<IScriptFunctions> m_Functions;
	bool bAvailableToCall = false;
	IOScriptFile *m_szScript = nullptr;
};

#endif
