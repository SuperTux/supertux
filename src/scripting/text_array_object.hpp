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

#ifndef HEADER_SUPERTUX_SCRIPTING_TEXT_ARRAY_OBJECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_TEXT_ARRAY_OBJECT_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class TextArrayObject;
#endif


namespace scripting {

/**
 * @summary A ""TextArrayObject"" that was given a name (or manually instantiated) can be controlled by scripts.
            Supports all functions of ${SRG_REF_Text}, applying them to the current text item.${SRG_NEWPARAGRAPH}
            Intended for scripts with narration.
 * @instances A ""TextArrayObject"" instance is already provided in sectors under ""sector.TextArray"".${SRG_NEWPARAGRAPH}
              A ""TextArrayObject"" can also be created in a script or from the console. Constructor:${SRG_NEWPARAGRAPH}
              """"""<textarrayobj> <- TextArrayObject()""""""
 */
class TextArrayObject final
#ifndef SCRIPTING_API
  : public GameObject<::TextArrayObject>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  TextArrayObject(const TextArrayObject&) = delete;
  TextArrayObject& operator=(const TextArrayObject&) = delete;
#endif

public:
  TextArrayObject();

  /*
   * The text array api.
   * @see: text_array_object.hpp
   */
  /**
   * Clears all text objects from the stack.
   */
  void clear();
  /**
   * Adds a text object with a specific text at the end of the stack.
   * @param string $text
   */
  void add_text(const std::string& text);
  /**
   * Adds a text object with a specific text and duration at the end of the stack.
   * @param string $text
   * @param float $duration
   */
  void add_text_duration(const std::string& text, float duration);
  /**
   * Sets the current text object by its index.
   * @param int $index
   */
  void set_text_index(int index);
  /**
   * If set, keeps the current text object visible.
   * @param bool $keep_visible
   */
  void set_keep_visible(bool keep_visible);
  /**
   * If set, allows for a fade-in and fade-out transition.
   * @param bool $fade_transition
   */
  void set_fade_transition(bool fade_transition);
  /**
   * Sets the fade-in and fade-out time.
   * @param float $fadetime
   */
  void set_fade_time(float fadetime);
  /**
   * If set, sets the text array as finished going through all text objects.
   * @param bool $done
   */
  void set_done(bool done);
  /**
   * If set, lets the text array automatically go through all text objects.
   * @param bool $is_auto
   */
  void set_auto(bool is_auto);
  /**
   * If available, goes to the next text object in the stack.
   */
  void next_text();
  /**
   * If available, goes to the previous text object in the stack.
   */
  void prev_text();

#ifndef DOXYGEN_SCRIPTING
  /*
   * Text object api related
   * @see: text_object.hpp
   */
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
  int get_anchor_point() const;
  void set_anchor_offset(float x, float y);
  float get_wrap_width() const;
  void set_wrap_width(float width);
  void set_front_fill_color(float red, float green, float blue, float alpha);
  void set_back_fill_color(float red, float green, float blue, float alpha);
  void set_text_color(float red, float green, float blue, float alpha);
  void set_roundness(float roundness);
#endif
};

} // namespace scripting

#endif

/* EOF */
