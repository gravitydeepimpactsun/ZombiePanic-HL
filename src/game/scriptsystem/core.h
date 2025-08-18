// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SCRIPT_SYSTEM_CORE
#define SCRIPT_SYSTEM_CORE
#pragma once

#include <string>       // std::string
#include <KeyValues.h>  // KeyValues

// Our script interface
#include "script_interface.h"

// Input/Output System
#include "scripts/iosys.h"

// Angelscript Support
#if defined( SCRIPT_ANGELSCRIPT )
#include "scripts/angelscript.h"
#endif

namespace ScriptSystem
{
    enum ScriptFunctionCall_t
    {
        GAMEDLL_INIT = 0,
        LEVEL_INIT,
        POST_LEVEL_INIT,
        LEVEL_SHUTDOWN
    };

    /// <summary>
    /// Calls our scripts to do certain actions
    /// </summary>
    /// <param name="nCallType">What type we want our scripts to call</param>
    void ScriptFunctionCall( ScriptFunctionCall_t nCallType );

    /// <summary>
    /// When we are shutting down the game
    /// </summary>
    void OnShutdown();

    /// <summary>
    /// Our simply think system, where we simply check when to call our I/O if we have a delay on them.
    /// </summary>
    void OnThink();

    /// <summary>
    /// Our callback enum states, used by pOnScriptCallbackReturn
    /// </summary>
    enum ScriptCallBackEnum
    {
        ScriptCall_OK = 0,
        ScriptCall_Warning,
        ScriptCall_Error
    };

    /// <summary>
    /// Add our script to our manager
    /// </summary>
    /// <param name="pScript">Our script object class</param>
    /// <returns>Returns true if our script was not added before.</returns>
    bool AddToScriptManager( IBaseScriptClass *pScript );

    /// <summary>
    /// Calls our script through our script system manager.
    /// </summary>
    /// <param name="nType">The specific type of script we want to call</param>
    /// <param name="pCallback">The callback function from this call</param>
    /// <param name="szFunctionName">The function we want to call</param>
    /// <param name="iNumArgs">The amount of arguments we need to use</param>
    /// <param name="...">Our functions we want to call in our KeyValue data</param>
    /// <returns>Returns the script callback state</returns>
    ScriptCallBackEnum CallScript(AvailableScripts_t nType, pOnScriptCallbackReturn pCallback, std::string szFunctionName, int iNumArgs, ...);

    /// <summary>
    /// Calls our script through our script system manager.
    /// </summary>
    /// <param name="nType">The specific type of script we want to call</param>
    /// <param name="pCallback">The callback function from this call</param>
    /// <param name="szFunctionName">The function we want to call</param>
    /// <param name="flDelay">The amount of seconds we should wait until we fire this function</param>
    /// <param name="iNumArgs">The amount of arguments we need to use</param>
    /// <param name="...">Our functions we want to call in our KeyValue data</param>
    /// <returns>Returns the script callback state</returns>
    void CallScriptDelay(AvailableScripts_t nType, pOnScriptCallbackReturn pCallback, std::string szFunctionName, float flDelay, int iNumArgs, ...);

    /// <summary>
    /// Register a default callback for our C++ code
    /// </summary>
    /// <param name="nType">The specific type of script we want our callback to register to</param>
    /// <param name="pCallback">Our callback object</param>
    /// <param name="szFunctionName">The name of our function</param>
    void RegisterScriptCallback(AvailableScripts_t nType, pOnScriptCallback pCallback, std::string szFunctionName);

    /// <summary>
    /// Register a default callback for our C++ code
    /// </summary>
    /// <param name="nType">The specific type of script we want our callback to register to</param>
    /// <param name="pEntity">Our callback entity</param>
    /// <param name="szFunctionName">The name of our function</param>
    void RegisterScriptCallback(AvailableScripts_t nType, CBaseEntity *pEntity, std::string szFunctionName);
}

#endif
