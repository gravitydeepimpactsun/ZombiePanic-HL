// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef SCRIPT_SYSTEM_SCRIPT_INTERFACE
#define SCRIPT_SYSTEM_SCRIPT_INTERFACE
#pragma once

// Our available scripts to register to
enum AvailableScripts_t
{
	InputOutput = 0,
	Angelscript,
};
using pOnScriptCallback = void (*)(KeyValues *pData);
using pOnScriptCallbackReturn = void (*)(KeyValues *pData, AvailableScripts_t nScript);

struct IScriptFunctions
{
	pOnScriptCallback Callback;
	edict_t *Entity;
	std::string Function;
};

class IBaseScriptClass
{
public:
	/// <summary>
	/// Our script type.
	/// </summary>
	/// <returns>The script type of choice</returns>
	virtual AvailableScripts_t GetScriptType() = 0;

	/// <summary>
	/// When our script is being called,
	/// we check our function name before sending our data further.
	/// </summary>
	/// <param name="pfnCallback">Our callback function</param>
	/// <param name="pData">Our data in keyvalues</param>
	/// <param name="szFunctionName">Our function we want to call</param>
	virtual void OnCalled(pOnScriptCallbackReturn pfnCallback, KeyValues *pData, const std::string &szFunctionName) = 0;

	/// <summary>
	/// Fires when our script class is created and added into our ScriptSystem manager.
	/// </summary>
	virtual void OnInit() = 0;

	/// <summary>
	/// Executes on each frame
	/// </summary>
	virtual void OnThink() = 0;

	/// <summary>
	/// When we are loading our level
	/// </summary>
	/// <param name="bPostLoad">If true, then we fired after loading our level</param>
	virtual void OnLevelInit(bool bPostLoad) = 0;

	/// <summary>
	/// When the level is shutting down
	/// </summary>
	virtual void OnLevelShutdown() = 0;

	/// <summary>
	/// This will trigger when ScriptSystem::RegisterScriptCallback is fired,
	/// if the specified script is called.
	/// </summary>
	/// <param name="pfnCallback">Our callback function</param>
	/// <param name="szFunctionName">The function name that this is tied to</param>
	virtual void OnRegisterFunction(pOnScriptCallback pfnCallback, const std::string &szFunctionName) = 0;

	/// <summary>
	/// This will trigger when ScriptSystem::RegisterScriptCallback is fired,
	/// if the specified script is called.
	/// </summary>
	/// <param name="pEntity">Our entity we will call</param>
	/// <param name="szFunctionName">The function name that this is tied to</param>
	virtual void OnRegisterFunction(CBaseEntity *pEntity, const std::string &szFunctionName) = 0;
};

#endif
