//========= Copyright (c) 2022 Zombie Panic! Team, All rights reserved. ============//

#if !defined( PANELLISTPANEL_H )
#define PANELLISTPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>

class KeyValues;


//-----------------------------------------------------------------------------
// Purpose: A list of variable height child panels
//-----------------------------------------------------------------------------
class C_ConfigList : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE( C_ConfigList, vgui2::Panel ); 

public:
	typedef struct dataitem_s
	{
		// Always store a panel pointer
		vgui2::Panel *panel;
	} DATAITEM;

	C_ConfigList( vgui2::Panel *parent, char const *panelName, bool inverseButtons = false );
	~C_ConfigList();

	// DATA & ROW HANDLING
	// The list now owns the panel
	virtual int	computeVPixelsNeeded( void );
	virtual int AddItem( vgui2::Panel *panel );
	virtual int	GetItemCount( void );
	virtual vgui2::Panel *GetItem(int itemIndex); // returns pointer to data the row holds
	virtual void RemoveItem(int itemIndex); // removes an item from the table (changing the indices of all following items)
	virtual void DeleteAllItems(); // clears and deletes all the memory used by the data items

	// career-mode UI wants to nudge sub-controls around
	void SetSliderYOffset(int pixels);

	// PAINTING
	virtual vgui2::Panel *GetCellRenderer( int row );

	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

	vgui2::Panel *GetEmbedded()
	{
		return _embedded;
	}

protected:

	DATAITEM	*GetDataItem( int itemIndex );

	virtual void PerformLayout();
	virtual void PaintBackground();
	virtual void OnMouseWheeled(int delta);

private:
	// list of the column headers
	vgui2::Dar<DATAITEM *>	_dataItems;
	vgui2::ScrollBar		*_vbar;
	vgui2::Panel			*_embedded;

	int					_tableStartX;
	int					_tableStartY;
	int					_sliderYOffset;
};

#endif // PANELLISTPANEL_H
