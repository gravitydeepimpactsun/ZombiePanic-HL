//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_CLIENT_ZPS_VGUI_MAINMENU_CREATE_SERVER_H
#define GAME_CLIENT_ZPS_VGUI_MAINMENU_CREATE_SERVER_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "vgui_controls/ListPanel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Slider.h"

class C_ConfigList;

enum configType
{
	Conf_Int,
	Conf_ComboBox,
	Conf_String,
	Conf_Slider,
	Conf_Bool
};

//-----------------------------------------------------------------------------
// Purpose: ConfigData
//-----------------------------------------------------------------------------
class CServerConfigData : public vgui2::Panel
{
public:
	CServerConfigData(vgui2::Panel* parent, char const* panelName);

	virtual	void	OnSizeChanged(int wide, int tall);

	vgui2::Panel* pControl;
	vgui2::Label* pPrompt;
	bool bIsCheat;
	configType confType;

	CServerConfigData* next;
};

//-----------------------------------------------------------------------------
// Purpose: Popup class
//-----------------------------------------------------------------------------
class C_CreateServer : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE( C_CreateServer, vgui2::Frame );

	C_CreateServer( vgui2::Panel *pParent );
	~C_CreateServer();

	void LoadMaps();
	void AddMap( const char *strMap );
	void SelectMap( int iMap );
	void RunMap( int iMap );
	void RunConfig( const char *strConfig, const char *strArg, configType cType, bool bRequireCheats = false );
	void LoadConfig( const char *strConfig, const char *strArg, configType cType );
	void SaveConfig( const char *strArg, const char *strValue, configType cType );

	void SaveConfigFile();
	const char *GetConfigFile( const char *strConfig, const char *strArg );
	void LoadConfigFile();
	void RunConfigFile();

	void ApplySchemeSettings( vgui2::IScheme *pScheme ) override;

	protected:
		virtual void OnCommand( const char * );

		vgui2::ComboBox *ui_MaxPlayers;
		vgui2::ComboBox *ui_MapList;
		KeyValues *kvMapData;
	    KeyValues *ConfigData;
	    KeyValues *ConfigSavedData;
		CServerConfigData* m_pList;
		C_ConfigList* m_pOptionsList;
};

#endif // GAME_CLIENT_ZPS_VGUI_MAINMENU_CREATE_SERVER_H
