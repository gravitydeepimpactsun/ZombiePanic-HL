#include <algorithm>
#include <tier1/strtools.h>
#include <tier1/KeyValues.h>
#include <vgui/ISurface.h>
#include "hud.h"
#include "cl_util.h"
#include "hud/death_notice_panel.h"
#include "vgui/client_viewport.h"
#include "hud_renderer.h"
#include "zp/ui/workshop/WorkshopItemList.h"

extern ConVar hud_deathnotice_time;
ConVar hud_deathnotice_time_self("hud_deathnotice_time_self", "12", FCVAR_BHL_ARCHIVE, "How long should your death notices stay up for");
//ConVar hud_deathnotice_vgui("hud_deathnotice_vgui", "1", FCVAR_BHL_ARCHIVE, "Use VGUI deathnotice panel");

static constexpr int KILLFEED_COUNT = 6;
static constexpr int SKULL_SPRITE_HEIGHT = 16;
static constexpr int FEED_START_Y = 80;

DEFINE_HUD_ELEM(CHudDeathNoticePanel);

CHudDeathNoticePanel::CHudDeathNoticePanel()
    : vgui2::Panel(NULL, "HudDeathNoticePanel")
{
	AssertFatal(CHudRenderer::Get().IsAvailable());

	SetParent(g_pViewport);
	SetPaintBackgroundEnabled(true);

	m_EntryList[0].resize(KILLFEED_COUNT);
	m_EntryList[1].resize(KILLFEED_COUNT);
}

void CHudDeathNoticePanel::VidInit()
{
	if ( m_iDefaultIcon == -1 )
	{
		m_iDefaultIcon = vgui2::surface()->CreateNewTextureID( true );
		vgui2::surface()->DrawSetTextureFile( m_iDefaultIcon, "ui/icons/d_skull", true, false );
	}

	switch ( gHUD.m_iRes )
	{
		default:
		case 320:
		case 640: m_iIconHeight = 16; m_iStartYPos = FEED_START_Y; break;
		case 1280: m_iIconHeight = 32; m_iStartYPos = ( FEED_START_Y * 2 ); break;
		case 2560: m_iIconHeight = 48; m_iStartYPos = ( FEED_START_Y * 3 );break;
	}

	int cornerWide, cornerTall;
	GetCornerTextureSize(cornerWide, cornerTall);

	int minRowTall = std::max(cornerTall * 2, m_iIconHeight);
	m_iRowTall = std::max(m_iRowHeight, minRowTall);
}

void CHudDeathNoticePanel::InitHudData()
{
	m_iEntryCount = 0;
}

void CHudDeathNoticePanel::Think()
{
	bool shouldBeVisible = true;

	if (IsVisible() != shouldBeVisible)
	{
		SetVisible(shouldBeVisible);
		m_iEntryCount = 0;

		if (shouldBeVisible)
			m_iFlags &= ~HUD_ACTIVE; // Disable HUD Think since it will be called from VGUI
		else
			m_iFlags |= HUD_ACTIVE; // Enable Think from HUD to update visibility
	}

	if (shouldBeVisible)
	{
		// Remove expired entries
		auto &oldList = m_EntryList[m_nActiveList];
		auto &newList = m_EntryList[!m_nActiveList];

		int newIdx = 0;

		for (int i = 0; i < m_iEntryCount; i++)
		{
			Entry &e = oldList[i];

			if (e.flEndTime > gHUD.m_flTime)
				newList[newIdx++] = e;
		}

		m_iEntryCount = newIdx;
		m_nActiveList = !m_nActiveList;
	}
}

