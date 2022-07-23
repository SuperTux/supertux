//  SuperTux
//  Copyright (C) 2018 Nir <goproducti@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_TEXT_ARRAY_HPP
#define HEADER_SUPERTUX_SCRIPTING_TEXT_ARRAY_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class TextArrayObject;
#endif


namespace scripting {

/**
 * A text array intended for scripts with narration
 */
class TextArray final
#ifndef SCRIPTING_API
  : public GameObject<::TextArrayObject>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  TextArray(const TextArray&) = delete;
  TextArray& operator=(const TextArray&) = delete;
#endif

public:
  /*
   * The text array api.
   * @see: text_array_object.hpp
   */
  void clear();
  void add_text(const std::string& text);
  void add_text_duration(const std::string& text, float duration);
  void set_text_index(int index_);
  void set_keep_visible(bool keep_visible_);
  void set_fade_transition(bool fade_transition);
  void set_fade_time(float fadetime);
  void set_done(bool done);
  void set_auto(bool is_auto);
  void next_text();
  void prev_text();

  /*
   * Text object api related
   * @see: text_object.hpp
   */
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
  int get_anchor_point() const;
  void set_front_fill_color(float red, float green, float blue, float alpha);
  void set_back_fill_color(float red, float green, float blue, float alpha);
  void set_text_color(float red, float green, float blue, float alpha);
  void set_roundness(float roundness);
};

} // namespace scripting

#endif

/* EOF */
