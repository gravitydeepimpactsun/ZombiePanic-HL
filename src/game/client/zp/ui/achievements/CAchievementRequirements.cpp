//========= Copyright (c) 2025 Monochrome Games, All rights reserved. ============//

#include "vgui/MouseCode.h"
#include "vgui/IInput.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"

#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Controls.h"
#include "CAchievementRequirements.h"
#include "zp/ui/workshop/WorkshopItemList.h"

#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui2;

CBasicItemCombo::CBasicItemCombo( Panel* parent, char const *panelName, bool bEarned1, const char *szLabel1, bool bEarned2, const char *szLabel2 ) : BaseClass( parent, panelName )
{
	SetPaintBackgroundEnabled( false );

	m_Label1 = new vgui2::Label( this, "label1", szLabel1 );
	m_Label2 = new vgui2::Label( this, "label2", szLabel2 ? szLabel2 : "" );

	m_Label1->SetEnabled( bEarned1 );
	m_Label2->SetEnabled( bEarned2 );

	m_Label1->SetContentAlignment( vgui2::Label::Alignment::a_west );
	m_Label2->SetContentAlignment( vgui2::Label::Alignment::a_west );

	m_Label1->SetPaintBackgroundEnabled( false );
	m_Label2->SetPaintBackgroundEnabled( false );

	m_Image1 = new vgui2::ImagePanel( this, "img1" );
	m_Image1->SetImage( bEarned1 ? "resource/icon_checked_noborder" : "" );
	m_Image1->SetShouldScaleImage( true );
	m_Image2 = new vgui2::ImagePanel( this, "img2" );
	m_Image2->SetImage( bEarned2 ? "resource/icon_checked_noborder" : "" );
	m_Image2->SetShouldScaleImage( true );

	UpdateLayout();
}

vgui2::CBasicItemCombo::CBasicItemCombo( vgui2::Panel *parent, char const *panelName, bool bEarned, const char *szLabel ) : BaseClass( parent, panelName )
{
	SetPaintBackgroundEnabled( false );

	m_Label1 = new vgui2::Label( this, "label1", szLabel );
	m_Label1->SetContentAlignment( vgui2::Label::Alignment::a_west );
	m_Label1->SetEnabled( bEarned );
	m_Label1->SetPaintBackgroundEnabled( false );

	m_Image1 = new vgui2::ImagePanel( this, "img1" );
	m_Image1->SetImage( bEarned ? "resource/icon_checked_noborder_green" : "" );
	m_Image1->SetShouldScaleImage( true );

	m_Label2 = nullptr;
	m_Image2 = nullptr;

	UpdateLayout();
}

bool vgui2::CBasicItemCombo::HasSecondItem() const
{
	if ( !m_Label2 ) return false;
	char szLabel[64];
	m_Label2->GetText( szLabel, sizeof(szLabel) );
	return !vgui2::FStrEq( szLabel, "" );
}

void vgui2::CBasicItemCombo::SetSecondItem( bool bEarned, const char *szLabel )
{
	if ( !m_Label2 )
		m_Label2 = new vgui2::Label( this, "label2", szLabel );
	else
		m_Label2->SetText( szLabel );
	m_Label2->SetContentAlignment( vgui2::Label::Alignment::a_west );
	m_Label2->SetEnabled( bEarned );
	m_Label2->SetPaintBackgroundEnabled( false );

	if ( !m_Image2 )
		m_Image2 = new vgui2::ImagePanel( this, "img2" );

	m_Image2->SetImage( bEarned ? "resource/icon_checked_noborder_green" : "" );
	m_Image2->SetShouldScaleImage( true );
}

void vgui2::CBasicItemCombo::UpdateLayout()
{
	int w, t;
	GetSize( w, t );
	w = (w / 2);
	int space = ( vgui2::scheme()->GetProportionalScaledValue( 5 ) / 2 );
	int x = 0;
	int img_size = 24;
	if ( m_Image1 )
	{
		m_Image1->SetBounds( 0, 0, img_size, img_size );
		x = m_Image1->GetWide() + 2;
	}
	if ( m_Label1 )
	{
		m_Label1->SetPos( x, 0 );
		m_Label1->SetSize( w - space, t - 8 );
	}

	if ( m_Image2 )
		m_Image2->SetBounds( w + space, 0, img_size, img_size );
	if ( m_Label2 )
	{
		m_Label2->SetPos( w + space + x, 0 );
		m_Label2->SetSize( w - space, t - 8 );
	}
}

