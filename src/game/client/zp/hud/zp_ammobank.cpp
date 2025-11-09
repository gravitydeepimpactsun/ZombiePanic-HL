// ============== Copyright (c) 2025 Monochrome Games ============== \\

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>
#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "../../hud/ammohistory.h"
#include "zp_ammobank.h"
#include "vgui/client_viewport.h"

DEFINE_HUD_ELEM(CHudAmmoBank);

ZP::ColorGradient::RedYellowGreen gColorGradientRedYellowGreen;

extern int g_ActiveAmmoIndex;
static ConVar cl_hideemptyammo( "cl_hideemptyammo", "0", FCVAR_BHL_ARCHIVE, "Hides empty ammo from the ammobank" );

/** Invisible color (black with no alpha). */
#define COLOR_INVISIBLE			Color( 0, 0, 0, 0 )
#define COLOR_DARK_GREY_A		Color( 64, 64, 64, 200 )
#define COLOR_YELLOW_F			Color( 255, 178, 0, 50 )
#define COLOR_YELLOW			Color( 255, 178, 0, 255 )
#define COLOR_WHITE				Color( 255, 255, 255, 255 )

CHudAmmoBank::CHudAmmoBank()
    : vgui2::Panel(NULL, "HudAmmoBank")
{
	SetParent( g_pViewport );

	SetScheme( "ClientScheme" );

	SetPaintBackgroundEnabled( true );

	m_iSelectedAmmoToDrop = 0;
	m_bHasPanelRect = false;

	panelrect.x = GetWide() / 2;
	panelrect.y = GetTall() / 2;
	panelrect.width = 0;
	panelrect.height = 0;

	m_pBackground = new vgui2::ImagePanel( this, "background" );
	m_pBackground->SetFillColor( COLOR_INVISIBLE );

	m_pWeightText = new vgui2::Label( m_pBackground, "weight_text", "#ZP_HUD_Ammo_DropAmount_Weight" );
	m_pWeightText->SetContentAlignment( vgui2::Label::a_northwest );

	m_pWeightStatus = new vgui2::Label( m_pBackground, "weight_status", "0lb" );
	m_pWeightStatus->SetContentAlignment( vgui2::Label::a_northeast );

	char TempBuffer[32];
	for ( int index = 0; index < 5; index++ )
	{
		Q_snprintf( TempBuffer, sizeof( TempBuffer ), "ammo_label%d", index + 1 );
		m_pAmmoCount[index] = new vgui2::Label( m_pBackground, TempBuffer, "999" );
		m_pAmmoCount[index]->SetContentAlignment( vgui2::Label::a_northeast );

		Q_snprintf( TempBuffer, sizeof( TempBuffer ), "ammo_name%d", index + 1 );
		m_pAmmoName[index] = new vgui2::Label( m_pBackground, TempBuffer, GetAmmoName( index ).c_str() );
		m_pAmmoName[index]->SetContentAlignment( vgui2::Label::a_northwest );
	}

	InvalidateLayout();

	HookMessage<&CHudAmmoBank::MsgFunc_AmmoBankUpdate>("AmmoBank");
}

void CHudAmmoBank::VidInit()
{
	int cornerWide, cornerTall;
	GetCornerTextureSize( cornerWide, cornerTall );
	m_bHasPanelRect = false;
	UpdateVisibility( false );
}

void CHudAmmoBank::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBgColor( COLOR_INVISIBLE );
	SetPaintBackgroundType( 1 );
	PaintBackground();

	vgui2::HFont fontNormal = pScheme->GetFont( "Default" );
	vgui2::HFont fontSmall = pScheme->GetFont( "DefaultSmall" );

	for ( int index = 0; index < 5; index++ )
	{
		m_pAmmoCount[index]->SetFont( fontNormal );
		m_pAmmoName[index]->SetFont( fontNormal );
	}

	m_pWeightText->SetFont( fontSmall );
	m_pWeightStatus->SetFont( fontSmall );

	UpdateVisibility( false );
}

