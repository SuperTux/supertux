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
#include "vector.h"

void display_text_file(const std::string& file, const std::string& surface, float scroll_speed);
void display_text_file(const std::string& file, Surface* surface, float scroll_speed);

/* Kinds of texts. */
enum {
   TEXT_TEXT,
   TEXT_NUM
};

/* Text type */
class Font
{
public:
  Surface* chars;
  Surface* shadow_chars;
  int kind;
  int w;
  int h;
  int shadowsize;
public:
  Font(const std::string& file, int kind, int w, int h, int shadowsize = 2);
  ~Font();

  float get_height() const;
  float get_text_width(const std::string& text) const;

private:
  friend class DrawingContext;
  
  void draw(const std::string& text, const Vector& pos);
  void draw_chars(Surface* pchars, const std::string& text, 
      const Vector& position);
};

#endif /*SUPERTUX_TEXT_H*/

