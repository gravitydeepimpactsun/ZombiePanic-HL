//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include "C_CreateServer.h"
#include "zp/ui/workshop/WorkshopItemList.h"
#include "client_vgui.h"
#include "FileSystem.h"

#include <fstream>

#include "C_ConfigList.h"

#include "vgui_controls/CheckButton.h"

using namespace vgui2;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

static std::vector<std::string> vMapList;

CServerConfigData::CServerConfigData( Panel *parent, char const *panelName )
: Panel( parent, panelName )
{
	pControl = NULL;
	pPrompt = NULL;
	bIsCheat = false;
	confType = Conf_Int;

	next = NULL;

	SetPaintBackgroundEnabled( false );
}

void CServerConfigData::OnSizeChanged( int wide, int tall )
{
	int inset = 4;

	if ( pPrompt )
	{
		int w = wide / 2;

		if ( pControl )
		{
			pControl->SetBounds( w + 20, inset, w - 20, tall - 2 * inset );
		}
		pPrompt->SetBounds( 0, inset, w + 20, tall - 2 * inset  );
	}
	else
	{
		if ( pControl )
		{
			pControl->SetBounds( 0, inset, wide, tall - 2 * inset  );
		}
	}
}

C_CreateServer::C_CreateServer( vgui2::Panel *pParent ) : BaseClass( pParent, "C_CreateServer" )
{
	SetTitleBarVisible( true );
	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( true );
	SetSizeable( false );
	SetMoveable( true );
	SetVisible( true );
	SetProportional( true );
	SetDeleteSelfOnClose( true );
	Activate();
	RequestFocus();

	vgui2::HScheme hScheme = vgui2::scheme()->LoadSchemeFromFile( VGUI2_ROOT_DIR "resource/ClientSourceScheme.res", "ClientSourceScheme" );
	SetScheme( hScheme );

	ui_MapList = new vgui2::ComboBox(this, "MapList", 1, false);
	ui_MaxPlayers = new vgui2::ComboBox(this, "server_maxplayers", 15, false);
	m_pOptionsList = new C_ConfigList(this, "GameOptions");
	m_pList = nullptr;

	kvMapData = new KeyValues("MapData");

	LoadControlSettings( VGUI2_ROOT_DIR "resource/zps/createserver.res" );

	SetTitle( "#ZP_UI_CreateServer_Title", true );

	LoadMaps();

	KeyValues *kvMaxPlayers = new KeyValues("PlayerData");
	KeyValues::AutoDelete DeleteData( kvMaxPlayers );
	for ( int iplymax = 2; iplymax <= 24; iplymax++ )
	{
		char strVal[ 250 ];
		strVal[0] = 0;
		Q_snprintf( strVal, sizeof( strVal ), "%i", iplymax );
		ui_MaxPlayers->AddItem( strVal, kvMaxPlayers );
	}

	// Set the combobox default values
	ui_MapList->ActivateItem( 0 );		// Default is random
	ui_MaxPlayers->ActivateItem( 14 );	// Default is 16

	KeyValues *Filters = new KeyValues( "ServerInfoDialog" );
	KeyValues::AutoDelete autoDelete( Filters );

	// If the file exist
	if ( Filters->LoadFromFile( g_pFullFileSystem, "serverinfo.vdf", "CONFIG" ) )
	{
		// Set current map
		for ( int iMap = 0; iMap < vMapList.size(); iMap++ )
		{
			if ( vgui2::FStrEq( Filters->GetString( "currentmap" ), vMapList[iMap].c_str() ) )
			{
				ui_MapList->ActivateItem( iMap );
				break;
			}
		}
	}

	vgui2::Label *pLabel = (vgui2::Label *)FindChildByName( "MapLabel" );
	if ( pLabel )
		pLabel->SetText( "#ZP_UI_CreateServer_Map" );

	pLabel = (vgui2::Label *)FindChildByName( "MaxPlayersLabel" );
	if ( pLabel )
		pLabel->SetText( "#ZP_UI_CreateServer_MaxPlayers" );

	ConfigData = new KeyValues( "ConfigData" );
	ConfigData->LoadFromFile( g_pFullFileSystem, "cfg/server_config.vdf", "GAME" );

	ConfigSavedData = new KeyValues( "Config" );
	if ( !ConfigSavedData->LoadFromFile( g_pFullFileSystem, "cfg/server_settings.vdf", "GAME" ) )
	{
		char filename[256];
		snprintf( filename, sizeof(filename), "%s/cfg/server_settings.vdf", gEngfuncs.pfnGetGameDirectory() );
		std::fstream file;
		file.open( filename, std::fstream::out );

		// Empty file
		file << "\"Config\"\n";
		file << "{\n";
		file << "}\n";

		file.close();
	}

	// Load our config
	LoadConfig( "server_maxplayers", "maxplayers", Conf_ComboBox );

	// Load from our file
	LoadConfigFile();

	// Make sure this always pops up at the center of the screen
	MoveToCenterOfScreen();
}


