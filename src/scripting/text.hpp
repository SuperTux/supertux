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

#ifndef SCRIPTING_API
#include <string>
#include "scripting/sqrat_object.hpp"

class TextObject;
#endif

namespace scripting {
#ifdef SCRIPTING_API
class Text
#else
class Text: SQRatObject<Text>
#endif
{
#ifndef SCRIPTING_API
private:
  ::TextObject* m_parent;

public:
  Text(::TextObject* parent);
  ~Text();
  static void register_exposed_methods(HSQUIRRELVM v, SQRatClassType squirrelClass)
  {
    squirrelClass.Func("set_text", &Text::set_text);
    squirrelClass.Func("set_font", &Text::set_font);
    squirrelClass.Func("fade_in", &Text::fade_in);
    squirrelClass.Func("fade_out", &Text::fade_out);
    squirrelClass.Func("set_visible", &Text::set_visible);
    squirrelClass.Func("set_centered", &Text::set_centered);
    squirrelClass.Func("set_pos", &Text::set_pos);
    squirrelClass.Func("get_pos_x", &Text::get_pos_x);
    squirrelClass.Func("get_pos_y", &Text::get_pos_y);
    squirrelClass.Func("set_anchor_point", &Text::set_anchor_point);
    squirrelClass.Func("get_anchor_point", &Text::get_anchor_point);
  }
  

private:
  Text(const Text&) = delete;
  Text& operator=(const Text&) = delete;
#endif

public:
  void set_text(const std::string& text);
  void set_font(const std::string& fontname);
  void fade_in(float fadetime);
  void fade_out(float fadetime);
  void set_visible(bool visible);
  void set_centered(bool centered);
  void set_pos(float x, float y);
  float get_pos_x() const;
  float get_pos_y() const;
  void set_anchor_point(int anchor);
  int  get_anchor_point() const;
};

}

#endif

/* EOF */
