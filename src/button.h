//
// C Interface: button
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_BUTTON_H
#define SUPERTUX_BUTTON_H

#include "texture.h"

enum {
  BN_CLICKED,
  BN_PRESSED,
  BN_HOVER
};

typedef struct button_type
  {
    texture_type icon;
    texture_type* bkgd;
    char *info;
    SDLKey shortcut;
    int x;
    int y;
    int w;
    int h;
    int show_info;
    int state;
    int tag;
  }
button_type;

void button_load(button_type* pbutton,char* icon_file, char* info, SDLKey shortcut, int x, int y);
button_type* button_create(char* icon_file, char* info, SDLKey shortcut, int x, int y);
button_type* button_change_icon(button_type* pbutton,char* icon_file);
void button_draw(button_type* pbutton);
void button_free(button_type* pbutton);
void button_event(button_type* pbutton, SDL_Event* event);
int button_get_state(button_type* pbutton);

typedef struct button_panel_type
  {
    int num_items;
    int hidden;
    int x,y;
    int w,h;
    button_type* item;
  }
button_panel_type;

void button_panel_init(button_panel_type* pbutton_panel, int x, int y, int w, int h);
void button_panel_free(button_panel_type* pbutton_panel);
void button_panel_draw(button_panel_type* pbutton_panel);
void button_panel_additem(button_panel_type* pbutton_panel, button_type* pbutton, int tag);
button_type* button_panel_event(button_panel_type* pbutton_panel, SDL_Event* event);

#endif /*SUPERTUX_BUTTON_H*/
