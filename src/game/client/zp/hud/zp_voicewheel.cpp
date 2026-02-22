// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include <vgui/IInputInternal.h>
#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "cl_voice_status.h"
#include "zp_voicewheel.h"
#include "vgui/client_viewport.h"

#ifndef _WIN32
// Linux doesn't have this function, and wcsncmp is close enough for our purposes
#define wcsnicmp wcsncmp
#endif

DEFINE_HUD_ELEM( CHudVoiceWheel );

#define COLOR_WHITE Color( 255, 255, 255, 255 )
#define COLOR_HIGHLIGHT Color( 255, 255, 0, 255 )

ConVar cl_vwheel_adjust_x( "cl_vwheel_adjust_x", "150", 0, "Adjust the X position of the voice wheel. Use this if the voice wheel is not perfectly centered on your screen." );
ConVar cl_vwheel_adjust_y( "cl_vwheel_adjust_y", "60", 0, "Adjust the Y position of the voice wheel. Use this if the voice wheel is not perfectly centered on your screen." );
ConVar cl_vwheel_adjust_triangle( "cl_vwheel_adjust_triangle", "5", 0, "Adjust the position of the triangle that points to the current option. Use this if the triangle is not perfectly centered on the options." );

extern void Input_ClearAttackState();

CHudVoiceWheel::CHudVoiceWheel()
    : CHudElemBase()
    , BaseClass( NULL, "HudVoiceWheel" )
{
	SetParent( g_pViewport );
	SetProportional( true );
	SetToggleState( false );
	// Load voice wheel options
	LoadVoiceOptions();

	MakePopup();
	SetZPos(-30);

	SetKeyBoardInputEnabled( false );
}

void CHudVoiceWheel::Init()
{
	SetToggleState( false );
}

void CHudVoiceWheel::VidInit()
{
	SetToggleState( false );
}

void CHudVoiceWheel::LoadVoiceOptions()
{
	// Let's load the KeyValues from file "scripts/voicewheel.txt"
	// The format of the file is as follows:
	// "VoiceWheel"
	// {
	//     "Option1"
	//     {
	//         "text" "Option 1"
	//         "command" "say Hello"
	//		}
	//     "Option2"
	//     {
	//         "text" "Option 2"
	//         "command" "say Goodbye"
	//		}
	// }
	KeyValuesAD pKeyValues( "VoiceWheel" );
	if ( !pKeyValues->LoadFromFile( g_pFullFileSystem, "scripts/voicewheel.txt", "GAME" ) )
	{
		Warning( "Failed to load voice wheel options from file!\n" );
		return;
	}
	for ( KeyValues *pOption = pKeyValues->GetFirstSubKey(); pOption != nullptr; pOption = pOption->GetNextKey() )
	{
		// If we have more than 12 options, we will ignore the rest, since we can only display 8 options on the wheel
		if ( m_VoiceWheelOptions.size() >= 12 )
		{
			Msg( "Voice wheel options limit reached! Ignoring option: %s\n", pOption->GetName() );
			continue;
		}

		VoiceWheelOption_t option;
		V_strcpy_safe( option.text, pOption->GetString( "text", "Example" ) );
		V_strcpy_safe( option.command, pOption->GetString( "command", "example" ) );
		m_VoiceWheelOptions.push_back( option );
	}
}

void CHudVoiceWheel::ApplySchemeSettings( vgui2::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	vgui2::HFont font = pScheme->GetFont( m_FontText, IsProportional() );
	if ( font != vgui2::INVALID_FONT )
		m_hFont = font;

	// Make sure we only load this once.
	if ( m_iBackgroundTextureID == -1 )
	{
		m_iBackgroundTextureID = vgui2::surface()->CreateNewTextureID();
		vgui2::surface()->DrawSetTextureFile( m_iBackgroundTextureID, "vgui/hud/voicewheel", true, false );
	}

	SetBgColor( Color(0, 0, 0, 0) );
}

