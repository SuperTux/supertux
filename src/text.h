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


/* Kinds of texts. */
enum {
   TEXT_TEXT,
   TEXT_NUM
};

enum TextHAlign {
   A_LEFT,
   A_HMIDDLE,
   A_RIGHT,
};

enum TextVAlign {
   A_TOP,
   A_VMIDDLE,
   A_BOTTOM,
};

/* Text type */
class Text
{
 public:
  Surface* chars;
  Surface* shadow_chars;
  int kind;
  int w;
  int h;
 public:
  Text(const std::string& file, int kind, int w, int h);
  ~Text();

  void draw(const char* text, int x, int y, int shadowsize = 1, int update = NO_UPDATE);
  void draw_chars(Surface* pchars, const char* text, int x, int y, int update = NO_UPDATE);
  void drawf(const char* text, int x, int y, TextHAlign halign, TextVAlign valign, int shadowsize, int update = NO_UPDATE);
  void draw_align(const char* text, int x, int y, TextHAlign halign, TextVAlign valign, int shadowsize = 1, int update = NO_UPDATE);
  void erasetext(const char * text, int x, int y, Surface* surf, int update, int shadowsize);
  void erasecenteredtext(const char * text, int y, Surface* surf, int update, int shadowsize);
};

#endif /*SUPERTUX_TEXT_H*/

