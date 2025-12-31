#include <IEngineVGui.h>
#include <vgui_controls/PropertySheet.h>
#include "adv_options_dialog.h"
#include "client_vgui.h"
#include "gameui/gameui_viewport.h"
#include "IBaseUI.h"
#include "hud.h"
#include "cl_util.h"

#include "options_misc_root.h"
#include "options_models.h"
#include "options_chat.h"
#include "options_crosshair.h"
#include "options_scoreboard.h"
#include "options_general.h"
#include "options_about.h"

CAdvOptionsDialog::CAdvOptionsDialog(vgui2::Panel *pParent)
    : BaseClass(pParent, "AdvOptionsDialog")
{
	SetSizeable(false);
	SetProportional(true);
	SetDeleteSelfOnClose(true);

	LoadControlSettings( VGUI2_ROOT_DIR "resource/options/AdvancedOptions.res" );

	SetTitle("#ZP_Options", true);

	AddPage(new CGeneralSubOptions(this), "#ZP_AdvOptions_General");
	AddPage(new CMiscSubOptionsRoot(this), "#ZP_AdvOptions_Misc"); // Was ZP_AdvOptions_HUD
	AddPage(new CChatSubOptions(this), "#ZP_AdvOptions_Chat");
	AddPage(new CScoreboardSubOptions(this), "#ZP_AdvOptions_Scores");
	AddPage(new CCrosshairSubOptions(this), "#ZP_AdvOptions_Cross");
	//AddPage(new CModelSubOptions(this), "#ZP_AdvOptions_Models");
	AddPage(new CAboutSubOptions(this), "#ZP_AdvOptions_About");

	SetApplyButtonVisible(false);
	EnableApplyButton(false);
	GetPropertySheet()->SetTabWidth( GetScaledValue(84) );

	MoveToCenterOfScreen();
}

void CAdvOptionsDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "Apply"))
	{
		BaseClass::OnCommand("Apply");
		EnableApplyButton(true); // Apply should always be enabled
	}
	else
		BaseClass::OnCommand(command);
}