void CHudVoiceWheel::SetToggleState( bool bState )
{
	// If not allowed, then don't.
	if ( !IsAllowedToDraw( true ) )
		bState = false;

	m_bActive = bState;
	if ( bState )
		vgui2::SETUP_PANEL( this );
	SetMouseInputEnabled( bState );
	if ( bState )
	{
		RequestFocus();
		vgui2::surface()->CalculateMouseVisible();
		// Move the cursor to the center of the screen, since that's where the voice wheel is
		int x, y;
		vgui2::surface()->GetScreenSize( x, y );
		vgui2::input()->SetCursorPos( x / 2, y / 2 );
		MoveToFront();
	}
}

bool CHudVoiceWheel::IsAllowedToDraw( const bool &bOnToggleCheck )
{
	if ( gHUD.m_RoundState < ZP::RoundState::RoundState_WaitingForPlayers ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_TEAM ) ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_MOTD ) ) return false;
	if ( !gEngfuncs.GetLocalPlayer() || gEngfuncs.GetLocalPlayer()->index <= 0 ) return false;
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer->IsConnected() ) return false;
	if ( localplayer->GetTeamNumber() != ZP::TEAM_SURVIVIOR ) return false;
	if ( bOnToggleCheck ) return true;
	return m_bActive;
}

