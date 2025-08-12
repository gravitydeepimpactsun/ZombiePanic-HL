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

#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui2;

DECLARE_BUILD_FACTORY(CAchievementRequirements);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CAchievementRequirements::CAchievementRequirements(vgui2::Panel *parent, char const *panelName) : EditablePanel(parent, panelName)
{
	SetBounds(0, 0, 100, 100);

	m_vbar = new vgui2::ScrollBar(this, "PanelListPanelVScroll", true);
	m_vbar->SetVisible(false);
	m_vbar->AddActionSignalTarget(this);

	m_pPanelEmbedded = new EditablePanel(this, "PanelListEmbedded");
	m_pPanelEmbedded->SetBounds(0, 0, 43, 20);
	m_pPanelEmbedded->SetPaintBackgroundEnabled(false);
	m_pPanelEmbedded->SetPaintBorderEnabled(false);

	m_bAutoHideScrollbar = true;

	m_iFirstColumnWidth = 100; // default width
	m_iNumColumns = 1; // 1 column by default

	if (IsProportional())
	{
		m_iDefaultHeight = vgui2::scheme()->GetProportionalScaledValue(GetScheme());
		m_iPanelBuffer = vgui2::scheme()->GetProportionalScaledValue(GetScheme());
	}
	else
	{
		m_iDefaultHeight = DEFAULT_HEIGHT;
		m_iPanelBuffer = PANELBUFFER;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CAchievementRequirements::~CAchievementRequirements()
{
	// free data from table
	DeleteAllItems();
}

void CAchievementRequirements::SetVerticalBufferPixels(int buffer)
{
	m_iPanelBuffer = buffer;
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: counts the total vertical pixels
//-----------------------------------------------------------------------------
int	CAchievementRequirements::ComputeVPixelsNeeded()
{
	int iCurrentItem = 0;
	int iLargestH = 0;

	int pixels = 0;
	for (int i = 0; i < m_SortedItems.Count(); i++)
	{
		vgui2::Panel *panel = m_DataItems[m_SortedItems[i]].texture;
		if (!panel)
			continue;

		if (panel->IsLayoutInvalid())
		{
			panel->InvalidateLayout(true);
		}

		int iCurrentColumn = iCurrentItem % m_iNumColumns;

		int w, h;
		panel->GetSize(w, h);

		if (iLargestH < h)
			iLargestH = h;

		if (iCurrentColumn == 0)
			pixels += m_iPanelBuffer; // add in buffer. between rows.

		if (iCurrentColumn >= m_iNumColumns - 1)
		{
			pixels += iLargestH;
			iLargestH = 0;
		}

		iCurrentItem++;
	}

	// Add in remaining largest height
	pixels += iLargestH;

	pixels += m_iPanelBuffer; // add in buffer below last item

	return pixels;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the panel to use to render a cell
//-----------------------------------------------------------------------------
vgui2::Panel *CAchievementRequirements::GetCellRenderer(int row)
{
	if (!m_SortedItems.IsValidIndex(row))
		return NULL;

	vgui2::Panel *panel = m_DataItems[m_SortedItems[row]].texture;
	return panel;
}

//-----------------------------------------------------------------------------
// Purpose: adds an item to the view
//			data->GetName() is used to uniquely identify an item
//			data sub items are matched against column header name to be used in the table
//-----------------------------------------------------------------------------
int CAchievementRequirements::AddItem(bool bObtained, const char *szText)
{
	int itemID = m_DataItems.AddToTail();
	DATAITEM &newitem = m_DataItems[itemID];

	vgui2::Label *pText = new vgui2::Label( m_pPanelEmbedded, "Text", "" );
	pText->SetColorCodedText( szText );
	newitem.title = pText;

	vgui2::ImagePanel *pImage = new vgui2::ImagePanel( m_pPanelEmbedded, "Image" );
	pImage->SetImage( bObtained ? "resource/icon_checked_noborder" : "" );
	newitem.texture = pImage;

	m_SortedItems.AddToTail(itemID);

	InvalidateLayout();
	return itemID;
}

//-----------------------------------------------------------------------------
// Purpose: iteration accessor
//-----------------------------------------------------------------------------
int	CAchievementRequirements::GetItemCount() const
{
	return m_DataItems.Count();
}

int CAchievementRequirements::GetItemIDFromRow(int nRow) const
{
	if (nRow < 0 || nRow >= GetItemCount())
		return m_DataItems.InvalidIndex();
	return m_SortedItems[nRow];
}


//-----------------------------------------------------------------------------
// Iteration. Use these until they return InvalidItemID to iterate all the items.
//-----------------------------------------------------------------------------
int CAchievementRequirements::FirstItem() const
{
	return m_DataItems.Head();
}

int CAchievementRequirements::NextItem(int nItemID) const
{
	return m_DataItems.Next(nItemID);
}

int CAchievementRequirements::InvalidItemID() const
{
	return m_DataItems.InvalidIndex();
}


//-----------------------------------------------------------------------------
// Purpose: returns label panel for this itemID
//-----------------------------------------------------------------------------
vgui2::Panel *CAchievementRequirements::GetItemLabel(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return NULL;

	return m_DataItems[itemID].title;
}

//-----------------------------------------------------------------------------
// Purpose: returns label panel for this itemID
//-----------------------------------------------------------------------------
vgui2::Panel *CAchievementRequirements::GetItemPanel(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return NULL;

	return m_DataItems[itemID].texture;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementRequirements::RemoveItem(int itemID)
{
	if (!m_DataItems.IsValidIndex(itemID))
		return;

	DATAITEM &item = m_DataItems[itemID];

	if (item.texture)
		item.texture->MarkForDeletion();

	if (item.title)
		item.title->MarkForDeletion();

	m_DataItems.Remove(itemID);
	m_SortedItems.FindAndRemove(itemID);

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: clears and deletes all the memory used by the data items
//-----------------------------------------------------------------------------
void CAchievementRequirements::DeleteAllItems()
{
	FOR_EACH_LL(m_DataItems, i)
	{
		if (m_DataItems[i].texture)
		{
			m_DataItems[i].texture->MarkForDeletion();
			m_DataItems[i].texture = NULL;
		}
		if (m_DataItems[i].title)
		{
			m_DataItems[i].title->MarkForDeletion();
			m_DataItems[i].title = NULL;
		}
	}

	m_DataItems.RemoveAll();
	m_SortedItems.RemoveAll();

	// move the scrollbar to the top of the list
	m_vbar->SetValue(0);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: clears and deletes all the memory used by the data items
//-----------------------------------------------------------------------------
void CAchievementRequirements::RemoveAll()
{
	m_DataItems.RemoveAll();
	m_SortedItems.RemoveAll();

	// move the scrollbar to the top of the list
	m_vbar->SetValue(0);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementRequirements::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: relayouts out the panel after any internal changes
//-----------------------------------------------------------------------------
void CAchievementRequirements::PerformLayout()
{
	int wide, tall;
	GetSize(wide, tall);

	int vpixels = ComputeVPixelsNeeded();

	m_vbar->SetRange(0, vpixels);
	m_vbar->SetRangeWindow(tall);
	m_vbar->SetButtonPressedScrollValue(tall / 4); // standard height of labels/buttons etc.

	m_vbar->SetPos(wide - m_vbar->GetWide() - 2, 0);
	m_vbar->SetSize(m_vbar->GetWide(), tall - 2);

	int top = m_vbar->GetValue();

	if ( vpixels < 306 )
		vpixels = 306;

	m_pPanelEmbedded->SetPos(0, -top);
	m_pPanelEmbedded->SetSize(wide - m_vbar->GetWide(), vpixels);	// scrollbar will sit on top (zpos set explicitly)

	bool bScrollbarVisible = true;
	// If we're supposed to automatically hide the scrollbar when unnecessary, check it now
	if (m_bAutoHideScrollbar)
		bScrollbarVisible = (m_pPanelEmbedded->GetTall() > tall);

	m_vbar->SetVisible(bScrollbarVisible);

	// Now lay out the controls on the embedded panel
	int y = 0;
	int h = 10;

	int xpos = m_iFirstColumnWidth + m_iPanelBuffer;
	int iColumnWidth = (wide - xpos - m_vbar->GetWide() - 12) / m_iNumColumns;

	for (int i = 0; i < m_SortedItems.Count(); i++)
	{
		int iCurrentColumn = i % m_iNumColumns;

		// add in a little buffer between panels
		if (iCurrentColumn == 0)
			y += m_iPanelBuffer;

		DATAITEM &item = m_DataItems[m_SortedItems[i]];

		// Override the texture height
		item.texture->SetTall(70);

		if (h < item.texture->GetTall())
			h = item.texture->GetTall();

		item.texture->SetBounds(5, y, iColumnWidth, item.texture->GetTall());

		if (item.title)
			item.title->SetBounds(xpos + iCurrentColumn - 25, y - 2, 400, 20);

		if (iCurrentColumn >= m_iNumColumns - 1)
		{
			y += h;
			h = 0;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: scheme settings
//-----------------------------------------------------------------------------
void CAchievementRequirements::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
	SetBgColor(GetSchemeColor("ListPanel.BgColor", GetBgColor(), pScheme));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementRequirements::OnSliderMoved(int position)
{
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementRequirements::MoveScrollBarToTop()
{
	m_vbar->SetValue(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementRequirements::SetFirstColumnWidth(int width)
{
	m_iFirstColumnWidth = width;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
int CAchievementRequirements::GetFirstColumnWidth()
{
	return m_iFirstColumnWidth;
}

void CAchievementRequirements::SetNumColumns(int iNumColumns)
{
	m_iNumColumns = iNumColumns;
}

int CAchievementRequirements::GetNumColumns(void)
{
	return m_iNumColumns;
}

//-----------------------------------------------------------------------------
// Purpose: moves the scrollbar with the mousewheel
//-----------------------------------------------------------------------------
void CAchievementRequirements::OnMouseWheeled(int delta)
{
	int val = m_vbar->GetValue();
	val -= (delta * DEFAULT_HEIGHT);
	m_vbar->SetValue(val);
}

//-----------------------------------------------------------------------------
// Purpose: selection handler
//-----------------------------------------------------------------------------
void CAchievementRequirements::SetSelectedPanel(Panel *panel)
{
	if (panel != m_hSelectedItem)
	{
		// notify the panels of the selection change
		if (m_hSelectedItem)
		{
			PostMessage(m_hSelectedItem.Get(), new KeyValues("PanelSelected", "state", 0));
		}
		if (panel)
		{
			PostMessage(panel, new KeyValues("PanelSelected", "state", 1));
		}
		m_hSelectedItem = panel;
	}
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
vgui2::Panel *CAchievementRequirements::GetSelectedPanel()
{
	return m_hSelectedItem;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAchievementRequirements::ScrollToItem(int itemNumber)
{
	if (!m_vbar->IsVisible())
	{
		return;
	}

	DATAITEM& item = m_DataItems[m_SortedItems[itemNumber]];
	if (!item.texture)
		return;

	int x, y;
	item.texture->GetPos(x, y);
	int lx, ly;
	lx = x;
	ly = y;
	m_pPanelEmbedded->LocalToScreen(lx, ly);
	ScreenToLocal(lx, ly);

	int h = item.texture->GetTall();

	if (ly >= 0 && ly + h < GetTall())
		return;

	m_vbar->SetValue(y);
	InvalidateLayout();
}

CAchievementRequirementsHolder::CAchievementRequirementsHolder( vgui2::Panel *parent, char const *panelName )
    : Panel( parent, panelName )
{
	SetBounds(0, 0, 100, 100);

	m_pButton = new vgui2::ButtonImage(this, "PanelListPanelVScroll", "resource/icon_plus", this, "ToggleOption");
	m_pLabel = new vgui2::Label(this, "PanelListPanelVScroll", "#ZP_UI_Achievements_ShowDetails");
	// !!! Change this to be double array!
	// I need to backport my stat panel from Contagion,
	// because that was a pain in the ass.
	m_pList = new CAchievementRequirements(this, "ItemList");
	m_pList->SetVisible( false );
	m_pList->SetBounds( 0, 0, 43, 20 );
	m_bShouldExpand = true;
}


void CAchievementRequirementsHolder::AddItem(bool obtained, const char *text)
{
	// TODO: Add the value into CAchievementRequirements
}

void CAchievementRequirementsHolder::OnCommand(const char *szCommand)
{
	if (!Q_stricmp(szCommand, "ToggleOption"))
		UpdateSize( m_bShouldExpand );
	else
		BaseClass::OnCommand( szCommand );
}

void CAchievementRequirementsHolder::UpdateSize(bool bExpand)
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
}
