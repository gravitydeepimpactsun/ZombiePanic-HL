// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "core.h"
#include "iosys.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <tier2/tier2.h>
#include "FileSystem.h"
#include <KeyValues.h>
#include <convar.h>

// For giving stuff for our players.
#include "player.h"

#include "zp/info_random_base.h"

static ConVar sv_ss_debug( "sv_ss_debug", "0", FCVAR_CHEATS );

// A simple static list of our I/O IOFunctions_t
static const char *g_IOFunctions[IO_ON_MAX] = {
	"OnMapStart",
	"OnRoundStart",
	"OnMapEnd",
	"OnPlayerSpawned",
	"OnInputReceived",
	"OnOutputSent"
};

// Same as above, a static list of our I/O IOFunctionCommands_t
static const char *g_IOCommands[IO_MAX] = {
	"FireOutput",
	"ASCall",
	"Wait",
	"Break",
	"if",
	"elseif",
	"else",
	"end",
	"PrintToConsole",
	"PrintToChat",
	"GiveItem",
	"Participated",
	"AddToSpawnList",
	"SpawnItems",
};

static std::vector<ISpawnListData> s_SpawnListData; // Used by IO_SPAWN_ITEMS and IO_ADD_TO_SPAWN_LIST

static std::string MessageCleanup( const std::string &str )
{
	const char *ws = "\"";
	size_t start = str.find_first_not_of(ws);
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(ws);
	std::string output = str.substr(start, end - start + 1);
	// Now, we check if we have a remaining " at the end, if we do, get rid of it.
	size_t last = output.find_last_of(ws);
	if (last == std::string::npos)
		return output;
	return output.substr(start, last - 1);
}

static std::string Trim( const std::string &str )
{
	const char *ws = " \t\n\r";
	size_t start = str.find_first_not_of(ws);
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(ws);
	return str.substr(start, end - start + 1);
}

static std::vector<std::string> Split( const std::string &s, char delimiter )
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(Trim(token));
	return tokens;
}

static std::vector<std::string> SplitQoutes( const std::string &s, char delimiter )
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while ( tokenStream >> std::quoted(token) )
		tokens.push_back( Trim(token) );
	return tokens;
}

static std::string GetArgValues(const std::string &message, const std::vector<std::string> &params)
{
	std::string result = message;
	for (size_t i = 0; i < params.size(); ++i)
	{
		std::string replacement = params[i];
		std::string pattern = "%s" + std::to_string(i) + "%";
		size_t pos = 0;
		while ((pos = result.find(pattern, pos)) != std::string::npos)
		{
			result.replace(pos, pattern.length(), replacement);
			pos += replacement.length();
		}
	}
	return result;
}

static std::string ReplaceScriptArgs(const std::string &message, const std::vector<std::string> &params)
{
	std::string result = message;
	for (size_t i = 0; i < params.size(); ++i)
	{
		std::string pattern = "{" + params[i] + "}";
		std::string replacement = "%s" + std::to_string(i) + "%";
		size_t pos = 0;
		while ((pos = result.find(pattern, pos)) != std::string::npos)
		{
			result.replace(pos, pattern.length(), replacement);
			pos += replacement.length();
		}
	}
	return result;
}

IOSystem::IOSystem()
{
	m_Functions.clear();
	// Make sure this is always a nullptr on creation.
	m_szScript = nullptr;
	// Make sure we add I/O system into our manager.
	ScriptSystem::AddToScriptManager( this );
}

// Does nothing.
static void DefaultGenericCallback( KeyValues * ) {}

void IOSystem::OnInit()
{
	// Default callbacks
	for ( size_t i = 0; i < IO_ON_MAX; i++ )
		ScriptSystem::RegisterScriptCallback( AvailableScripts_t::InputOutput, DefaultGenericCallback, g_IOFunctions[i] );
}

void IOSystem::OnThink()
{
	if ( !m_szScript ) return;
	m_szScript->OnThink();
}

