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

typedef struct button_type
  {
    texture_type icon;
    char *info;
    char *text;
    int x;
    int y;
    int w;
    int h;
    int state;
  } button_type;
  
void button_load(button_type* pbutton,char* icon_file, char* text, char* info, int x, int y);
void button_draw(button_type* pbutton);
void button_free(button_type* pbutton);
int button_pressed(button_type* pbutton, int x, int y);

enum {
  BN_PRESSED
};

#endif /*SUPERTUX_BUTTON_H*/