C_CreateServer::~C_CreateServer()
{
	if ( ConfigData )
		ConfigData->deleteThis();
	if ( ConfigSavedData )
		ConfigSavedData->deleteThis();
	if ( kvMapData )
		kvMapData->deleteThis();
	vMapList.clear();
}


void C_CreateServer::SaveConfigFile()
{
	char filename[256];
	snprintf( filename, sizeof(filename), "%s/cfg/server_settings.vdf", gEngfuncs.pfnGetGameDirectory() );
	std::fstream file;
	file.open( filename, std::fstream::out );

	file << "\"Config\"\n";
	file << "{\n";
	// Apply our config to the file as KeyValues
	for ( KeyValues *sub = ConfigSavedData->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
	{
		file << "\t\"" << sub->GetName() << "\"\t\"" << sub->GetString() << "\"\n";
	}
	file << "}\n";

	file.close();
}

const char *C_CreateServer::GetConfigFile(const char *strConfig, const char *strArg)
{
	for ( CServerConfigData *mp = m_pList; mp != NULL; mp = mp->next )
	{
		Panel *control = mp->pControl;
		if ( control && !stricmp( mp->GetName(), strConfig ) )
		{
			if ( mp->bIsCheat )
			{
				SaveConfig( "sv_cheats", "1", Conf_Bool );
				gEngfuncs.pfnClientCmd( "sv_cheats 1" );
			}

			KeyValues *data = new KeyValues("GetText");
			static char buf[128];
			if ( control && control->RequestInfo( data ) )
				Q_strncpy( buf, data->GetString( "text", strArg ), sizeof( buf ) - 1 );
			else
			{
				// no value found, copy in default text
				Q_strncpy( buf, strArg, sizeof( buf ) - 1 );
			}

			// ensure null termination of string
			buf[sizeof(buf) - 1] = 0;

			// free
			data->deleteThis();
			return buf;
		}
	}
	return "";
}

void C_CreateServer::LoadConfigFile()
{
	Panel *objParent = m_pOptionsList;
	CheckButton* pBox = nullptr;
	TextEntry* pEdit = nullptr;
	ComboBox *pCombo = nullptr;
	Slider *pSlider = nullptr;

	for ( KeyValues *sub = ConfigData->GetFirstSubKey(); sub != NULL ; sub = sub->GetNextKey() )
	{
		// Make sure this is valid
		const char *strConvar = sub->GetString( "convar", "" );
		if ( strConvar[0] == 0 ) continue;

		// Ditto, make sure we have a type
		const char *strType = sub->GetString( "type", "" );
		if ( strType[0] == 0 ) continue;

		const char *strLabel = sub->GetString( "string", "" );
		const char *strDefault = sub->GetString( "default", "" );

		configType confType = Conf_Int;
		if ( !Q_stricmp( strType, "bool" ) )
			confType = Conf_Bool;
		else if ( !Q_stricmp( strType, "string" ) )
			confType = Conf_String;
		else if ( !Q_stricmp( strType, "combobox" ) )
			confType = Conf_ComboBox;
		else if ( !Q_stricmp( strType, "slider" ) )
			confType = Conf_Slider;

		bool bHasData = vgui2::FStrEq( ConfigSavedData->GetString( strConvar, "" ), "" ) == false;

		CServerConfigData *pCtrl = new CServerConfigData( objParent, strConvar );

		// Is this a cheat var?
		pCtrl->bIsCheat = sub->GetBool( "cheat" );

		// Set the conf type
		pCtrl->confType = confType;

		switch ( confType )
		{
			case Conf_Int:
			case Conf_String:
			{
				pEdit = new TextEntry( pCtrl, "DescEdit");
				std::string strDefVal = strDefault;
			    if ( bHasData )
					strDefVal = ConfigSavedData->GetString( strConvar, strDefVal.c_str() );
				pEdit->InsertString( strDefVal.c_str() );
				pCtrl->pControl = (Panel *)pEdit;
			}
			break;

			case Conf_Bool:
			{
				if ( bHasData )
					bHasData = ConfigSavedData->GetBool( strConvar );
			    else
					bHasData = sub->GetBool( "default" );

				pBox = new CheckButton( pCtrl, "DescCheckButton", strLabel );
				pBox->SetSelected( bHasData );
				pCtrl->pControl = (Panel*)pBox;
			}
			break;

			case Conf_ComboBox:
			{
			    pCombo = new ComboBox( pCtrl, "DescComboBox", 5, false );
			    // Find the options for this combo box
				KeyValues *pSubCombo = sub->FindKey( "options" );
			    if ( pSubCombo )
				{
					KeyValues *kvComboData = new KeyValues( "ComboData" );
					KeyValues::AutoDelete autoDelete( kvComboData );
					for ( KeyValues *subCombo = pSubCombo->GetFirstSubKey(); subCombo != NULL ; subCombo = subCombo->GetNextKey() )
					{
						const char *strVal = subCombo->GetString( "string", "" );
						if ( strVal[0] == 0 ) continue;
						kvComboData->SetString( strVal, "" );
						pCombo->AddItem( strVal, kvComboData );
					}
			    }
				if ( bHasData )
				    pCombo->ActivateItem( ConfigSavedData->GetInt( strConvar, 0 ) );
			    else
					pCombo->ActivateItem( sub->GetInt( "default", 0 ) );
			    pCtrl->pControl = (Panel *)pCombo;
			}
		    break;

			case Conf_Slider:
			{
				pSlider = new Slider( pCtrl, "DescSlider" );
				pSlider->SetRange( sub->GetInt( "min", 0 ), sub->GetInt( "max", 100 ) );
			    pSlider->SetTickCaptions( sub->GetString( "minstring", "" ), sub->GetString( "maxstring", "" ) );
				if ( bHasData )
					pSlider->SetValue( ConfigSavedData->GetInt( strConvar, 0 ) );
				else
					pSlider->SetValue( sub->GetInt( "default", 0 ) );
			    pCtrl->pControl = (Panel *)pSlider;
			}
		    break;
		}

		if ( confType != Conf_Bool )
		{
			pCtrl->pPrompt = new vgui2::Label( pCtrl, "DescLabel", "" );
			pCtrl->pPrompt->SetContentAlignment( vgui2::Label::a_west );
			pCtrl->pPrompt->SetTextInset( 5, 0 );
			pCtrl->pPrompt->SetText( strLabel );
		}

		pCtrl->SetSize( GetScaledValue( 100 ), GetScaledValue( 28 ) );
		m_pOptionsList->AddItem( pCtrl );

		// Link it in
		if ( !m_pList )
		{
			m_pList = pCtrl;
			pCtrl->next = NULL;
		}
		else
		{
			CServerConfigData*p;
			p = m_pList;
			while ( p )
			{
				if ( !p->next )
				{
					p->next = pCtrl;
					pCtrl->next = NULL;
					break;
				}
				p = p->next;
			}
		}
	}
}


void C_CreateServer::RunConfigFile()
{
	for ( CServerConfigData *mp = m_pList; mp != NULL; mp = mp->next )
	{
		Panel *control = mp->pControl;
		if ( control )
		{
			// It's empty, ignore it
			if ( FStrEq( mp->GetName(), "" ) ) continue;
			KeyValues *data = new KeyValues("GetText");
			static char buf[128];
			if ( control->RequestInfo( data ) )
			{
				Q_strncpy( buf, data->GetString( "text", "" ), sizeof( buf ) - 1 );

				// ensure null termination of string
				buf[sizeof(buf) - 1] = 0;

				// free
				data->deleteThis();

				vgui2::CheckButton *pValue = dynamic_cast<vgui2::CheckButton *>(mp->pControl);
				if ( pValue )
				{
					if ( mp->bIsCheat && pValue->IsSelected() )
					{
						SaveConfig( "sv_cheats", "1", Conf_Bool );
						gEngfuncs.pfnClientCmd( "sv_cheats 1" );
					}
					Q_snprintf( buf, sizeof( buf ), "%i", pValue->IsSelected() );
				}
				else
				{
					if ( mp->bIsCheat && !FStrEq( buf, "" ) )
					{
						SaveConfig( "sv_cheats", "1", Conf_Bool );
						gEngfuncs.pfnClientCmd( "sv_cheats 1" );
					}
				}
			}

			char command[ 1024 ];
			Q_snprintf(
				command,
				sizeof( command ),
				"%s \"%s\"\n",
				mp->GetName(),
				buf
			);
			gEngfuncs.pfnClientCmd( command );
			SaveConfig( mp->GetName(), buf, mp->confType );
		}
	}
}


void C_CreateServer::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	vgui2::Label *pLabel = (vgui2::Label *)FindChildByName( "TimeLimitLabel" );
	if ( pLabel )
		pLabel->SetFont( pScheme->GetFont( "DefaultSmall" ) );
	pLabel = (vgui2::Label *)FindChildByName( "MaxRoundsLabel" );
	if ( pLabel )
		pLabel->SetFont( pScheme->GetFont( "DefaultSmall" ) );
}


