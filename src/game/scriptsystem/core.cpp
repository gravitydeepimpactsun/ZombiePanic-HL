// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "core.h"

struct DelayedScriptCall
{
	AvailableScripts_t Type;
	pOnScriptCallbackReturn Callback;
	std::string FunctionName;
	std::vector<std::string> args;
	float Delay;
};
static std::vector<DelayedScriptCall> m_DelayedCalls;
static std::vector<IBaseScriptClass *> m_Scripts;

void ScriptSystem::ScriptFunctionCall(ScriptFunctionCall_t nCallType)
{
	for (size_t i = 0; i < m_Scripts.size(); i++)
	{
		IBaseScriptClass *pScript = m_Scripts[i];
		if ( pScript )
		{
			switch ( nCallType )
			{
				case ScriptFunctionCall_t::GAMEDLL_INIT: pScript->OnInit(); break;
				case ScriptFunctionCall_t::LEVEL_INIT: pScript->OnLevelInit( false ); break;
				case ScriptFunctionCall_t::POST_LEVEL_INIT: pScript->OnLevelInit( true ); break;
				case ScriptFunctionCall_t::LEVEL_SHUTDOWN: pScript->OnLevelShutdown(); break;
			}
		}
	}
}

void ScriptSystem::OnShutdown()
{
	m_Scripts.clear();
}

void ScriptSystem::OnThink()
{
	for ( size_t i = 0; i < m_DelayedCalls.size(); i++ )
	{
		DelayedScriptCall caller = m_DelayedCalls[i];
		if ( gpGlobals->time > caller.Delay )
		{
			CallScriptArray( caller.Type, caller.Callback, caller.FunctionName, caller.args );
			m_DelayedCalls.erase( m_DelayedCalls.begin() + i );
			break;
		}
	}
}

bool ScriptSystem::AddToScriptManager(IBaseScriptClass *pScript)
{
	for (size_t i = 0; i < m_Scripts.size(); i++)
	{
		if ( pScript == m_Scripts[i] )
			return false;
	}
	m_Scripts.push_back( pScript );
	return true;
}

ScriptSystem::ScriptCallBackEnum ScriptSystem::CallScript(AvailableScripts_t nType, pOnScriptCallbackReturn pCallback, std::string szFunctionName, int iNumArgs, ...)
{
	ScriptCallBackEnum nRet = ScriptCall_OK;
	for (size_t i = 0; i < m_Scripts.size(); i++)
	{
		IBaseScriptClass *pScript = m_Scripts[i];
		if ( pScript && pScript->GetScriptType() == nType )
		{
			KeyValuesAD pItems( "Items" );

			va_list argptr;
			va_start(argptr, iNumArgs);
			// Go trough the list, and add them as "arg0" etc.
			for ( int y = 0; y < iNumArgs; y++ )
			{
				std::string arg = va_arg(argptr, std::string);
				pItems->SetString( UTIL_VarArgs( "arg%i", y ), arg.c_str() );
			}
			va_end(argptr);

			pScript->OnCalled( pCallback, pItems, szFunctionName );
		}
	}

	return nRet;
}

ScriptSystem::ScriptCallBackEnum ScriptSystem::CallScriptArray(AvailableScripts_t nType, pOnScriptCallbackReturn pCallback, std::string szFunctionName, std::vector<std::string> nArray)
{
	ScriptCallBackEnum nRet = ScriptCall_OK;
	for (size_t i = 0; i < m_Scripts.size(); i++)
	{
		IBaseScriptClass *pScript = m_Scripts[i];
		if ( pScript && pScript->GetScriptType() == nType )
		{
			KeyValuesAD pItems( "Items" );

			// Go trough the list, and add them as "arg0" etc.
			for ( size_t y = 0; y < nArray.size(); y++ )
				pItems->SetString( UTIL_VarArgs( "arg%i", y ), nArray[y].c_str() );

			pScript->OnCalled( pCallback, pItems, szFunctionName );
		}
	}
	return nRet;
}

void ScriptSystem::CallScriptDelay(AvailableScripts_t nType, pOnScriptCallbackReturn pCallback, std::string szFunctionName, float flDelay, int iNumArgs, ...)
{
	DelayedScriptCall caller;
	caller.Type = nType;
	caller.Callback = pCallback;
	caller.FunctionName = szFunctionName;
	caller.Delay = gpGlobals->time + flDelay;
	caller.args.clear();
	va_list argptr;
	va_start( argptr, iNumArgs );
	// Go trough the list, and add them as "arg0" etc.
	for ( int i = 0; i < iNumArgs; i++ )
	{
		std::string arg = va_arg(argptr, std::string);
		caller.args.push_back( arg );
	}
	va_end( argptr );
	m_DelayedCalls.push_back( caller );
}

void ScriptSystem::RegisterScriptCallback(AvailableScripts_t nType, pOnScriptCallback pCallback, std::string szFunctionName)
{
	for (size_t i = 0; i < m_Scripts.size(); i++)
	{
		IBaseScriptClass *pScript = m_Scripts[i];
		if ( pScript && pScript->GetScriptType() == nType )
			pScript->OnRegisterFunction( pCallback, szFunctionName );
	}
}

void ScriptSystem::RegisterScriptCallback(AvailableScripts_t nType, CBaseEntity *pEntity, std::string szFunctionName)
{
	for (size_t i = 0; i < m_Scripts.size(); i++)
	{
		IBaseScriptClass *pScript = m_Scripts[i];
		if ( pScript && pScript->GetScriptType() == nType )
			pScript->OnRegisterFunction( pEntity, szFunctionName );
	}
}
