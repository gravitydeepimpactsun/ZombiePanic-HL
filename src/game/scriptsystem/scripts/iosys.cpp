// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include "core.h"
#include "iosys.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <tier2/tier2.h>
#include "FileSystem.h"
#include <KeyValues.h>

// A simple static list of our I/O IOFunctions_t
static const char *g_IOFunctions[IO_ON_MAX] = {
	"OnMapStart",
	"OnMapEnd",
	"OnPlayerSpawned",
	"OnInputReceived",
	"OnOutputSent"
};

// Same as above, a static list of our I/O IOFunctionCommands_t
static const char *g_IOCommands[IO_MAX] = {
	"FireOutput",
	"Wait",
	"PrintToConsole",
	"PrintToChat",
};

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

static std::string ReplaceScriptArgs(const std::string &message, const std::vector<std::string> &params)
{
	std::string result = message;
	for (size_t i = 0; i < params.size(); ++i)
	{
		std::string pattern = "\" + " + params[i] + " + \"";
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

void IOSystem::OnInit()
{
	// Nothing to do here yet.
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

	// Load our map script file.
	m_szScript = new IOScriptFile( "scripts/maps/" + std::string( STRING( gpGlobals->mapname ) ) + ".txt" );
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
	Function OnInputReceived( String EntityID, String InputName, String InputValue )
	{
		// We recieved an input!
		// Arguments:
		// arg0 = Entity ID (entindex number)
		// arg1 = Input name
		// arg2 = Value
		// Example:
		PrintToConsole "We recieved an input from entity: " + EntityID + " with input name: " + InputName + " and value: " + InputValue
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
		PrintToConsole "We sent an output to entity: " + EntityID + " with output name: " + OutputName + " and value: " + OutputValue + " after a delay of: " + OutputDelay
	}
	*/

	// Let's parse our file.
	m_szFileName = szFile;
	m_Functions.clear();

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
					// EntFire = arg0, Message = arg2, Delay = arg3
					auto args = Split(restOfLine, ' ');
					if (args.size() > 0)
						cmd.EntFire = args[0];
					if (args.size() > 2)
						cmd.Message = args[2];
					if (args.size() > 3)
						cmd.Delay = std::stof(args[3]);
					break;
				}
				case IO_WAIT:
				{
					// Example: Wait 2.5
					if (!restOfLine.empty())
						cmd.Delay = std::stof(restOfLine);
					break;
				}
				case IO_PRINT_TO_CONSOLE:
				case IO_PRINT_TO_CHAT:
				{
					// Example: PrintToConsole "Message here"
				    // Replace " + param + " with %sN%
				    cmd.Message = ReplaceScriptArgs(restOfLine, currentFunction.Parameters);
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

IOScriptFile::~IOScriptFile()
{
	m_Functions.clear();
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
	CallData( IO_ON_OUTPUT_SENT, UTIL_VarArgs( "%i, %s, %s", pEnt->entindex(), szAction.c_str(), szValue.c_str() ) );
}

void IOScriptFile::OnInput( CBaseEntity *pEnt, const std::string &szAction, const std::string &szValue )
{
	if ( !pEnt ) return;
	CallData( IO_ON_INPUT_RECEIVED, UTIL_VarArgs( "%i, %s, %s", pEnt->entindex(), szAction.c_str(), szValue.c_str() ) );

	KeyValues *pScriptCall = new KeyValues( "ScriptCallback" );
	pScriptCall->SetString( "Action", szAction.c_str() );
	pScriptCall->SetString( "Value", szValue.c_str() );
	pEnt->ScriptCallback( pScriptCall );
	pScriptCall->deleteThis();
}

void IOScriptFile::CallData( const std::string &szFunction )
{
	// Let's call our function with no arguments.
	CallData( szFunction, "" );
}

void IOScriptFile::CallData( const std::string &szFunction, const std::string &szArgs )
{
	// TODO: Create the crap
	// Format our arguments, read function, etc...
}

void IOScriptFile::CallData( const IOFunctions_t &nFunction, const std::string &szArgs )
{
	CallData( g_IOFunctions[ nFunction ], szArgs );
}