void CHudDeathNoticePanel::AddItem(int killerId, int victimId, int assistId, const char *killedwith, int death_flags)
{
	if (!GetThisPlayerInfo())
	{
		// Not yet connected
		return;
	}

	Entry e;
	CPlayerInfo *killer = GetPlayerInfoSafe(killerId);
	CPlayerInfo *victim = GetPlayerInfoSafe(victimId);
	CPlayerInfo *assist = nullptr;
	if ( killerId != assistId )
		assist = GetPlayerInfoSafe(assistId);
	int thisPlayerId = GetThisPlayerInfo()->GetIndex();

	// Check for suicide
	if (killerId == victimId || killerId == 0)
	{
		e.bIsSuicide = true;
	}

	// Check for team kill
	if (!strcmp(killedwith, "d_teammate"))
		e.bIsTeamKill = true;

	Color nameColor = m_ColorNameDefault;

	if (victimId == thisPlayerId)
	{
		e.type = EntryType::Death;
		nameColor = m_ColorNameDeath;
	}
	else if (killerId == thisPlayerId)
	{
		e.type = EntryType::Kill;
		nameColor = m_ColorNameKill;
	}

	// Fill killer info
	if (killer && !e.bIsSuicide)
	{
		bool removeColorCodes = killer->GetTeamNumber() != 0;
		e.iKillerLen = Q_UTF8ToWString(killer->GetDisplayName(removeColorCodes), e.wszKiller, sizeof(e.wszKiller), STRINGCONVERT_REPLACE);
		e.iKillerLen /= sizeof(wchar_t);
		e.iKillerLen--; // L'\0'
		e.iKillerWide = GetColoredTextWide(e.wszKiller, e.iKillerLen);
		e.killerColor = gHUD.GetClientColor(killerId, nameColor);
	}

	// Fill assist info
	if (assist && !e.bIsSuicide)
	{
		bool removeColorCodes = assist->GetTeamNumber() != 0;
		e.iAssistLen = Q_UTF8ToWString( vgui2::VarArgs( "+ %s", assist->GetDisplayName(removeColorCodes) ), e.wszAssist, sizeof(e.wszAssist), STRINGCONVERT_REPLACE);
		e.iAssistLen /= sizeof(wchar_t);
		e.iAssistLen--; // L'\0'
		e.iAssistWide = GetColoredTextWide(e.wszAssist, e.iAssistLen);
		e.assistColor = gHUD.GetClientColor(killerId, nameColor); // always use the killer color
	}

	// Fill victim info
	if (victim)
	{
		bool removeColorCodes = victim->GetTeamNumber() != 0;
		e.iVictimLen = Q_UTF8ToWString(victim->GetDisplayName(removeColorCodes), e.wszVictim, sizeof(e.wszVictim), STRINGCONVERT_REPLACE);
		e.iVictimLen /= sizeof(wchar_t);
		e.iVictimLen--; // L'\0'
		e.iVictimWide = GetColoredTextWide(e.wszVictim, e.iVictimLen);
		e.victimColor = gHUD.GetClientColor(victimId, nameColor);
	}

	// Expiration time
	float ttl = e.type != EntryType::Other ? hud_deathnotice_time_self.GetFloat() : hud_deathnotice_time.GetFloat();
	e.flEndTime = gHUD.m_flTime + ttl;

	e.iFlag = -1;
	e.iIcon = m_iDefaultIcon;
	e.iIconWidth[0] = 0;
	e.iIconWidth[1] = 0;

	// Check our kill icon list
	CHud::RegisteredIcon sHUDTexture = gHUD.GetRegisteredIcon( killedwith );
	if ( sHUDTexture.Icon != -1 )
	{
		e.iIcon = sHUDTexture.Icon;
		e.iIconWidth[0] = sHUDTexture.Wide;
	}
	else
		ConPrintf( Color( 255, 22, 22, 255 ), "Failed to find kill icon \"%s\"!\n", killedwith );

	// Icon flag
	sHUDTexture = GetFlagIcon( death_flags );
	if ( sHUDTexture.Icon != -1 )
	{
		e.iFlag = sHUDTexture.Icon;
		e.iIconWidth[1] = sHUDTexture.Wide;
	}

	// Insert into the list
	Assert(m_iEntryCount <= KILLFEED_COUNT);

	if (m_iEntryCount < KILLFEED_COUNT)
	{
		// Add to the end
		auto &list = m_EntryList[m_nActiveList];
		list[m_iEntryCount] = e;
		m_iEntryCount++;
	}
	else
	{
		auto &oldList = m_EntryList[m_nActiveList];
		auto &newList = m_EntryList[!m_nActiveList];

		// Find the item with the lowest end time
		int minItem = 0;
		float minEndTime = oldList[minItem].flEndTime;

		for (int i = 0; i < m_iEntryCount; i++)
		{
			Entry &t = oldList[i];

			if (t.flEndTime < minEndTime)
			{
				minEndTime = t.flEndTime;
				minItem = i;
			}
		}

		// Copy all items except the previously found item
		std::copy(oldList.begin(), oldList.begin() + minItem, newList.begin());
		std::copy(oldList.begin() + minItem + 1, oldList.end(), newList.begin() + minItem);

		// Add to the end
		newList[m_iEntryCount - 1] = std::move(e);

		m_nActiveList = !m_nActiveList;
	}
}