ScriptCallBackEnum IOSystem::OnCalled(pOnScriptCallbackReturn pfnCallback, KeyValues *pData, const std::string &szFunctionName)
{
	if ( !bAvailableToCall ) return ScriptCall_Warning;
	ScriptCallBackEnum nRet = ScriptCall_Error;
	bool bIsScriptCall = FStrEq( pData->GetString( "arg0" ), "SCall" );
	for (size_t i = 0; i < m_Functions.size(); i++)
	{
		IScriptFunctions ScriptFunction = m_Functions[i];
		if ( ScriptFunction.Function == szFunctionName )
		{
			// Only call once.
			if ( m_szScript && !bIsScriptCall )
			{
				nRet = m_szScript->OnCalled( szFunctionName, pData );
				bIsScriptCall = true;
			}
			else
			{
				// We found our function, return OK
				nRet = ScriptCall_OK;
			}
			if ( ScriptFunction.Callback ) (*ScriptFunction.Callback)(pData);
			if ( ScriptFunction.Entity )
			{
				if ( bIsScriptCall )
				{
					if ( sv_ss_debug.GetBool() )
					{
						Msg( "Action: %s\n", szFunctionName.c_str() );
						Msg( "Argument 1: %s\n", pData->GetString( "arg1" ) );
						Msg( "Argument 2: %s\n", pData->GetString( "arg2" ) );
					}

					// Make sure we call the right entity, if not, ignore.
					CBaseEntity *pFind = UTIL_FindEntityByTargetname( nullptr, pData->GetString( "arg1" ) );
					while ( pFind )
					{
						KeyValues *pKvNew = new KeyValues( "Items" );
						pKvNew->SetString( "Action", szFunctionName.c_str() );
						pKvNew->SetString( "arg0", pData->GetString( "arg2" ) );
						pFind->ScriptCallback( pKvNew );
						pKvNew->deleteThis();
						pFind = UTIL_FindEntityByTargetname( pFind, pData->GetString( "arg1" ) );
					}
					break;
				}
				else
				{
					CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE( ScriptFunction.Entity );
					if ( pEntity && pEntity->entindex() == atoi( pData->GetString( "arg0" ) ) )
					{
						if ( sv_ss_debug.GetBool() )
						{
							Msg( "Entity: %s\n", pData->GetString( "arg0" ) );
							Msg( "Action: %s\n", szFunctionName.c_str() );
							Msg( "Argument 1: %s\n", pData->GetString( "arg1" ) );
							Msg( "Argument 2: %s\n", pData->GetString( "arg2" ) );
						}

						KeyValues *pKvNew = new KeyValues( "Items" );
						pKvNew->SetString( "Action", szFunctionName.c_str() );
						pKvNew->SetString( "arg0", pData->GetString( "arg1" ) );
						pEntity->ScriptCallback( pKvNew );
						pKvNew->deleteThis();
						break;
					}
				}
			}
		}
	}
	if ( pfnCallback )
		(*pfnCallback)(pData, GetScriptType());
	return nRet;
}

void IOSystem::OnLevelInit( bool bPostLoad )
{
	if ( !bPostLoad )
	{
		bAvailableToCall = true;
		OnLoadMapScriptFile();
		ScriptSystem::CallScriptDelay( AvailableScripts_t::InputOutput, nullptr, g_IOFunctions[ IO_ON_MAP_START ], 5.0f, 0 );
	}
}

void IOSystem::OnLevelShutdown()
{
	ScriptSystem::CallScript( AvailableScripts_t::InputOutput, nullptr, g_IOFunctions[ IO_ON_MAP_END ], 0 );
	if ( m_szScript )
		delete m_szScript;
	m_szScript = nullptr;
	bAvailableToCall = false;
	// Clear our functions on shutdown
	m_Functions.clear();
}

void IOSystem::OnRoundRestart()
{
	if ( m_szScript )
		m_szScript->OnRoundRestart();
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
	if ( FunctionAlreadyExist( pEntity, szFunctionName ) ) return;
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

	// Load our map script file.
	m_szScript = new IOScriptFile( "scripts/maps/" + std::string( STRING( gpGlobals->mapname ) ) + ".io" );
}