void C_CreateServer::LoadMaps()
{
	// Insert the missing maps
	if ( vMapList.size() <= 0 )
	{
		// Add the random one first
		AddMap( "#ZP_UI_CreateServer_RandomMap" );

		FileFindHandle_t findHandle;
		const char *pFilename = g_pFullFileSystem->FindFirst( "maps/*.bsp", &findHandle, "GAME" );
		while ( pFilename )
		{
			AddMap( pFilename );
			pFilename = g_pFullFileSystem->FindNext( findHandle );
		}
		g_pFullFileSystem->FindClose( findHandle );

		// Now check the workshop items under "ADDON"
		pFilename = g_pFullFileSystem->FindFirst( "maps/*.bsp", &findHandle, "ADDON" );
		while ( pFilename )
		{
			AddMap( pFilename );
			pFilename = g_pFullFileSystem->FindNext( findHandle );
		}
		g_pFullFileSystem->FindClose( findHandle );
	}

	// Add to list
	for ( int iMap = 0; iMap < vMapList.size(); iMap++ )
		ui_MapList->AddItem( vMapList[iMap].c_str(), kvMapData );

	// Update the amount of lines we got
	ui_MapList->SetNumberOfEditLines( vMapList.size() >= 15 ? 15 : vMapList.size() );
}