CHud::RegisteredIcon CHudDeathNoticePanel::GetFlagIcon( int flags )
{
	if ( ( flags & PLR_DEATH_FLAG_HEADSHOT ) != 0 ) return gHUD.GetRegisteredIcon( "d_headshot" );
	else if ( ( flags & PLR_DEATH_FLAG_GIBBED ) != 0 ) return gHUD.GetRegisteredIcon( "d_gibbed" );
	else if ( ( flags & PLR_DEATH_FLAG_FELL ) != 0 ) return gHUD.GetRegisteredIcon( "d_fell" );
	else if ( ( flags & PLR_DEATH_FLAG_BEYOND_GRAVE ) != 0 ) return gHUD.GetRegisteredIcon( "d_grave" );
	else if ( ( flags & PLR_DEATH_FLAG_TEAMKILLER ) != 0 ) return gHUD.GetRegisteredIcon( "d_teamkill" );
	return CHud::RegisteredIcon();
}

void CHudDeathNoticePanel::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);
}

void CHudDeathNoticePanel::PaintBackground()
{
	// HACK ALERT: See Paint()
	auto &entries = m_EntryList[m_nActiveList];
	int panelWide = GetWide();
	int y = m_iStartYPos;

	// Draw background
	for (int i = 0; i < m_iEntryCount; i++)
	{
		Entry &entry = entries[i];
		int wide = 2 * m_iHPadding + GetEntryContentWide(entry);
		int x = panelWide - wide;
		Color bgColor;

		if (entry.type == EntryType::Kill)
			bgColor = m_ColorBgKill;
		else if (entry.type == EntryType::Death)
			bgColor = m_ColorBgDeath;
		else
			bgColor = m_ColorBg;

		// Draw background
		DrawBox(x, y, wide, m_iRowTall, bgColor, 1.0f);

		y += m_iRowTall + m_iVMargin;
	}

	// Draw sprites
	y = m_iStartYPos;

	for (int i = 0; i < m_iEntryCount; i++)
	{
		Entry &entry = entries[i];
		int wide = 2 * m_iHPadding + GetEntryContentWide(entry);
		int x = panelWide - wide;

		// Calculate sprite pos
		int iconX = m_iHPadding + entry.iKillerWide + entry.iAssistWide;

		if (entry.iKillerWide != 0)
			iconX += m_iIconPadding; // padding on the left only if there is a text

		// Do the same for assist, if we have any
		if (entry.iAssistWide != 0)
			iconX += m_iIconPadding; // padding on the left only if there is a text

		int iconY = (m_iRowTall - m_iIconHeight) / 2;

		// Get sprite color
		Color color;

		if (entry.bIsTeamKill)
			color = m_ColorIconTK;
		else if (entry.type == EntryType::Kill)
			color = m_ColorIconKill;
		else if (entry.type == EntryType::Death)
			color = m_ColorIconDeath;
		else
			color = m_ColorIcon;

		if ( entry.iIcon != -1 )
		{
			vgui2::surface()->DrawSetTexture( entry.iIcon );
			vgui2::surface()->DrawSetColor( color );
			vgui2::surface()->DrawTexturedRect( x + iconX, y + iconY, x + iconX + entry.iIconWidth[0], y + iconY + m_iIconHeight );
		}

		if ( entry.iFlag != -1 )
		{
			iconX += entry.iIconWidth[0] + 5;
			vgui2::surface()->DrawSetTexture( entry.iFlag );
			vgui2::surface()->DrawSetColor( color );
			vgui2::surface()->DrawTexturedRect( x + iconX, y + iconY, x + iconX + entry.iIconWidth[1], y + iconY + m_iIconHeight );
		}

		y += m_iRowTall + m_iVMargin;
	}
}

