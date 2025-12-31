// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <stdio.h>
#include <utlsymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInputInternal.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include <vgui_controls/ButtonImage.h>
#include <vgui_controls/FocusNavGroup.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui2;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( ButtonImage, ButtonImage );

ButtonImage::ButtonImage(Panel *parent, const char *panelName, const char *image, Panel *pActionSignalTarget, const char *pCmd)
    : Button(parent, panelName, "", pActionSignalTarget, pCmd)
{
	pImage = new vgui2::ImagePanel( this, "Image" );
	pImage->SetSize( 0, 0 );
	pImage->SetPos( 0, 0 );
	pImage->DisableMouseInputForThisPanel( true );
	pImage->SetShouldScaleImage( true );
}

ButtonImage::~ButtonImage()
{
	if ( pImage )
		pImage->DeletePanel();
}

void vgui2::ButtonImage::SetImage( const char *szImage )
{
	pImage->SetImage( szImage );
}

void ButtonImage::Paint( void )
{
	BaseClass::Paint();

	if ( !BaseClass::ShouldPaint() )
		return; 

	int wide, tall;
	GetSize( wide, tall );
	pImage->SetSize( wide, tall );
}

void vgui2::ButtonImage::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	const char *imageText = inResourceData->GetString( "image", NULL );
	if ( imageText )
	{
		// Clear it
		SetText( "" );
		SetImage( imageText );
	}
}
