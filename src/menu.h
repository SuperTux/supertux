/*
  menu.h
  
  Super Tux - Menu
  
  by Tobias Glaesser
  tobi.web@gmx.de
  http://www.newbreedsoftware.com/supertux/
  
  December 20, 2003
*/

#ifndef SUPERTUX_MENU_H
#define SUPERTUX_MENU_H

#include <SDL.h>

/* (global) menu variables */
int menuaction;
int menuitem;
int menumenu;
int show_menu;
int menu_change;

/* Action done on the menu */
enum {
  MN_UP,
  MN_DOWN,
  MN_HIT
};

/* Menus */
enum {
  MENU_MAIN,
  MENU_OPTIONS
};

/* Initialize the menu variables */
void initmenu(void);

/* "Calculate" and draw the menu */
int drawmenu(void);

/* Check for a menu event */
void menu_event(SDLKey key);

#endif /*SUPERTUX_MENU_H*/

