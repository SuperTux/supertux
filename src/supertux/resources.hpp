//  SuperTux
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_SUPERTUX_RESOURCES_HPP
#define HEADER_SUPERTUX_SUPERTUX_RESOURCES_HPP

#include <memory>

#include "video/font_ptr.hpp"
#include "video/surface_ptr.hpp"

class MouseCursor;

class Resources
{
public:
  static std::unique_ptr<MouseCursor> mouse_cursor;

  static FontPtr fixed_font;
  static FontPtr normal_font;
  static FontPtr small_font;
  static FontPtr big_font;

  // Fallback fonts
  static FontPtr fixed_font_fallback;
  static FontPtr normal_font_fallback;
  static FontPtr small_font_fallback;
  static FontPtr big_font_fallback;

  static SurfacePtr checkbox;
  static SurfacePtr checkbox_checked;
  static SurfacePtr back;
  static SurfacePtr arrow_left;
  static SurfacePtr arrow_right;
  static SurfacePtr no_tile;

  static void reload_font_config();

  static FontPtr& get_fallback_font(const Font* font)
  {
    if(font == fixed_font.get())
      return fixed_font_fallback;
    if(font == normal_font.get())
      return normal_font_fallback;
    if(font == small_font.get())
      return small_font_fallback;
    if(font == big_font.get())
      return big_font_fallback;
    
    return normal_font_fallback;
  }

private:
  static void use_fallback_fonts();

public:
  Resources();
  ~Resources();
};

#endif

/* EOF */
