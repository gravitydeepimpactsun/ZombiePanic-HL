//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include "AchievementList.h"
 
#include <vgui/VGUI2.h>
#include <vgui_controls/PropertyDialog.h>

#include <vgui/IVGui.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/RichText.h>

// ===================================
// Achievement Dialog
// ===================================

class C_AchievementDialog : public vgui2::PropertyDialog
{
public:
	DECLARE_CLASS_SIMPLE(C_AchievementDialog, vgui2::PropertyDialog);

	C_AchievementDialog(vgui2::Panel *pParent);

protected:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
	virtual void LoadAchievements();

private:

	vgui2::ComboBox *ui_AchvList;
	vgui2::CheckButton *ui_AchvTaken;
	vgui2::AchievementList *ui_AchvPList;
	vgui2::ImagePanel *ui_TotalProgress;
	vgui2::Label *ui_CurrentCompleted;

	int iAchievement;
	int CurrentCategory;

	bool HideAchieved;
	int miTotalAchievements;
	int miCompletedAchievements;
	int miProgressBar;
};

// ===================================
// Setup
// ===================================

class DialogAchievementData
{
private:
	EAchievements m_eAchievementID;
	char m_pchAchievementID[64];
	bool m_bAchieved;
	int m_iIconImage;
	int m_eCategory;
	EStats m_nStat;
	std::vector<EStats> m_RequiredSteps;

public:
	DialogAchievementData( EAchievements nID, const char *szName, int nCategory, EStats nStatID );
	DialogAchievementData( EAchievements nID, const char *szName, int nCategory, EStats nStatID, std::vector<EStats> nRequiredSteps );

	StatData_t GetData();
	const bool HasStatID() { return (m_nStat > INVALID_STAT) ? true : false; }
	const bool HasRequiredSteps() { return (m_RequiredSteps.size() > 0) ? true : false; }

	size_t GetRequiredStepCount() { return m_RequiredSteps.size(); }
	EStats GetRequiredStepID( int iIdx ) { return m_RequiredSteps[iIdx]; }

	const bool IsAchieved() { return m_bAchieved; }
	void SetAchieved( bool bState ) { m_bAchieved = bState; }

	int GetIconImage() { return m_iIconImage; }
	void SetIconImage( int iImage ) { m_iIconImage = iImage; }

	const EAchievements GetAchievementID() { return m_eAchievementID; }
	const char *GetAchievementName() { return m_pchAchievementID; }
	const int GetCategoryID() { return m_eCategory; }
};

#define _ACH_ID( id, category, steamstat ) DialogAchievementData( id, #id, category, steamstat )
#define _ACH_ID_LIST( id, category, steamstat, list ) DialogAchievementData( id, #id, category, steamstat, list )

DialogAchievementData GetAchievementByID( int eAchievement );