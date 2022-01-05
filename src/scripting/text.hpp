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
#include <memory>
#include "scripting/game_object.hpp"

class TextObject;
#endif

namespace scripting {

class Text final
#ifndef SCRIPTING_API
  : public GameObject<::TextObject>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  Text(const Text&) = delete;
  Text& operator=(const Text&) = delete;
#endif

public:
  void set_text(const std::string& text);
  void set_font(const std::string& fontname);
  void fade_in(float fadetime);
  void fade_out(float fadetime);
  void grow_in(float fadetime);
  void grow_out(float fadetime);
  void set_visible(bool visible);
  void set_centered(bool centered);
  void set_pos(float x, float y);
  float get_pos_x() const;
  float get_pos_y() const;
  void set_anchor_point(int anchor);
  int  get_anchor_point() const;
  void set_front_fill_color(float red, float green, float blue, float alpha);
  void set_back_fill_color(float red, float green, float blue, float alpha);
  void set_text_color(float red, float green, float blue, float alpha);
  void set_roundness(float roundness);
};

} // namespace scripting

#endif

/* EOF */