//
// ==============================================================================
// ==============================================================================
//

CAchievementRequirementsHolder::CAchievementRequirementsHolder( vgui2::Panel *parent, char const *panelName )
    : Panel( parent, panelName )
{
	SetBounds(0, 0, 100, 100);
	SetPaintBackgroundEnabled( false );

	m_pButton = new vgui2::ButtonImage(this, "ButtonImage", "resource/icon_plus", this, "ToggleOption");
	m_pButton->SetBounds( 5, 5, 18, 18 );
	m_pButton->SetImage( "resource/icon_plus" );
	m_pLabel = new vgui2::Label(this, "Label", "#ZP_UI_Achievements_ShowDetails");
	m_pLabel->SetBounds( 28, 0, 150, 30 );
	m_pLabel->SetPaintBackgroundEnabled( false );

	m_pList.clear();
	m_bShouldExpand = true;
	m_iSizeHeightDefault = m_iSizeHeight = m_pLabel->GetTall() + 5;
}

vgui2::CAchievementRequirementsHolder::~CAchievementRequirementsHolder()
{
	for (size_t i = 0; i < m_pList.size(); i++)
	{
		CBasicItemCombo *pComboItem = m_pList[i];
		if ( pComboItem )
			pComboItem->MarkForDeletion();
	}
	m_pList.clear();
	InvalidateLayout();
}


void CAchievementRequirementsHolder::AddItem( bool obtained, const char *text )
{
	// Before we add a new entry, check if we have an item that wants a second item.
	for (size_t i = 0; i < m_pList.size(); i++)
	{
		CBasicItemCombo *pComboItem = m_pList[i];
		if ( pComboItem && !pComboItem->HasSecondItem() )
		{
			pComboItem->SetSecondItem( obtained, text );
			return;
		}
	}
	int ypos = m_pLabel->GetYPos() + m_pLabel->GetTall() + 5;
	ypos += (32 * m_pList.size()) + 5;
	int xpos = m_pLabel->GetXPos();
	CBasicItemCombo *pComboItem = new CBasicItemCombo( this, "ComboItem", obtained, text );
	pComboItem->SetBounds( xpos, ypos, 500, 32 );
	pComboItem->UpdateLayout();
	m_pList.push_back( pComboItem );
}

void CAchievementRequirementsHolder::OnCommand(const char *szCommand)
{
	if (!Q_stricmp(szCommand, "ToggleOption"))
		UpdateSize( m_bShouldExpand );
	else
		BaseClass::OnCommand( szCommand );
}

void CAchievementRequirementsHolder::UpdateSize( bool bExpand )
{
	if ( bExpand )
	{
		m_pButton->SetImage( "resource/icon_minus" );
		m_pLabel->SetText( "#ZP_UI_Achievements_HideDetails" );
	}
	else
	{
		m_pButton->SetImage( "resource/icon_plus" );
		m_pLabel->SetText( "#ZP_UI_Achievements_ShowDetails" );
	}

	// Toggle the boolean
	m_bShouldExpand = !m_bShouldExpand;

	int iHeight = m_iSizeHeightDefault;

	// Toggle our items visibility
	for ( size_t i = 0; i < m_pList.size(); i++ )
	{
		CBasicItemCombo *pComboItem = m_pList[i];
		if ( !pComboItem ) continue;
		pComboItem->UpdateLayout();
		iHeight += pComboItem->GetTall();
	}

	// Update our size!
	if ( bExpand )
		m_iSizeHeight = iHeight;
	else
		m_iSizeHeight = m_iSizeHeightDefault;

	InvalidateLayout();

	PostActionSignal( new KeyValues("RequirementStepSizeUpdate") );
}