void CHudDeathNoticePanel::Paint()
{
	// HACK ALERT!
	// Text rendering is done in Paint() because DrawFlushText
	// (called internally during rendering) disables alpha-blending at the end.
	// This causes transparent background to be drawn opaque.
	int panelWide = GetWide();
	int y = m_iStartYPos;
	int fontTall = vgui2::surface()->GetFontTall(m_TextFont);
	int textY = (m_iRowTall - fontTall) / 2;

	vgui2::surface()->DrawSetTextFont(m_TextFont);

	auto &entries = m_EntryList[m_nActiveList];

	for (int i = 0; i < m_iEntryCount; i++)
	{
		Entry &entry = entries[i];
		int wide = 2 * m_iHPadding + GetEntryContentWide(entry);
		int x = panelWide - wide;
		x += m_iHPadding;

		// Draw killer name
		if (entry.iKillerWide != 0)
		{
			DrawColoredText(x, y + textY, entry.wszKiller, entry.iKillerLen, entry.killerColor);
			x += entry.iKillerWide + m_iIconPadding;
		}

		// Draw assist name
		if (entry.iAssistWide != 0)
		{
			DrawColoredText(x, y + textY, entry.wszAssist, entry.iAssistLen, entry.assistColor);
			x += entry.iAssistWide + m_iIconPadding;
		}

		// Skip icon
		x += entry.iIconWidth[0] + m_iIconPadding;
		if ( entry.iIconWidth[1] > 0 )
			x += entry.iIconWidth[1] + 5;

		// Draw victim name
		DrawColoredText(x, y + textY, entry.wszVictim, entry.iVictimLen, entry.victimColor);
		x += entry.iVictimWide;

		y += m_iRowTall + m_iVMargin;
	}
}

int CHudDeathNoticePanel::GetEntryContentWide(const Entry &e)
{
	int w = e.iKillerWide + e.iAssistWide + m_iIconPadding + e.iIconWidth[0] + e.iVictimWide;

	if ( e.iKillerWide != 0 )
		w += m_iIconPadding;

	if ( e.iAssistWide != 0 )
		w += m_iIconPadding;

	if ( e.iIconWidth[1] > 0 )
		w += 5 + e.iIconWidth[1];

	return w;
}

int CHudDeathNoticePanel::GetColoredTextWide(const wchar_t *str, int len)
{
	int x = 0;

	for (int i = 0; i < len; i++)
	{
		if (i + 1 < len && IsColorCode(str + i))
		{
			// Skip color code
			i++;
			continue;
		}

		x += vgui2::surface()->GetCharacterWidth(m_TextFont, str[i]);
	}

	return x;
}

int CHudDeathNoticePanel::DrawColoredText(int x0, int y0, const wchar_t *str, int len, Color c)
{
	int x = 0;
	vgui2::surface()->DrawSetTextColor(c);

	for (int i = 0; i < len; i++)
	{
		if (i + 1 < len && IsColorCode(str + i))
		{
			// Set color
			i++;
			int idx = str[i] - L'0';
			if (idx == 0 || idx == 9)
				vgui2::surface()->DrawSetTextColor(c);
			else
				vgui2::surface()->DrawSetTextColor(gHUD.GetColorCodeColor(idx));
			continue;
		}

		vgui2::surface()->DrawSetTextPos(x0 + x, y0);
		vgui2::surface()->DrawUnicodeChar(str[i]);
		x += vgui2::surface()->GetCharacterWidth(m_TextFont, str[i]);
	}

	return x;
}