IOScriptFile::IOScriptFile( const std::string &szFile )
{
	// Let's parse our file here.
	// This is how we will parse our I/O file.
	/*
	// An example of a function callback (with no paramaters), an empty () works as well:
	Function OnMapStart
	{
		// Our map started!
		// Fire's an output to an entity.
		// Arguments:
		// arg0 = Entity targetname
		// arg1 = Input name
		// arg2 = Value
		// arg3 = Delay
		// Example:
		FireOutput "my_target" "MyInput" "MyValue" 5.0
	}

	Function OnMapEnd()
	{
		// Our map ended!
		// Fire's an output to an entity.
		// Arguments:
		// arg0 = Entity targetname
		// arg1 = Input name
		// arg2 = Value
		// arg3 = Delay
		// Example:
		FireOutput "my_target" "MyInput" "MyValue" 0.0
	}

	// An example of a function callback (with paramaters):
	Function OnPlayerSpawned( myInput )
	{
		// Our player spawned!
		// Fire's an output to an entity.
		// Arguments:
		// arg0 = Entity targetname
		// arg1 = Input name
		// arg2 = Value
		// arg3 = Delay
		// Example:
		FireOutput "my_target" "MyInput" myInput 0.0
	}

	// An example of input being recieved:
	Function OnInputReceived( EntityID, InputName, InputValue )
	{
		// We recieved an input!
		// Arguments:
		// arg0 = Entity ID (entindex number)
		// arg1 = Input name
		// arg2 = Value
		// Example:
		PrintToConsole "We recieved an input from entity: {EntityID} with input name: {InputName} and value: {InputValue}"
	}

	// An example of output being sent:
	Function OnOutputSent( EntityID, OutputName, OutputValue, OutputDelay )
	{
		// We sent an output!
		// Arguments:
		// arg0 = Entity ID (entindex number)
		// arg1 = Output name
		// arg2 = Value
		// arg3 = Delay
		// Example:
		PrintToConsole "We sent an output to entity: {EntityID} with output name: {OutputName} and value: {OutputValue} after a delay of: {OutputDelay}"
	}
	*/

	// Let's parse our file.
	m_szFileName = szFile;
	m_Functions.clear();
	m_Commands.clear();

	// Make sure we get the full path to file!
	char szFullPath[1024];
	g_pFullFileSystem->GetLocalPath( szFile.c_str(), szFullPath, sizeof( szFullPath ) );

	// Open file
	std::ifstream file( szFullPath );
	if ( !file.is_open() )
		return;

	std::string line;
	IOFunctionData currentFunction;
	bool inFunction = false;
	bool inBlock = false;
	bool inElse = false;
	bool inElseIf = false;
	std::string inIf;
	IORequirementStatements currentRequirement = IORequirementStatements::IF_EQUAL;

	while ( std::getline(file, line) )
	{
		line = Trim(line);
		if (line.empty() || line[0] == '/' || line[0] == '*')
			continue; // Skip comments and empty lines

		// Function header
		if (line.find("Function ") == 0)
		{
			if (inFunction && !currentFunction.FunctionName.empty())
			{
				m_Functions.push_back(currentFunction);
				currentFunction = IOFunctionData();
			}
			inFunction = true;
			inBlock = false;
			inIf.clear();
			inElse = false;
			inElseIf = false;
			currentRequirement = IORequirementStatements::IF_EQUAL;

			// Parse function name and parameters
			size_t nameStart = strlen("Function ");
			size_t parenOpen = line.find('(', nameStart);
			size_t parenClose = line.find(')', nameStart);
			size_t braceOpen = line.find('{', nameStart);

			if (parenOpen != std::string::npos && parenClose != std::string::npos)
			{
				currentFunction.FunctionName = Trim(line.substr(nameStart, parenOpen - nameStart));
				std::string params = line.substr(parenOpen + 1, parenClose - parenOpen - 1);
				currentFunction.Parameters = Split(params, ',');
			}
			else
			{
				// No parameters
				size_t end = (braceOpen != std::string::npos) ? braceOpen : line.length();
				currentFunction.FunctionName = Trim(line.substr(nameStart, end - nameStart));
				currentFunction.Parameters.clear();
			}
			// If '{' is on the same line, enter block
			if (braceOpen != std::string::npos)
				inBlock = true;
			continue;
		}

		// Entering function block
		if (line == "{")
		{
			inBlock = true;
			continue;
		}
		// Exiting function block
		if (line == "}")
		{
			if (inFunction && !currentFunction.FunctionName.empty())
			{
				m_Functions.push_back(currentFunction);
				currentFunction = IOFunctionData();
			}
			inFunction = false;
			inBlock = false;
			continue;
		}

		// Inside function block: parse commands
		if (inFunction && inBlock)
		{
			// Remove inline comments
			size_t comment = line.find("//");
			if (comment != std::string::npos)
				line = Trim(line.substr(0, comment));
			if (line.empty())
				continue;

			// Command name is first word, rest are arguments
			std::istringstream iss(line);
			std::string cmdName;
			iss >> cmdName;
			IOFunctionCommand cmd;
			cmd.Require = inIf;
			cmd.RequireStatement = currentRequirement;
			cmd.IsElseIf = inElseIf;
			cmd.IsElse = inElse;
			cmd.SpawnItem.ListName.clear();
			cmd.SpawnItem.ItemName.clear();
			cmd.SpawnItem.Limit = 0;

			// Identify command type (IOFunctionCommands_t)
			int commandType = -1;
			for (int i = 0; i < IO_MAX; ++i)
			{
				if (cmdName == g_IOCommands[i])
				{
					commandType = i;
					break;
				}
			}
			//IO_SPECIAL_FUNCTION
			cmd.Type = (IOFunctionCommands_t)commandType;

			// Parse arguments according to command type
			std::string restOfLine;
			std::getline(iss, restOfLine);
			restOfLine = Trim(restOfLine);

			switch (cmd.Type)
			{
				case IO_FIRE_OUTPUT:
				{
					// Example: FireOutput "my_target" "MyInput" "MyValue" 5.0
					// EntFire = arg0, Input = arg1, Message = arg2, Delay = arg3
				    auto args = SplitQoutes(restOfLine, ' ');
					if (args.size() > 0)
						cmd.EntFire = MessageCleanup( args[0] );
					if (args.size() > 1)
					    cmd.Input = MessageCleanup( args[1] );
					if (args.size() > 2)
						cmd.Message = MessageCleanup( args[2] );
					if (args.size() > 3)
						cmd.Delay = std::stof( args[3] );
					break;
				}
				case IO_WAIT:
				{
					// Example: Wait 2.5
					if (!restOfLine.empty())
						cmd.Delay = std::stof(restOfLine);
					break;
				}
			    case IO_BREAK:
				{
				    // Example: Break {param} if "MyValue"
				    // Message = arg0, Input = arg1
				    auto args = Split(restOfLine, ' ');
				    // Replace {param} with %sN%
					cmd.Message = ReplaceScriptArgs( MessageCleanup( args[0] ), currentFunction.Parameters );
				    cmd.Input = ReplaceScriptArgs( MessageCleanup( args[2] ), currentFunction.Parameters );
					break;
				}
			    case IO_IF:
			    case IO_ELSEIF:
				{
					// Example: if EntityName == "apple"
					// or if EntityName is "apple"
				    // or if PlayerCount >= 5
				    // or if PlayerCount > 5
				    // or if PlayerCount <= 5
					// This also applies to elseif and else
					auto args = Split(restOfLine, ' ');
					if ( args.size() < 2 )
						inIf = "_invalid_if_statement";
					else
					{
						if ( args[1] == "==" || args[1] == "is" )
						{
							inIf = MessageCleanup( args[2] );
						    currentRequirement = IORequirementStatements::IF_EQUAL;
						}
					    else if (args[1] == "!=" || args[1] == "not" || args[1] == "isnot")
						{
							inIf = MessageCleanup( args[2] );
						    currentRequirement = IORequirementStatements::IF_NOT_EQUAL;
						}
					    else if (args[1] == ">=")
						{
							inIf = MessageCleanup( args[2] );
						    currentRequirement = IORequirementStatements::IF_GREATER_EQUAL;
						}
					    else if (args[1] == "<=")
						{
							inIf = MessageCleanup( args[2] );
						    currentRequirement = IORequirementStatements::IF_LESS_EQUAL;
						}
						else if (args[1] == ">")
						{
							inIf = MessageCleanup( args[2] );
							currentRequirement = IORequirementStatements::IF_GREATER;
						}
						else if (args[1] == "<")
						{
							inIf = MessageCleanup( args[2] );
							currentRequirement = IORequirementStatements::IF_LESS;
					    }
					    // Make sure this is set correctly.
					    if ( cmd.Type == IO_ELSEIF )
							inElseIf = true;
						else
						    inElseIf = false;
					}
				    break;
				}
				case IO_ELSE:
				{
				    // Else has no condition.
				    // IsElse is already set above.
				    currentRequirement = IORequirementStatements::IF_INVALID;
				    inElseIf = false;
				    inElse = true;
				}
			    break;
			    case IO_END:
				{
					if ( !inIf.empty() )
						inIf.clear();
				    break;
				}
			    case IO_EXEC_AS:
				{
					// Do nothing for now.
				    break;
				}
			    case IO_GIVE_ITEM:
				{
				    // Example: GiveItem 0 "item_name"
				    // arg0 = Player ID (0 for all players)
				    const std::string msg = ReplaceScriptArgs( restOfLine, currentFunction.Parameters );
				    auto args = Split( msg, ' ' );
					if ( args.size() < 2 ) break;
				    // We will store player ID in EntFire.
				    cmd.EntFire = args[0];
				    // Replace {param} with %sN%
					cmd.Message = ReplaceScriptArgs( MessageCleanup( args[1] ), currentFunction.Parameters );
				    break;
				}
				case IO_HAS_PARTICIPATED:
				{
					// Example: Participated 0 "ID_Name"
					// arg0 = Player ID (0 for all players)
					// arg1 = ID Name
					const std::string msg = ReplaceScriptArgs( restOfLine, currentFunction.Parameters );
				    auto args = Split(msg, ' ');
					if ( args.size() < 2 ) break;
					// We will store player ID in EntFire.
					cmd.EntFire = args[0];
				    // Replace {param} with %sN%
					cmd.Message = ReplaceScriptArgs( MessageCleanup( args[1] ), currentFunction.Parameters );
					break;
			    }
				case IO_ADD_TO_SPAWN_LIST:
				{
				    // Example: AddToSpawnList Ammo "item_name" 5
				    // arg0 = List name (Ammo/Weapons/Items)
					// arg1 = Item name
					// arg2 = Limit
					const std::string msg = ReplaceScriptArgs( restOfLine, currentFunction.Parameters );
				    auto args = Split(msg, ' ');
					if ( args.size() < 3 ) break;
					cmd.SpawnItem.ListName = MessageCleanup( args[0] );
					cmd.SpawnItem.ItemName = MessageCleanup( args[1] );
					cmd.SpawnItem.Limit = std::stoi( args[2] );
					break;
			    }
				case IO_PRINT_TO_CONSOLE:
				case IO_PRINT_TO_CHAT:
				{
					// Example: PrintToConsole "Message here"
					// Example: PrintToChat "Message here"
					// Replace {param} with %sN%
					cmd.Message = ReplaceScriptArgs( restOfLine, currentFunction.Parameters );
					break;
				}
				default:
					break;
			}

			currentFunction.Commands.push_back(cmd);
		}
	}
	// Push last function if file doesn't end with '}'
	if (inFunction && !currentFunction.FunctionName.empty())
		m_Functions.push_back(currentFunction);
	file.close();
}

