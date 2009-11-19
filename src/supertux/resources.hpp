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

class Font;
class MouseCursor;

class Resources
{
public:
  static MouseCursor* mouse_cursor;

  static Font* fixed_font;
  static Font* normal_font;
  static Font* small_font;
  static Font* big_font;

public:
  static void load_shared();
  static void unload_shared();
};

#endif

/* EOF */
