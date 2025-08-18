//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_CLIENT_ZPS_VGUI_MAINMENU_PLAYER_SELECTION_H
#define GAME_CLIENT_ZPS_VGUI_MAINMENU_PLAYER_SELECTION_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>

class C_PlayerSelection : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(C_PlayerSelection, vgui2::Frame);
	
	C_PlayerSelection(vgui2::Panel *pParent);

	// Setup the player selection
	typedef struct PLAYERMODELS_t
	{
		char			strType[250];
		char			strName[250];
		char			strImage[250];
		char			strBio[250];
	} PLAYERMODELS;

	// list of our sources
	CUtlLinkedList<PLAYERMODELS, int>		m_playermodels;

protected:
	void OnTick();
	void OnCommand(const char* pcCommand);
	void SetupAvailablePlayerModels();
	void AddPlayerOption( const char* strPrefix, const char* strModelName, const char* strModelImage, const char* strModelBio );
	void LoadSurvivorInfo( const char* strPrefix, KeyValues *kvSurvivorInfo );
	
	// Page Num
	int iPageNum;
	int iMaxPages;

	void LoadPageInfo( const char *strBio, int pagenum );
	void SetupPages();

private:
	vgui2::ComboBox *ui_SelectPlayerModel;
	vgui2::ImagePanel *ui_SelectPlayerAvatar;
	vgui2::CheckButton *ui_SelectRandomModel;
	vgui2::Label *ui_SelectPlayerBio;
	KeyValues *kvPlayerData;

	vgui2::Label *ui_Label_pagenum;

	// Buttons
	vgui2::Button *ui_Bttn_Back;
	vgui2::Button *ui_Bttn_Next;

	int iHasDescription;
	bool bReadPages;

	char strCurrentBio[52];
	std::string strLocalizationString;
};

#endif // GAME_CLIENT_ZPS_VGUI_MAINMENU_PLAYER_SELECTION_H