void C_CreateServer::AddMap( const char *strMap )
{
	// Strip away .bsp
	char sString_OutPut[ 1024 ];
	Q_StrSubst( strMap, ".bsp", "", sString_OutPut, sizeof( sString_OutPut ) );

	// Check if the value exist
	for ( int iMap = 0; iMap < vMapList.size(); iMap++ )
	{
		if ( FStrEq( sString_OutPut, vMapList[iMap].c_str() ) ) return;
	}

	// Add to map list
	vMapList.push_back( sString_OutPut );
}


void C_CreateServer::SelectMap( int iMap )
{
	KeyValuesAD ServerInfoDialog( "ServerInfoDialog" );
	ServerInfoDialog->LoadFromFile( g_pFullFileSystem, "serverinfo.vdf", "CONFIG" );
	ServerInfoDialog->SetString( "currentmap", vMapList[iMap].c_str() );
	ServerInfoDialog->SaveToFile( g_pFullFileSystem, "serverinfo.vdf", "CONFIG" );
}


void C_CreateServer::RunMap( int iMap )
{
	// Disconnect first, so we can change maxplayers
	gEngfuncs.pfnClientCmd( "disconnect\nwait\n" );

	// Setup the configs
	RunConfig( "server_maxplayers", "maxplayers", Conf_ComboBox );

	// Load from our json file
	RunConfigFile();

	// Grab the map
	std::string strMap = iMap > 0 ? vMapList[iMap] : vMapList[ RandomInt(1, vMapList.size() - 1) ];

	// Run the map itself
	char command[ 1024 ];
	Q_snprintf(
		command,
		sizeof( command ),
		"wait\nwait\nmap %s\n",
		strMap.c_str()
	);

	gEngfuncs.pfnClientCmd( command );
}


