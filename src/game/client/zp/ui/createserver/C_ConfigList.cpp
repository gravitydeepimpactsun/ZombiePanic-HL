//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>

#include <KeyValues.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include "C_ConfigList.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui2;

class VScrollBarReversedButtons : public ScrollBar
{
public:
	VScrollBarReversedButtons( Panel *parent, const char *panelName, bool vertical );
	virtual void ApplySchemeSettings( IScheme *pScheme );
};

VScrollBarReversedButtons::VScrollBarReversedButtons( Panel *parent, const char *panelName, bool vertical ) : ScrollBar( parent, panelName, vertical )
{
}

void VScrollBarReversedButtons::ApplySchemeSettings( IScheme *pScheme )
{
	ScrollBar::ApplySchemeSettings( pScheme );

	Button *pButton;
	pButton = GetButton( 0 );
	pButton->SetArmedColor(		pButton->GetSchemeColor("DimBaseText", pScheme), pButton->GetBgColor());
	pButton->SetDepressedColor(	pButton->GetSchemeColor("DimBaseText", pScheme), pButton->GetBgColor());
	pButton->SetDefaultColor(	pButton->GetFgColor(),							 pButton->GetBgColor());
	
	pButton = GetButton( 1 );
	pButton->SetArmedColor(		pButton->GetSchemeColor("DimBaseText", pScheme), pButton->GetBgColor());
	pButton->SetDepressedColor(	pButton->GetSchemeColor("DimBaseText", pScheme), pButton->GetBgColor());
	pButton->SetDefaultColor(	pButton->GetFgColor(),							 pButton->GetBgColor());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : x - 
//			y - 
//			wide - 
//			tall - 
// Output : 
//-----------------------------------------------------------------------------
C_ConfigList::C_ConfigList( vgui2::Panel *parent, char const *panelName, bool inverseButtons ) : Panel( parent, panelName )
{
	SetBounds( 0, 0, 100, 100 );
	SetProportional( true );
	_sliderYOffset = 0;

	if (inverseButtons)
	{
		_vbar = new VScrollBarReversedButtons(this, "C_ConfigListVScroll", true );
	}
	else
	{
		_vbar = new ScrollBar(this, "C_ConfigListVScroll", true );
	}
	_vbar->SetBounds( 0, 0, 20, 20 );
	_vbar->SetVisible(false);
	_vbar->AddActionSignalTarget( this );

	_embedded = new Panel( this, "PanelListEmbedded" );
	_embedded->SetBounds( 0, 0, 20, 20 );
	_embedded->SetPaintBackgroundEnabled( false );
	_embedded->SetPaintBorderEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_ConfigList::~C_ConfigList()
{
	// free data from table
	DeleteAllItems();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	C_ConfigList::computeVPixelsNeeded( void )
{
	int pixels =0;
	DATAITEM *item;
	Panel *panel;
	for ( int i = 0; i < _dataItems.GetCount(); i++ )
	{
		item = _dataItems[ i ];
		if ( !item )
			continue;

		panel = item->panel;
		if ( !panel )
			continue;

		int w, h;
		panel->GetSize( w, h );

		pixels += h;
	}
	pixels+=5; // add a buffer after the last item

	return pixels;

}

//-----------------------------------------------------------------------------
// Purpose: Returns the panel to use to render a cell
// Input  : column - 
//			row - 
// Output : Panel
//-----------------------------------------------------------------------------
Panel *C_ConfigList::GetCellRenderer( int row )
{
	DATAITEM *item = _dataItems[ row ];
	if ( item )
	{
		Panel *panel = item->panel;
		return panel;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: adds an item to the view
//			data->GetName() is used to uniquely identify an item
//			data sub items are matched against column header name to be used in the table
// Input  : *item - 
//-----------------------------------------------------------------------------
int C_ConfigList::AddItem( Panel *panel )
{
	InvalidateLayout();

	DATAITEM *newitem = new DATAITEM;
	newitem->panel = panel;
	panel->SetParent( _embedded );
	return _dataItems.PutElement( newitem );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
int	C_ConfigList::GetItemCount( void )
{
	return _dataItems.GetCount();
}

//-----------------------------------------------------------------------------
// Purpose: returns pointer to data the row holds
// Input  : itemIndex - 
// Output : KeyValues
//-----------------------------------------------------------------------------
Panel *C_ConfigList::GetItem(int itemIndex)
{
	if ( itemIndex < 0 || itemIndex >= _dataItems.GetCount() )
		return NULL;

	return _dataItems[itemIndex]->panel;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : itemIndex - 
// Output : DATAITEM
//-----------------------------------------------------------------------------
C_ConfigList::DATAITEM *C_ConfigList::GetDataItem( int itemIndex )
{
	if ( itemIndex < 0 || itemIndex >= _dataItems.GetCount() )
		return NULL;

	return _dataItems[ itemIndex ];
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
//-----------------------------------------------------------------------------
void C_ConfigList::RemoveItem(int itemIndex)
{
	DATAITEM *item = _dataItems[ itemIndex ];
	delete item->panel;
	delete item;
	_dataItems.RemoveElementAt(itemIndex);

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: clears and deletes all the memory used by the data items
//-----------------------------------------------------------------------------
void C_ConfigList::DeleteAllItems()
{
	for (int i = 0; i < _dataItems.GetCount(); i++)
	{
		if ( _dataItems[i] )
		{
			delete _dataItems[i]->panel;
		}
		delete _dataItems[i];
	}
	_dataItems.RemoveAll();

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ConfigList::OnMouseWheeled(int delta)
{
	int val = _vbar->GetValue();
	val -= (delta * 3 * 5);
	_vbar->SetValue(val);
}

//-----------------------------------------------------------------------------
// Purpose: relayouts out the panel after any internal changes
//-----------------------------------------------------------------------------
void C_ConfigList::PerformLayout()
{
	int wide, tall;
	GetSize( wide, tall );

	int vpixels = computeVPixelsNeeded();

	//!! need to make it recalculate scroll positions
	_vbar->SetVisible(true);
	_vbar->SetEnabled(false);
	_vbar->SetRange( 0, vpixels - tall + 24);
	_vbar->SetRangeWindow( 24 /*vpixels / 10*/ );
	_vbar->SetButtonPressedScrollValue( 24 );
	_vbar->SetPos(wide - 20, _sliderYOffset);
	_vbar->SetSize(GetScaledValue(18), tall - 2 - _sliderYOffset);
	_vbar->InvalidateLayout();

	int top = _vbar->GetValue();

	_embedded->SetPos( 0, -top );
	_embedded->SetSize( wide-20, vpixels );

	// Now lay out the controls on the embedded panel
	int y = 0;
	int h = 0;
	for ( int i = 0; i < _dataItems.GetCount(); i++, y += h )
	{
		DATAITEM *item = _dataItems[ i ];
		if ( !item || !item->panel )
			continue;

		h = item->panel->GetTall();
		item->panel->SetBounds( 8, y, wide-36, h );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ConfigList::PaintBackground()
{
	Panel::PaintBackground();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *inResourceData - 
//-----------------------------------------------------------------------------
void C_ConfigList::ApplySchemeSettings(IScheme *pScheme)
{
	Panel::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
	SetBgColor(GetSchemeColor("Label.BgColor", GetBgColor(), pScheme));


//	_labelFgColor = GetSchemeColor("WindowFgColor");
//	_selectionFgColor = GetSchemeColor("ListSelectionFgColor", _labelFgColor);
}

void C_ConfigList::OnSliderMoved( int position )
{
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_ConfigList::SetSliderYOffset( int pixels )
{
	_sliderYOffset = pixels;
}
