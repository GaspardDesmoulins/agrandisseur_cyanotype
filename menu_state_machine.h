#ifndef MENU_STATE_MACHINE_H
#define MENU_STATE_MACHINE_H

#include "globals.h"

enum MenuState
{
	MENU_STANDBY,
	MENU_EDIT
};

struct MenuMachine
{
	MenuState state;
	uint8_t selectedItem;
	uint8_t currentPage;
	uint8_t pageCount;
	bool needsRefresh;
	bool editing;
	bool forceRefresh;
	unsigned long lastRefreshMs;
	char displayTarget[LCD_ROWS][LCD_COLS];
	char displayRendered[LCD_ROWS][LCD_COLS];
};

void menuInit(MenuMachine &machine);
void menuUpdate(MenuMachine &machine);
void menuOutput(MenuMachine &machine);

#endif
