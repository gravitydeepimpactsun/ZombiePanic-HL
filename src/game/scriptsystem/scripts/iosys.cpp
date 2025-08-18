// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "core.h"
#include "iosys.h"

IOSystem::IOSystem()
{
	m_Functions.clear();
	// Make sure this is always a nullptr on creation.
	m_szScript = nullptr;
	// Make sure we add I/O system into our manager.
	ScriptSystem::AddToScriptManager( this );
}

void IOSystem::OnInit()
{
	// TODO: Add data here?
}

void IOSystem::OnCalled(pOnScriptCallbackReturn pfnCallback, KeyValues *pData, const std::string &szFunctionName)
{
	if ( !bAvailableToCall ) return;
	for (size_t i = 0; i < m_Functions.size(); i++)
	{
		IScriptFunctions ScriptFunction = m_Functions[i];
		if ( ScriptFunction.Function == szFunctionName )
		{
			if ( m_szScript ) m_szScript->OnCalled( szFunctionName, pData );
			if ( ScriptFunction.Callback ) (*ScriptFunction.Callback)(pData);
			if ( ScriptFunction.Entity )
			{
				CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ScriptFunction.Entity);
				if ( pEntity )
					pEntity->ScriptCallback( pData );
			}
			break;
		}
	}
	if ( pfnCallback )
		(*pfnCallback)(pData, GetScriptType());
}

void IOSystem::OnLevelInit(bool bPostLoad)
{
	if ( !bPostLoad )
	{
		bAvailableToCall = true;
		OnLoadMapScriptFile();
	}
}

void IOSystem::OnLevelShutdown()
{
	if ( m_szScript )
		delete m_szScript;
	m_szScript = nullptr;
	bAvailableToCall = false;
	// Clear our functions on shutdown
	m_Functions.clear();
}

void IOSystem::OnRegisterFunction(pOnScriptCallback pCallback, const std::string &szFunctionName)
{
	if ( FunctionAlreadyExist( szFunctionName ) ) return;
	IScriptFunctions ScriptFunction;
	ScriptFunction.Callback = pCallback;
	ScriptFunction.Entity = nullptr;
	ScriptFunction.Function = szFunctionName;
	m_Functions.push_back( ScriptFunction );
}

void IOSystem::OnRegisterFunction(CBaseEntity *pEntity, const std::string &szFunctionName)
{
	if ( FunctionAlreadyExist( szFunctionName ) ) return;
	IScriptFunctions ScriptFunction;
	ScriptFunction.Callback = nullptr;
	ScriptFunction.Entity = pEntity->edict();
	ScriptFunction.Function = szFunctionName;
	m_Functions.push_back( ScriptFunction );
}

void IOSystem::OnLoadMapScriptFile()
{
	// If we have a script already, get rid of it.
	if ( m_szScript )
		delete m_szScript;
	m_szScript = nullptr;
	// TODO: Load our map script file with STRING( gpGlobals->mapname )
}

IOScriptFile::IOScriptFile( const std::string &szFile )
{
	m_szFileName = szFile;
	// TODO: Setup our script file
}

IOScriptFile::~IOScriptFile()
{
	// TODO: Free our memory here
}

void IOScriptFile::OnCalled( const std::string &szFunction, KeyValues *pData )
{
	// Fire an output variable
	if ( !pData ) return;
	CBaseEntity *pEnt = nullptr;
	edict_t *pEdict = INDEXENT( atoi( pData->GetString( "arg0" ) ) );
	if ( pEdict && !pEdict->free )
		pEnt = CBaseEntity::Instance( pEdict );
	if ( pData->GetBool( "IsInput" ) )
		OnInput( pEnt, szFunction, pData->GetString( "arg1" ) );
	else
		OnOutput( pEnt, szFunction, pData->GetString( "arg1" ), atof( pData->GetString( "arg2" ) ) );
}

void IOScriptFile::OnOutput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue, const float &szDelay )
{
	// TODO: We called an output
}

void IOScriptFile::OnInput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue )
{
	// TODO: We called an input
}
