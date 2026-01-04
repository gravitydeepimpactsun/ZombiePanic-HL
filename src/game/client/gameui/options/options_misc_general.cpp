#include <vgui_controls/Label.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/ComboBox.h>
#include <KeyValues.h>
#include <MinMax.h>
#include "client_vgui.h"
#include "options_misc_general.h"
#include "cvar_text_entry.h"
#include "cvar_color.h"
#include "cvar_check_button.h"
#include "hud.h"
#include "hud_renderer.h"

CMiscSubOptions::CMiscSubOptions(vgui2::Panel *parent)
    : BaseClass(parent, "MiscSubOptions")
{
	SetSize(100, 100); // Silence "parent not sized yet" warning

	m_pZMVision = new CCvarCheckButton(this, "ZMVision", "#ZP_AdvOptions_General_KeepZMVision", "cl_keepzombovision");
	m_pAutoWeaponSwitch = new CCvarCheckButton(this, "AutoSwitch", "#ZP_AdvOptions_General_AutoSwitch", "cl_autopickup");
	m_pPanicToMelee = new CCvarCheckButton(this, "PanicToMelee", "#ZP_AdvOptions_General_PanicToMelee", "cl_panictomelee");
	m_pHideScoreBoardIcon = new CCvarCheckButton(this, "HideScoreBoardIcon", "#ZP_AdvOptions_General_HideScoreBoardIcon", "cl_hidescoreboardicon");
	m_pZombieLadderDrawMode = new CCvarCheckButton(this, "ZombieLadderDrawMode", "#ZP_AdvOptions_General_ZombieLadderDrawMode", "cl_zladderdrawmode");

	LoadControlSettings(VGUI2_ROOT_DIR "resource/options/MiscSubOptions.res");
}

void CMiscSubOptions::OnResetData()
{
	m_pZMVision->ResetData();
	m_pAutoWeaponSwitch->ResetData();
	m_pPanicToMelee->ResetData();
	m_pHideScoreBoardIcon->ResetData();
	m_pZombieLadderDrawMode->ResetData();
}

void CMiscSubOptions::OnApplyChanges()
{
	m_pZMVision->ApplyChanges();
	m_pAutoWeaponSwitch->ApplyChanges();
	m_pPanicToMelee->ApplyChanges();
	m_pHideScoreBoardIcon->ApplyChanges();
	m_pZombieLadderDrawMode->ApplyChanges();
}