void CHudVoiceWheel::Paint()
{
	if ( !IsAllowedToDraw( false ) )
		return;

	int x, y, w, h;
	GetBounds( x, y, w, h );

	// We will draw the voice wheel in the center of the screen, so we need to offset it by half of its size
	// Since we want to draw the text (aka, our options) above the wheel, we will also offset it by the height of the text
	// When we hover our mouse over the option, we need to highlight it, so we will also offset it by the height of the highlight
	// In short, we need to make it easy for the user to know "oh, I can select this option" and "oh, I have selected this option"
	int cornerWide, cornerTall;
	GetCornerTextureSize( cornerWide, cornerTall );
	int textTall = vgui2::surface()->GetFontTall( m_hFont );
	int highlightTall = textTall + GetScaledValue( 10 ); // Add some padding to make it look nicer
	int totalOffset = cornerTall / 2 + textTall + highlightTall;
	int centerX = x + w / 2;
	int centerY = y + h / 2 - totalOffset;

	// Draw the background texture
	vgui2::surface()->DrawSetTexture( m_iBackgroundTextureID );
	vgui2::surface()->DrawSetColor( Color(255, 255, 255, 255) );
	vgui2::surface()->DrawTexturedRect( centerX - cornerWide / 2, centerY - cornerTall / 2, centerX + cornerWide / 2, centerY + cornerTall / 2 );

	// Draw the options
	for ( size_t i = 0; i < m_VoiceWheelOptions.size(); i++ )
	{
		// Calculate the position of this option based on its index
		const VoiceWheelOption_t &option = m_VoiceWheelOptions[i];

		// Convert the option text to unicode for drawing
		wchar_t szText[512];
		szText[0] = 0;
		g_pVGuiLocalize->ConvertANSIToUnicode( option.text, szText, sizeof( szText ) );

		// If the text starts with '#' then its a localization string.
		if ( option.text[0] == '#' )
		{
			wchar_t *temp = g_pVGuiLocalize->Find( option.text );
			if ( temp )
			{
				// Found the text, copy it to our working buffer
				// But we need to make sure we can't cause a buffer overflow
				V_wcsncpy( szText, temp, sizeof( szText ) );
			}
		}

		// Get the size of the text so we can center it on the option position
		int textWidth, textHeight;
		vgui2::surface()->GetTextSize( m_hFont, szText, textWidth, textHeight );

		// Calculate the angle for this option. We want to start at the top (270 degrees) and go clockwise, so we will add 270 degrees to the angle
		float angle = (float)i / m_VoiceWheelOptions.size() * 360.0f + 270.0f;
		float radians = angle * M_PI / 180.0f;

		// Calculate the position of this option based on the angle and the radius of the circle we want to draw it on
		int radius = cornerWide / 2 + GetScaledValue( 5 );
		int optionX = centerX + cos( radians ) * radius - textWidth / 2;
		int optionY = centerY + sin( radians ) * radius - textHeight / 2;

		// The text may draw over each other, so let's adjust their positions using GetScaledValue to add some spacing between them.
		optionX += GetScaledValue( cl_vwheel_adjust_x.GetInt() ) * cos( radians );
		optionY += GetScaledValue( cl_vwheel_adjust_y.GetInt() ) * sin( radians );

		// Check if the mouse is hovering over this option
		int mouseX, mouseY;
		vgui2::input()->GetCursorPos( mouseX, mouseY );

		// Is the mouse hovering over this option? We need to account for the size of the text, since the option is not just a point, but an area around the text
		bool isHovering = mouseX >= optionX - 5 && mouseX <= optionX + textWidth + 5 && mouseY >= optionY - 5 && mouseY <= optionY + textTall + 5;

		// Do another hover check, but this time we will check if the mouse is hovering over the area "box" of the option, which is a larger area that encompasses the text and some padding around it. This will make it easier for the user to select the option, since they don't have to be pixel perfect with their mouse movement. But we don't want to make it a simple box, but more of a triangle that points towards the center of the wheel, since that will make it more clear that the option is associated with the wheel and not just floating around it. So we will check if the mouse is within a triangle that has its base at the option position and its tip at the center of the wheel.
		bool isHoveringBox = false;
		// Calculate the vertices of the triangle
		int triangleX1 = optionX - GetScaledValue( cl_vwheel_adjust_triangle.GetInt() );
		int triangleY1 = optionY - GetScaledValue( cl_vwheel_adjust_triangle.GetInt() );
		int triangleX2 = optionX + textWidth + GetScaledValue( cl_vwheel_adjust_triangle.GetInt() );
		int triangleY2 = optionY - GetScaledValue( cl_vwheel_adjust_triangle.GetInt() );
		int triangleX3 = centerX;
		int triangleY3 = centerY;
		// Check if the mouse is within the triangle using barycentric coordinates
		float denominator = (float)( ( triangleY2 - triangleY3 ) * ( triangleX1 - triangleX3 ) + ( triangleX3 - triangleX2 ) * ( triangleY1 - triangleY3 ) );
		float a = ( ( triangleY2 - triangleY3 ) * ( mouseX - triangleX3 ) + ( triangleX3 - triangleX2 ) * ( mouseY - triangleY3 ) ) / denominator;
		float b = ( ( triangleY3 - triangleY1 ) * ( mouseX - triangleX3 ) + ( triangleX1 - triangleX3 ) * ( mouseY - triangleY3 ) ) / denominator;
		float c = 1.0f - a - b;
		isHoveringBox = a >= 0 && b >= 0 && c >= 0;
		isHovering = isHovering || isHoveringBox; // If we are hovering over the box, we should also consider it as hovering over the option

		// Draw the text of this option
		vgui2::surface()->DrawSetTextFont( m_hFont );
		vgui2::surface()->DrawSetTextColor( isHovering ? COLOR_HIGHLIGHT : COLOR_WHITE );
		vgui2::surface()->DrawSetTextPos( optionX, optionY );
		vgui2::surface()->DrawPrintText( szText, wcsnicmp( szText, L"\n", 1 ) ? wcslen( szText ) : wcslen( szText ) - 1 );

		// If we are hovering and the user clicks, execute the command of this option
		if ( isHovering && vgui2::input()->IsMouseDown( vgui2::MouseCode::MOUSE_LEFT ) )
		{
			// Stop pressing it down, so we don't accidentally shoot our weapon on the next frame...
			Input_ClearAttackState();
			std::string szCmd( "vocalize " + std::string( option.command ) );
			EngineClientCmd( szCmd.c_str() );
			SetToggleState( false ); // Close the voice wheel after selecting an option
		}
	}
}