void IOScriptFile::OnThink()
{
	for ( size_t i = 0; i < m_Commands.size(); i++ )
		RunCommands( i );
}

void IOScriptFile::OnRoundRestart()
{
	m_Commands.clear();
}

IOScriptFile::~IOScriptFile()
{
	m_Functions.clear();
}

ScriptCallBackEnum IOScriptFile::OnCalled(const std::string &szFunction, KeyValues *pData)
{
	// Fire an output variable
	if ( !pData ) return ScriptCall_Error;
	if ( FStrEq( pData->GetString( "arg0" ), "Function" ) )
		return CallData( szFunction, pData->GetString( "arg1" ) );

	CBaseEntity *pEnt = nullptr;
	edict_t *pEdict = INDEXENT( atoi( pData->GetString( "arg0" ) ) );
	if ( pEdict && !pEdict->free )
		pEnt = CBaseEntity::Instance( pEdict );
	if ( pData->GetBool( "IsInput" ) )
		return OnInput( pEnt, szFunction, pData->GetString( "arg1" ) );
	else
		return OnOutput( pEnt, szFunction, pData->GetString( "arg1" ), atof( pData->GetString( "arg2" ) ) );
}

ScriptCallBackEnum IOScriptFile::OnOutput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue, const float &szDelay )
{
	if ( !pEnt ) return ScriptCall_Error;
	return CallData( IO_ON_OUTPUT_SENT, UTIL_VarArgs( "%i, %s, %s, %s, %f", pEnt->entindex(), STRING( pEnt->pev->targetname ), szAction.c_str(), szValue.c_str(), szDelay ) );
}

