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
   texture_type chars;
   texture_type shadow_chars;
   int kind;
   int w;
   int h;
  }  
text_type;

/* Kinds of texts. */
enum {
   TEXT_TEXT,
   TEXT_NUM
};

enum {
   A_LEFT,
   A_HMIDDLE,
   A_RIGHT,
   A_TOP,
   A_VMIDDLE,
   A_BOTTOM,
   A_NONE
};

void text_load(text_type* ptext, char* file, int kind, int w, int h);
void text_draw(text_type* ptext, char* text, int x, int y, int shadowsize, int update);
void text_draw_chars(text_type* ptext, texture_type* pchars, char* text, int x, int y, int update);
void text_drawf(text_type* ptext, char* text, int x, int y, int halign, int valign, int shadowsize, int update);
void text_free(text_type* ptext);
void erasetext(text_type* ptext, char * text, int x, int y, texture_type * surf, int update, int shadowsize);
void erasecenteredtext(text_type* ptext, char * text, int y, texture_type * surf, int update, int shadowsize);

#endif /*SUPERTUX_TEXT_H*/

