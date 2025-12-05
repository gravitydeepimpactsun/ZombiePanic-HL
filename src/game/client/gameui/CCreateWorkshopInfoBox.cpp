// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui/IVGui.h>
#include "CCreateWorkshopInfoBox.h"
#include "gameui_viewport.h"
#include "client_vgui.h"

CCreateWorkshopInfoBox::CCreateWorkshopInfoBox(vgui2::Panel *pParent)
    : BaseClass(pParent, "CreateWorkshopInfoBox")
{
	SetTitle("", false);
	SetSizeable( false );
	SetSize( 100, 80 );
	SetPos( 0, 0 );
	SetTitleBarVisible( false );
	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_pText = new vgui2::Label( this, "Text", "My Example Addon" );
	m_pState = new vgui2::Label( this, "State", "#ZP_Workshop_InfoBox_GatheringData" );
	m_pProgressBar = new vgui2::ProgressBar( this, "Progress" );

	SetScheme( vgui2::scheme()->LoadSchemeFromFile( VGUI2_ROOT_DIR "resource/ClientSourceSchemeBase.res", "ClientSourceSchemeBase" ) );

	LoadControlSettings( VGUI2_ROOT_DIR "resource/workshop/infobox.res" );

	SetScheme( CGameUIViewport::Get()->GetScheme() );
	InvalidateLayout();

	m_state = State_GatheringData;
	vgui2::ivgui()->AddTickSignal( GetVPanel(), 1000 );

	MoveToFront();

	m_RemoveTime = -1.0f;
}

void CCreateWorkshopInfoBox::SetData( const char *szString, WorkshopInfoBoxState nState )
{
	if ( m_state >= State_DownloadingMapContent && nState < State_DownloadingMapContent ) return;
	m_state = nState;
	m_pText->SetColorCodedText( szString );
	switch ( m_state )
	{
		case State_GatheringData: m_pState->SetText( "#ZP_Workshop_InfoBox_GatheringData" ); break;
		case State_Downloading: m_pState->SetText( "#ZP_Workshop_InfoBox_Downloading" ); break;
		case State_Updating: m_pState->SetText( "#ZP_Workshop_InfoBox_Updating" ); break;
		case State_Dismounting: m_pState->SetText( "#ZP_Workshop_InfoBox_Dismounting" ); break;
		case State_Mounting: m_pState->SetText( "#ZP_Workshop_InfoBox_Mounting" ); break;
		case State_DownloadingMapContent: m_pState->SetText( "#ZP_Workshop_InfoBox_DownloadingWorkshopContent" ); break;
		//default: m_pState->SetText( "Example Text." ); break;
	}
	if ( m_state == WorkshopInfoBoxState::State_Done )
		m_RemoveTime = 2.0f;
	// Delay the disconnect. Because we may be changing level
	else if ( m_state == WorkshopInfoBoxState::State_DownloadingMapContentDisconnect )
		m_RemoveTime = 1.0f;
}

void CCreateWorkshopInfoBox::SetProgressState( float flProgress )
{
	m_pProgressBar->SetProgress( flProgress );
}

void CCreateWorkshopInfoBox::OnTick()
{
	BaseClass::OnTick();
	if ( m_RemoveTime <= -1 ) return;
	if ( m_RemoveTime <= 0 )
	{
		// Disconnect
		if ( m_state == State_DownloadingMapContentDisconnect )
		{
			gEngfuncs.pfnClientCmd( "disconnect\n" );
			m_state = State_DownloadingMapContentComplete;
			// Wait 1 more second
			m_RemoveTime = 1.0f;
			return;
		}
		// Reconnect to the server
		else if ( m_state == State_DownloadingMapContentComplete )
			gEngfuncs.pfnClientCmd( "retry\n" );
		Close();
	}
	m_RemoveTime--;
}