bool CHudAmmoBank::IsAllowedToDraw()
{
	if ( gHUD.m_RoundState < ZP::RoundState::RoundState_RoundHasBegunPost ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_TEAM ) ) return false;
	if ( g_pViewport->IsVGUIVisible( MENU_MOTD ) ) return false;
	if ( gEngfuncs.GetLocalPlayer()->index <= 0 ) return false;
	CPlayerInfo *localplayer = GetPlayerInfo( gEngfuncs.GetLocalPlayer()->index );
	if ( !localplayer->IsConnected() ) return false;
	if ( localplayer->GetTeamNumber() != ZP::TEAM_SURVIVIOR ) return false;
	return hud_draw.GetFloat() > 0.0f;
}

void CHudAmmoBank::PaintBackground()
{
	if ( !IsAllowedToDraw() ) return;
	// hurrdidurr
	DrawBox(
		0, 0,
	    GetWide(), GetTall(),
		Color( 15, 15, 15, 160 ),
		1.0f
	);
}

void CHudAmmoBank::Paint()
{
	if ( !IsAllowedToDraw() )
	{
		UpdateVisibility( false );
		return;
	}

	m_pBackground->SetSize( GetWide(), GetTall() );
	m_pBackground->SetPos( 0, 0 );

	const int TextBuffer = 8;

	int TextX = TextBuffer;
	int TextY = TextBuffer;

	panelrect.width = XRES( m_pAmmoBank_width );
	panelrect.height = (TextBuffer * 2);

	int TextWideDefault = XRES( m_pText_wide );
	int TextWideAmmo = XRES( m_pText_wide_ammo );
	int TextTall = YRES( m_pText_tall );

	// Draw our stuff
	for ( int index = 0; index < 5; index++ )
	{
		int iAmmoCount = gWR.CountAmmo( AmmoDropToAmmoIndex( index ) );

		if ( cl_hideemptyammo.GetBool() && iAmmoCount <= 0 )
		{
			m_pAmmoName[index]->SetVisible( false );
			m_pAmmoCount[index]->SetVisible( false );
			m_pAmmoName[index]->SetFgColor( COLOR_INVISIBLE );
			m_pAmmoCount[index]->SetFgColor( COLOR_INVISIBLE );
			continue;
		}
		else
		{
			m_pAmmoName[index]->SetVisible( true );
			m_pAmmoCount[index]->SetVisible( true );

			bool bIsSelected = ( index == m_iSelectedAmmoToDrop );

			// Draw color
			if ( iAmmoCount > 0 )
			{
				m_pAmmoName[index]->SetFgColor( bIsSelected ? COLOR_YELLOW : COLOR_WHITE );
				m_pAmmoCount[index]->SetFgColor( bIsSelected ? COLOR_YELLOW : COLOR_WHITE );
			}
			else
			{
				m_pAmmoName[index]->SetFgColor( bIsSelected ? COLOR_YELLOW_F : COLOR_DARK_GREY_A );
				m_pAmmoCount[index]->SetFgColor( bIsSelected ? COLOR_YELLOW_F : COLOR_DARK_GREY_A );
			}
		}

		// Draw the text buffer and grab our text size
		m_pAmmoName[index]->SetSize( TextWideDefault, TextTall );
		m_pAmmoName[index]->SetPos( TextX, TextY );

		// Draw the ammo buffer and grab our text size
		m_pAmmoCount[index]->SetText( std::to_string( iAmmoCount ).c_str() );
		m_pAmmoCount[index]->SetSize( TextWideAmmo, TextTall );
		m_pAmmoCount[index]->SetPos( TextX + TextWideDefault + (TextBuffer * 2), TextY );

		// Set the new po
		TextY += TextTall + 2;
		panelrect.height += TextTall + 2;
	}

	// Increase it before we show weight
	TextY += TextTall + 2;
	panelrect.height += TextTall + 2;

	// Weight status
	{
		// =======================================================================================
		m_pWeightText->SetSize( TextWideDefault, TextTall );
		m_pWeightText->SetPos( TextX, TextY );

		// Weight status
		float flWeight = GetCarry();

		// Set the buffer msg
		char Buffer[128];
		Q_snprintf( Buffer, sizeof( Buffer ), "%.0f Lb", flWeight );

		// Set our weight status
		m_pWeightStatus->SetText( Buffer );

		// Set color value here (green > red)
		float ratio = 1.0f - (flWeight / GetMaxCarry());
		m_pWeightStatus->SetFgColor( gColorGradientRedYellowGreen.GetColorForValue( ratio ) );

		m_pWeightStatus->SetPos( TextX + TextWideDefault + (TextBuffer * 2), TextY );
		m_pWeightStatus->SetSize( TextWideAmmo, TextTall );

		// Set the new po
		TextY += TextTall + 2;
		panelrect.height += TextTall + 2;
		// =======================================================================================
	}

	int xy[2];
	switch ( gHUD.m_iRes )
	{
		default:
		case 320: xy[0] = m_pAmmoBank_up_x05; xy[1] = m_pAmmoBank_left_x05; break;
		case 640: xy[0] = m_pAmmoBank_up_x1; xy[1] = m_pAmmoBank_left_x1; break;
		case 1280: xy[0] = m_pAmmoBank_up_x2; xy[1] = m_pAmmoBank_left_x2; break;
		case 2560: xy[0] = m_pAmmoBank_up_x4; xy[1] = m_pAmmoBank_left_x4; break;
	}

	int panel_wide = panelrect.width + xy[1];
	int panel_tall = panelrect.height + xy[0];

	panelrect.x = ScreenWidth - panel_wide;
	panelrect.y = ScreenHeight - panel_tall;

	UpdateVisibility( m_bHasPanelRect );

	SetSize( panelrect.width, panelrect.height );
	SetPos( panelrect.x, panelrect.y );

	// On next frame, we will draw it.
	if ( !m_bHasPanelRect )
		m_bHasPanelRect = true;
}

