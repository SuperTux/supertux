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

#ifndef SUPERTUX_FONT_H
#define SUPERTUX_FONT_H

#include <string>

#include "../video/surface.h"
#include "../math/vector.h"

namespace SuperTux
  {

  /** Reads a text file (using LispReader, so it as to be in its formatting)
      and displays it in a StarTrek fashion */
  void display_text_file(const std::string& file, float scroll_speed);

  /* Text type */
  class Font
    {
    public:
      /* Kinds of texts. */
      enum FontType {
        TEXT, // images for all characters
        NUM   // only images for numbers
      };

      Font(const std::string& file, FontType type, int w, int h, int shadowsize=2);
      ~Font();

      /** returns the height of the font */
      float get_height() const;
      /** returns the width of a given text. (Note that I won't add a normal
       * get_width function here, as we might switch to variable width fonts in the
       * future.
       */
      float get_text_width(const std::string& text) const;

    private:
      friend class DrawingContext;

      void draw(const std::string& text, const Vector& pos,
                Uint32 drawing_effect = NONE_EFFECT);
      void draw_chars(Surface* pchars, const std::string& text,
                      const Vector& position, Uint32 drawing_effect);

      Surface* chars;
      Surface* shadow_chars;
      FontType type;
      int w;
      int h;
      int shadowsize;

      /// the number of the first character that is represented in the font
      int first_char;
      /// the number of the last character that is represented in the font
      int last_char;
    };

} //namespace SuperTux

#endif /*SUPERTUX_FONT_H*/

