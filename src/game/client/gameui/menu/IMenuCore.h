// ============== Copyright (c) 2025 Monochrome Games ============== \\

#ifndef GAMEUI_MENU_CORE_H
#define GAMEUI_MENU_CORE_H

enum MenuPagesTable_t
{
	PAGE_MAIN = 0,
	PAGE_OPTIONS,
	PAGE_EXTRAS,
	PAGE_MAX
};

struct MenuItemsMenuAdjustments
{
	int iRes;
	int PosStart;
	int MenuWide;
	int MenuItemTall;
	EHudScale Type;
};

#define MAX_PAGE_MENU_ITEMS 10

#endif