int CHudAmmoBank::MsgFunc_AmmoBankUpdate(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	m_iSelectedAmmoToDrop = READ_SHORT();
	return 1;
}

int CHudAmmoBank::AmmoDropToAmmoIndex( int index )
{
	switch ( index )
	{
		// 9mm
		case 0: return ZPAmmoTypes::AMMO_PISTOL;
		// 357
	    case 1: return ZPAmmoTypes::AMMO_MAGNUM;
		// buckshot
	    case 2: return ZPAmmoTypes::AMMO_SHOTGUN;
		// 556ar
	    case 3: return ZPAmmoTypes::AMMO_RIFLE;
		// longrifle
	    case 4: return ZPAmmoTypes::AMMO_LONGRIFLE;
	}
	return 0;
}

std::string CHudAmmoBank::GetAmmoName(int index)
{
	switch ( index )
	{
		case 0: return "Pistol";
		case 1: return "Magnum";
		case 2: return "Shotgun";
		case 3: return "Rifle";
		case 4: return "Long Rifle";
	}
	return "null";
}

float CHudAmmoBank::GetWeightPerBullet( int index, int amount )
{
	if ( amount <= 0 ) return 0.0f;
	AmmoData ammo = GetAmmoByAmmoID( index );
	return amount * ammo.WeightPerBullet;
}

float CHudAmmoBank::GetCarry()
{
	float flWeight = 0.0f;
	for ( int index = 0; index < ZPAmmoTypes::AMMO_MAX; index++ )
	{
		int iAmmoCount = gWR.CountAmmo( index );
		flWeight += GetWeightPerBullet( index, iAmmoCount );
	}

	for ( int slot = 0; slot < MAX_WEAPON_SLOTS; slot++ )
	{
		WEAPON *pWeapon = gWR.GetWeaponBySlot( slot );
		if ( pWeapon )
			flWeight += pWeapon->iWeight;
	}

	// If we got a backpack, add 10 punds.
	if (gHUD.m_iWeaponBits & (1 << WEAPON_BACKPACK))
		flWeight += 10.0f;

	return flWeight;
}

float CHudAmmoBank::GetMaxCarry()
{
	// Our min speed is 50, so reduce 50 from this.
	// It's very simplistic, since we check our max speed as the max carry.
	return ZP::MaxSpeeds[0] - 50;
}

void CHudAmmoBank::UpdateVisibility( bool state )
{
	for ( int index = 0; index < 5; index++ )
	{
		m_pAmmoName[index]->SetVisible( state );
		m_pAmmoCount[index]->SetVisible( state );
	}
	m_pWeightText->SetVisible( state );
	m_pWeightStatus->SetVisible( state );
	m_pBackground->SetVisible( state );
}
