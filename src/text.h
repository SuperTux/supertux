//
// C Interface: text
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_TEXT_H
#define SUPERTUX_TEXT_H

#include "texture.h"

/* Text type */
typedef struct text_type
  {
   texture_type chars[78];
  }  
text_type;

enum {
   A_LEFT,
   A_HMIDDLE,
   A_RIGHT,
   A_TOP,
   A_VMIDDLE,
   A_BOTTOM,
   A_NONE
};

void text_load(text_type* ptext, char* file);
void text_draw(text_type* ptext, char* text, int x, int y, int shadowsize, int update);
void text_drawf(text_type* ptext, char* text, int x, int y, int halign, int valign, int shadowsize, int update);
void text_free(text_type* ptext);

#endif /*SUPERTUX_TEXT_H*/