void C_CreateServer::RunConfig( const char *strConfig, const char *strArg, configType cType, bool bRequireCheats )
{
	vgui2::Panel *pPanel = FindChildByName( strConfig, true );
	std::string strValue = "";

	switch ( cType )
	{
		default:
		case Conf_Bool:
		{
			vgui2::CheckButton *pValue = (vgui2::CheckButton *)pPanel;
			if ( pValue )
			{
				if ( bRequireCheats && pValue->IsSelected() )
				{
					SaveConfig( "sv_cheats", "1", cType );
					gEngfuncs.pfnClientCmd( "sv_cheats 1" );
				}
				strValue = pValue->IsSelected() ? "1" : "0";
			}
		}
		break;

		case Conf_String:
		{
			vgui2::TextEntry *pValue = (vgui2::TextEntry *)pPanel;
			if ( pValue )
			{
				char szText[ 64 ];
				pValue->GetText( szText, 64 );
				if ( bRequireCheats && !FStrEq( szText, "" ) )
				{
					SaveConfig( "sv_cheats", "1", cType );
					gEngfuncs.pfnClientCmd( "sv_cheats 1" );
				}
				strValue = szText;
			}
		}
		break;

		case Conf_Int:
		{
			vgui2::Slider *pValue = (vgui2::Slider *)pPanel;
			if ( pValue )
			{
				char strVal[ 250 ];
				strVal[0] = 0;
				Q_snprintf( strVal, sizeof( strVal ), "%i", pValue->GetValue() );
				if ( bRequireCheats && pValue->GetValue() > 0 )
				{
					SaveConfig( "sv_cheats", "1", cType );
					gEngfuncs.pfnClientCmd( "sv_cheats 1" );
				}
				strValue = strVal;
			}
		}
		break;

		case Conf_ComboBox:
		{
			vgui2::ComboBox *pValue = (vgui2::ComboBox *)pPanel;
			if ( pValue )
			{
				char strVal[ 250 ];
				strVal[0] = 0;
				int iValue = pValue->GetActiveItem();
				if ( FStrEq( strConfig, "server_maxplayers" ) )
					iValue += 2; // We start on slot 2, not 1
				Q_snprintf( strVal, sizeof( strVal ), "%i", iValue );
				if ( bRequireCheats && pValue->GetActiveItem() > 0 )
				{
					SaveConfig( "sv_cheats", "1", cType );
					gEngfuncs.pfnClientCmd( "sv_cheats 1" );
				}
				strValue = strVal;
			}
		}
		break;

		case Conf_Slider:
		{
			vgui2::Slider *pValue = (vgui2::Slider *)pPanel;
			if ( pValue )
			{
				char strVal[ 250 ];
				strVal[0] = 0;
				Q_snprintf( strVal, sizeof( strVal ), "%i", pValue->GetValue() );
				if ( bRequireCheats && pValue->GetValue() > 0 )
				{
					SaveConfig( "sv_cheats", "1", cType );
					gEngfuncs.pfnClientCmd( "sv_cheats 1" );
				}
				strValue = strVal;
		    }
		}
	    break;
	}

	// Try the config file
	if ( FStrEq( strValue.c_str(), "" ) )
		strValue = GetConfigFile( strValue.c_str(), strArg );

	// empty? then don't run it
	if ( FStrEq( strValue.c_str(), "" ) ) return;

	char command[ 1024 ];
	Q_snprintf(
		command,
		sizeof( command ),
		"%s %s\n",
		strArg,
		strValue.c_str()
	);
	gEngfuncs.pfnClientCmd( command );
	SaveConfig( strArg, strValue.c_str(), cType );
}


