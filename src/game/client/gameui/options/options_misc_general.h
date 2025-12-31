#ifndef CMISCSUBOPTIONS_H
#define CMISCSUBOPTIONS_H
#include <vgui_controls/PropertyPage.h>
#include "gameui/options/cvar_combo_box.h"

class CCvarCheckButton;

class CMiscSubOptions : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CMiscSubOptions, vgui2::PropertyPage);

public:
	CMiscSubOptions(vgui2::Panel *parent);

	void OnResetData() override;
	void OnApplyChanges() override;

private:
	CCvarCheckButton *m_pZMVision = nullptr;
	CCvarCheckButton *m_pAutoWeaponSwitch = nullptr;
	CCvarCheckButton *m_pPanicToMelee = nullptr;
	CCvarCheckButton *m_pHideScoreBoardIcon = nullptr;
};

#endif