ScriptCallBackEnum IOScriptFile::OnInput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue )
{
	if ( !pEnt ) return ScriptCall_Error;
	ScriptCallBackEnum nRet = CallData( IO_ON_INPUT_RECEIVED, UTIL_VarArgs( "%i, %s, %s", pEnt->entindex(), szAction.c_str(), szValue.c_str() ) );

	KeyValues *pScriptCall = new KeyValues( "Items" );
	pScriptCall->SetString( "Action", szAction.c_str() );
	pScriptCall->SetString( "arg0", szValue.c_str() );
	pEnt->ScriptCallback( pScriptCall );
	pScriptCall->deleteThis();
	return nRet;
}

ScriptCallBackEnum IOScriptFile::CallData( const std::string &szFunction )
{
	// Let's call our function with no arguments.
	return CallData( szFunction, "" );
}

ScriptCallBackEnum IOScriptFile::CallData( const std::string &szFunction, const std::string &szArgs )
{
	ScriptCallBackEnum nRet = ScriptCall_Warning;
	// We need this if we have more than 1 wait command.
	float flPreviousWait = 0.0f;
	for ( size_t i = 0; i < m_Functions.size(); i++ )
	{
		// Get our function data
		IOFunctionData function = m_Functions[i];
		if ( function.FunctionName != szFunction ) continue;

		// Let's add our command to our vector.
		// We must also apply the Input call
		IOFunctionCall IOCall;
		IOCall.ID = GetCurrentID() + 1;
		IOCall.Arguments = Split(szArgs, ',');

		if ( IOCall.Arguments.size() > 0 )
		{
			// Very simplified for now
			if ( szFunction == "OnOutputSent" )
				IOCall.InputCall = IOCall.Arguments[1];
			else
				IOCall.InputCall = IOCall.Arguments[0];
		}
		else
			IOCall.InputCall = "";

		// Add our commands
		for ( size_t y = 0; y < function.Commands.size(); y++ )
		{
			IOFunctionCommand cmd = function.Commands[y];
			// Only wait needs this
			if ( cmd.Type == IO_WAIT )
			{
				float flDelay = cmd.Delay;
				cmd.Delay = gpGlobals->time + flDelay + flPreviousWait;
				flPreviousWait += flDelay;
			}
			IOCall.Commands.push_back( cmd );
		}

		m_Commands.push_back( IOCall );

		// We found our function, return OK
		nRet = ScriptCall_OK;
	}
	return nRet;
}