void C_CreateServer::LoadConfig( const char *strConfig, const char *strArg, configType cType )
{
	vgui2::Panel *pPanel = FindChildByName( strConfig, true );
	switch ( cType )
	{
		default:
		case Conf_Bool:
		{
			vgui2::CheckButton *pValue = (vgui2::CheckButton *)pPanel;
			if ( pValue )
			    pValue->SetSelected( ConfigSavedData->GetBool( strArg ) );
		}
		break;

		case Conf_String:
		{
			vgui2::TextEntry *pValue = (vgui2::TextEntry *)pPanel;
			if ( pValue )
				pValue->SetText( ConfigSavedData->GetString( strArg ) );
		}
		break;

		case Conf_Int:
		{
			vgui2::Slider *pValue = (vgui2::Slider *)pPanel;
			if ( pValue )
				pValue->SetValue( ConfigSavedData->GetInt( strArg ) );
		}
		break;

		case Conf_ComboBox:
		{
			vgui2::ComboBox *pValue = (vgui2::ComboBox *)pPanel;
			if ( pValue )
			{
			    int iValue = ConfigSavedData->GetInt( strArg );
				if ( FStrEq( strConfig, "server_maxplayers" ) )
					iValue -= 2; // We start on slot 2, not 1
				pValue->ActivateItem( iValue );
			}
		}
		break;

		case Conf_Slider:
		{
			vgui2::Slider *pValue = (vgui2::Slider *)pPanel;
			if ( pValue )
				pValue->SetValue( ConfigSavedData->GetInt( strArg ) );
	    }
	}
}


void C_CreateServer::SaveConfig( const char *strArg, const char *strValue, configType cType )
{
	switch ( cType )
	{
		default:
		case Conf_Bool:
		    ConfigSavedData->SetBool( strArg, atoi( strValue ) > 0 ? true : false );
		break;

		case Conf_String:
			ConfigSavedData->SetString( strArg, strValue );
		break;

		case Conf_Int:
		case Conf_ComboBox:
	    case Conf_Slider:
		    ConfigSavedData->SetInt( strArg, atoi( strValue ) );
		break;
	}
}


void C_CreateServer::OnCommand( const char *command )
{
	if ( !Q_stricmp( command, "Create" ) )
	{
		// Our current map
		int iMap = ui_MapList->GetActiveItem();

		// Save to file
		SelectMap( iMap );

		// Run the map
		RunMap( iMap );

		// Save our config
		SaveConfigFile();

		// Close the dialog
		Close();
	}
	else
		BaseClass::OnCommand( command );
}
