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
 * @summary A ""TextArrayObject"" that was given a name can be controlled by scripts.
            Supports all functions of ${SRG_REF_Text}, applying them to the current text item.${SRG_NEWPARAGRAPH}
            Intended for scripts with narration.
 * @instances A ""TextArrayObject"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
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

  /*
   * Text object api related
   * @see: text_object.hpp
   */
  /**
   * Current ""TextObject"": Sets the text string to be displayed.
   * @param string $text
   */
  void set_text(const std::string& text);
  /**
   * Current ""TextObject"": Sets the font of the text to be displayed.
   * @param string $fontname Valid values are normal, big and small.
   */
  void set_font(const std::string& fontname);
  /**
   * Current ""TextObject"": Fades in the specified text for the next ""fadetime"" seconds.
   * @param float $fadetime
   */
  void fade_in(float fadetime);
  /**
   * Current ""TextObject"": Fades out the specified text for the next ""fadetime"" seconds.
   * @param float $fadetime
   */
  void fade_out(float fadetime);
  /**
   * Current ""TextObject"": Grows in the specified text for the next ""fadetime"" seconds.
   * @param float $fadetime
   */
  void grow_in(float fadetime);
  /**
   * Current ""TextObject"": Grows out the specified text for the next ""fadetime"" seconds.
   * @param float $fadetime
   */
  void grow_out(float fadetime);
  /**
   * Current ""TextObject"": Shows or hides the text abruptly (drastic counterpart to ""fade_in()"" and ""fade_out()"").
   * @param bool $visible
   */
  void set_visible(bool visible);
  /**
   * Current ""TextObject"": If ""centered"" is ""true"", the text will be centered on the screen. Otherwise, it will be left-aligned.
   * @param bool $centered
   */
  void set_centered(bool centered);
  /**
   * Current ""TextObject"": Sets the offset of the text, relative to the anchor point.
   * @param float $x
   * @param float $y
   */
  void set_pos(float x, float y);
  /**
   * Current ""TextObject"": Returns the X offset of the text, relative to the anchor point.
   */
  float get_pos_x() const;
  /**
   * Current ""TextObject"": Returns the Y offset of the text, relative to the anchor point.
   */
  float get_pos_y() const;
  /**
   * Current ""TextObject"": Sets the anchor point of the text.
   * @param int $anchor One of the ""ANCHOR_*"" constants (see ${SRG_REF_AnchorPoints}).
   */
  void set_anchor_point(int anchor);
  /**
   * Current ""TextObject"": Returns the current anchor point of the text (one of the ""ANCHOR_*"" constants; see ${SRG_REF_AnchorPoints}).
   */
  int get_anchor_point() const;
  /**
   * Current ""TextObject"": Sets the anchor offset of the text.
   * @param float $x
   * @param float $y
   */
  void set_anchor_offset(float x, float y);
  /**
   * Current ""TextObject"": Sets the front fill color of the text.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   */
  void set_front_fill_color(float red, float green, float blue, float alpha);
  /**
   * Current ""TextObject"": Sets the back fill color of the text.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   */
  void set_back_fill_color(float red, float green, float blue, float alpha);
  /**
   * Current ""TextObject"": Sets the text color.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   */
  void set_text_color(float red, float green, float blue, float alpha);
  /**
   * Current ""TextObject"": Sets the frame's roundness.
   * @param float $roundness
   */
  void set_roundness(float roundness);
};

} // namespace scripting

#endif

/* EOF */
