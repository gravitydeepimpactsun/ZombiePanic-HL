#include <IEngineVGui.h>
#include <vgui_controls/PropertySheet.h>
#include "CWorkshopDialog.h"
#include "client_vgui.h"
#include "gameui/gameui_viewport.h"
#include "IBaseUI.h"
#include "hud.h"
#include "cl_util.h"

#include "CWorkshopSubList.h"
#include "CWorkshopSubUploaded.h"
#include "CWorkshopSubUpload.h"

CWorkshopDialog::CWorkshopDialog(vgui2::Panel *pParent)
    : BaseClass(pParent, "WorkshopDialog")
{
	SetSizeable(false);
	SetProportional(true);
	SetDeleteSelfOnClose(true);

	LoadControlSettings( VGUI2_ROOT_DIR "resource/workshop/base.res" );

	SetTitle("#ZP_Workshop", true);

	CWorkshopSubUpload *pUploadPage = new CWorkshopSubUpload(this);
	CWorkshopSubUploaded *pUploaded = new CWorkshopSubUploaded(this);
	pUploaded->SetPropertyDialog( this );
	pUploaded->SetUploadPage( pUploadPage );

	AddPage(new CWorkshopSubList(this), "#ZP_Workshop_Tab_Subscribed");
	AddPage(pUploaded, "#ZP_Workshop_Tab_Uploaded");
	AddPage(pUploadPage, "#ZP_Workshop_Tab_Upload");

	SetOKButtonVisible(false);
	SetApplyButtonVisible(false);
	EnableApplyButton(false);
	SetCancelButtonText("#PropertyDialog_Close");
	GetPropertySheet()->SetTabWidth( GetScaledValue( 84 ) );
	MoveToCenterOfScreen();
}

void CWorkshopDialog::OnCommand(const char *command)
{
	BaseClass::OnCommand(command);
}