ScriptCallBackEnum IOScriptFile::CallData( const IOFunctions_t &nFunction, const std::string &szArgs )
{
	return CallData( g_IOFunctions[ nFunction ], szArgs );
}

void IOScriptFile::RunCommands( int nID )
{
	IOFunctionCall &pFunctionCall = m_Commands[ nID ];
	// If we have commands to execute, then let's do that.
	if ( pFunctionCall.Commands.size() > 0 )
	{
		IOFunctionCommand cmd = pFunctionCall.Commands[ 0 ];

		// Not empty? check what we require
		if ( !cmd.Require.empty() )
		{
			bool bMatch = true;
			// Make sure what we require is the same as our input call
			if ( cmd.Require != pFunctionCall.InputCall )
				bMatch = false;

			// Maybe it's a argument value?
			if ( !bMatch && cmd.RequireStatement != IORequirementStatements::IF_EQUAL )
			{
				// Reset, and try again.
				bMatch = true;

				const std::string &szValue = pFunctionCall.InputCall;
				const std::string &szArgument = cmd.Require;

				// Check our requirement statement (only for numeric values)
				if ( cmd.RequireStatement == IORequirementStatements::IF_GREATER )
				{
					if ( atoi( szValue.c_str() ) <= atoi( szArgument.c_str() ) )
						bMatch = false;
				}
				else if ( cmd.RequireStatement == IORequirementStatements::IF_LESS )
				{
					if ( atoi( szValue.c_str() ) >= atoi( szArgument.c_str() ) )
						bMatch = false;
				}
				else if ( cmd.RequireStatement == IORequirementStatements::IF_GREATER_EQUAL )
				{
					if ( atoi( szValue.c_str() ) < atoi( szArgument.c_str() ) )
						bMatch = false;
				}
				else if ( cmd.RequireStatement == IORequirementStatements::IF_LESS_EQUAL )
				{
					if ( atoi( szValue.c_str() ) > atoi( szArgument.c_str() ) )
						bMatch = false;
				}
				else if ( cmd.RequireStatement == IORequirementStatements::IF_NOT_EQUAL )
				{
					if ( szValue == szArgument )
						bMatch = false;
					else if ( cmd.Require == szArgument )
						bMatch = false;
				}
			}

			// If we don't match, erase this command and return.
			if ( !bMatch )
			{
				pFunctionCall.Commands.erase( pFunctionCall.Commands.begin() );
				return;
			}

			// We match, but are we on the correct block?
			switch ( pFunctionCall.InsideIfBlock )
			{
				case IOFunctionCallIfBlockStatements::IFBLOCK_IF:
					// We are inside an if block, but is it the correct one?
					if ( cmd.Type != IO_IF )
					{
						// Not the correct one, erase and return.
						pFunctionCall.Commands.erase( pFunctionCall.Commands.begin() );
						return;
					}
				break;
				case IOFunctionCallIfBlockStatements::IFBLOCK_ELSEIF:
					// We are inside an elseif block, but is it the correct one?
					if ( cmd.Type != IO_ELSEIF )
					{
						// Not the correct one, erase and return.
						pFunctionCall.Commands.erase( pFunctionCall.Commands.begin() );
						return;
					}
				break;
				case IOFunctionCallIfBlockStatements::IFBLOCK_ELSE:
					// We are inside an else block, but is it the correct one?
					if ( cmd.Type != IO_ELSE )
					{
						// Not the correct one, erase and return.
						pFunctionCall.Commands.erase( pFunctionCall.Commands.begin() );
						return;
					}
				break;
			}

			// We are inside an if block now, let's check what type we are.
			switch( cmd.Type )
			{
				case IO_IF: pFunctionCall.InsideIfBlock = IOFunctionCallIfBlockStatements::IFBLOCK_IF; break;
				case IO_ELSE: pFunctionCall.InsideIfBlock = IOFunctionCallIfBlockStatements::IFBLOCK_ELSE; break;
				case IO_ELSEIF: pFunctionCall.InsideIfBlock = IOFunctionCallIfBlockStatements::IFBLOCK_ELSEIF; break;
			}
		}

		if ( cmd.Type == IO_BREAK )
		{
			std::string szArgument = GetArgValues( cmd.Message, pFunctionCall.Arguments );
			std::string szInput = GetArgValues( cmd.Input, pFunctionCall.Arguments );
			if ( szArgument == szInput )
			{
				pFunctionCall.Commands.clear();
				return;
			}
		}

		switch ( cmd.Type )
		{
			case IO_FIRE_OUTPUT:
			{
				// Let's fire an output!
				// Make sure it's I/O
				// We don't use the entity index here, since we don't want to call ourselves and make an infinite loop (if it happens)
			    const std::string &szArg0( "SCall" );
			    const std::string &szArg1( cmd.EntFire );
			    const std::string &szArg2( cmd.Message );
			    ScriptSystem::CallScriptDelay( AvailableScripts_t::InputOutput, nullptr, cmd.Input, cmd.Delay, 3, szArg0, szArg1, szArg2 );
			}
			break;

			case IO_EXEC_AS:
				// Doesn't do anything for now.
			break;

			case IO_GIVE_ITEM:
				// Give item to player(s)
				{
					std::string szPlayer = GetArgValues( cmd.EntFire, pFunctionCall.Arguments );
					int nPlayerID = atoi( szPlayer.c_str() );
					std::string szItem = GetArgValues( cmd.Message, pFunctionCall.Arguments );
					int iszItem = ALLOC_STRING( szItem.c_str() ); // Make a copy of the classname
					if ( nPlayerID == 0 )
					{
						// Give to all players
						for ( int i = 1; i <= gpGlobals->maxClients; i++ )
						{
							CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( i );
							if ( pPlayer )
								pPlayer->GiveNamedItem( STRING(iszItem) );
						}
					}
					else
					{
						CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( nPlayerID );
						if ( pPlayer )
							pPlayer->GiveNamedItem( STRING(iszItem) );
					}
			    }
			break;

			case IO_HAS_PARTICIPATED:
			{
				std::string szPlayer = GetArgValues( cmd.EntFire, pFunctionCall.Arguments );
				int nPlayerID = atoi( szPlayer.c_str() );
				std::string szIDName = GetArgValues( cmd.Message, pFunctionCall.Arguments );
				DialogAchievementData ach = GetAchievementByID( szIDName.c_str() );
				if ( nPlayerID == 0 )
				{
					for ( int i = 1; i <= gpGlobals->maxClients; i++ )
					{
						CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( i );
						if ( pPlayer )
							pPlayer->Participated( ach.GetAchievementID() );
					}
				}
				else
				{
					CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( nPlayerID );
					if ( pPlayer )
						pPlayer->Participated( ach.GetAchievementID() );
				}
		    }
		    break;

			case IO_ADD_TO_SPAWN_LIST:
			{
				// Let's add our spawn item to our list.
				if ( !cmd.SpawnItem.ListName.empty() && !cmd.SpawnItem.ItemName.empty() && cmd.SpawnItem.Limit > 0 )
					s_SpawnListData.push_back( cmd.SpawnItem );
		    }
		    break;

			case IO_SPAWN_ITEMS:
			{
				KeyValuesAD mySpawnItems( "SpawnItems" );
			    // Let's populare our spawn items.
				for ( size_t i = 0; i < s_SpawnListData.size(); i++ )
			    {
					ISpawnListData item = s_SpawnListData[i];
					KeyValuesAD pItem( item.ListName.c_str() );
					pItem->SetString( "Classname", item.ItemName.c_str() );
					pItem->SetInt( "Amount", item.Limit );
					mySpawnItems->AddSubKey( pItem->MakeCopy() );
			    }
			    // Let's spawn our items now.
				ZP::IO_CalculatePlayerAmount( mySpawnItems );
				// Clear our list after use.
				s_SpawnListData.clear();
			}
		    break;

			case IO_WAIT:
			{
				// We still have delay, don't go to the next command until we are done.
			    if ( cmd.Delay > gpGlobals->time )
					return;
			}
			break;

			case IO_PRINT_TO_CHAT:
			{
			    std::string szOutput = GetArgValues( cmd.Message, pFunctionCall.Arguments );
			    szOutput = MessageCleanup( szOutput ) + "\n";
				UTIL_ClientPrintAll( HUD_PRINTTALK, szOutput.c_str() );
			}
			break;

			case IO_PRINT_TO_CONSOLE:
			{
			    std::string szOutput = GetArgValues( cmd.Message, pFunctionCall.Arguments );
			    szOutput = MessageCleanup( szOutput ) + "\n";
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
				    if ( pPlayer )
						UTIL_PrintConsole( szOutput.c_str(), pPlayer );
				}
			}
			break;

			case IO_END:
			{
				// We reached the end of the if block, reset it.
			    pFunctionCall.InsideIfBlock = IOFunctionCallIfBlockStatements::IFBLOCK_NONE;
			}
			break;
		}

		// Erase after use
		pFunctionCall.Commands.erase( pFunctionCall.Commands.begin() );
	}
	else
		m_Commands.erase( m_Commands.begin() + nID );
}

uint IOScriptFile::GetCurrentID() const
{
	if ( m_Commands.size() == 0 ) return -1;
	return m_Commands[ m_Commands.size() - 1 ].ID;
}

const char *IO_GetAvailableFunctions( IOFunctions_t nFunc )
{
	return g_IOFunctions[ nFunc ];
}

const char *IO_GetAvailableFunctionCommands( IOFunctionCommands_t nCommand )
{
	return g_IOCommands[ nCommand ];
}
