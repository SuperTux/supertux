//  SuperTux
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

#ifndef HEADER_SUPERTUX_SCRIPTING_TEXT_HPP
#define HEADER_SUPERTUX_SCRIPTING_TEXT_HPP

namespace Scripting
{

class Text
{
public:
#ifndef SCRIPTING_API
  virtual ~Text()
  { }
#endif

  virtual void set_text(const std::string& text) = 0;
  virtual void set_font(const std::string& fontname) = 0;
  virtual void fade_in(float fadetime) = 0;
  virtual void fade_out(float fadetime) = 0;
  virtual void set_visible(bool visible) = 0;
  virtual void set_centered(bool centered) = 0;
  virtual void set_pos(float x, float y) = 0;
  virtual float get_pos_x() = 0;
  virtual float get_pos_y() = 0;
  virtual void set_anchor_point(int anchor) = 0;
  virtual int  get_anchor_point() = 0;
};

}

#endif

/* EOF */
