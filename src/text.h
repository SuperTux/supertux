//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#ifndef SUPERTUX_TEXT_H
#define SUPERTUX_TEXT_H

#include <string>
#include "texture.h"

void display_text_file(const std::string& file, const std::string& surface, float scroll_speed);
void display_text_file(const std::string& file, Surface* surface, float scroll_speed);

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

