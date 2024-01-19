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

#ifndef HEADER_SUPERTUX_SCRIPTING_TEXT_OBJECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_TEXT_OBJECT_HPP

#ifndef SCRIPTING_API
#include <memory>
#include "scripting/game_object.hpp"

class TextObject;
#endif

namespace scripting {

/**
 * @summary A ""TextObject"" that was given a name (or manually instantiated) can be controlled by scripts.
 * @instances A ""TextObject"" instance is already provided in sectors under ""sector.Text"".${SRG_NEWPARAGRAPH}
              A ""TextObject"" can also be created in a script or from the console. Constructor:${SRG_NEWPARAGRAPH}
              """"""<textobj> <- TextObject()""""""
 */
class TextObject final
#ifndef SCRIPTING_API
  : public GameObject<::TextObject>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  TextObject(const TextObject&) = delete;
  TextObject& operator=(const TextObject&) = delete;
#endif

public:
  TextObject();

  /**
   * Sets the text string to be displayed.
   * @param string $text
   */
  void set_text(const std::string& text);
  /**
   * Sets the font of the text to be displayed.
   * @param string $fontname Valid values are normal, big and small.
   */
  void set_font(const std::string& fontname);
  /**
   * Fades in the specified text for the next ""fadetime"" seconds.
   * @param float $fadetime
   */
  void fade_in(float fadetime);
  /**
   * Fades out the specified text for the next ""fadetime"" seconds.
   * @param float $fadetime
   */
  void fade_out(float fadetime);
  /**
   * Grows in the specified text for the next ""fadetime"" seconds.
   * @param float $fadetime
   */
  void grow_in(float fadetime);
  /**
   * Grows out the specified text for the next ""fadetime"" seconds.
   * @param float $fadetime
   */
  void grow_out(float fadetime);
  /**
   * Shows or hides the text abruptly (drastic counterpart to ""fade_in()"" and ""fade_out()"").
   * @param bool $visible
   */
  void set_visible(bool visible);
  /**
   * If ""centered"" is ""true"", the text will be centered on the screen. Otherwise, it will be left-aligned.
   * @param bool $centered
   */
  void set_centered(bool centered);
  /**
   * Sets the offset of the text, relative to the anchor point.
   * @param float $x
   * @param float $y
   */
  void set_pos(float x, float y);
  /**
   * Returns the X offset of the text, relative to the anchor point.
   */
  float get_pos_x() const;
  /**
   * Returns the Y offset of the text, relative to the anchor point.
   */
  float get_pos_y() const;
  /**
   * Sets the anchor point of the text.
   * @param int $anchor One of the ""ANCHOR_*"" constants (see ${SRG_REF_AnchorPoints}).
   */
  void set_anchor_point(int anchor);
  /**
   * Returns the current anchor point of the text (one of the ""ANCHOR_*"" constants; see ${SRG_REF_AnchorPoints}).
   */
  int get_anchor_point() const;
  /**
   * Sets the anchor offset of the text.
   * @param float $x
   * @param float $y
   */
  void set_anchor_offset(float x, float y);
  /**
   * Gets the text wrap width of the text.
   */
  float get_wrap_width() const;
  /**
   * Sets the text wrap width of the text.
   * @param float $width
   */
  void set_wrap_width(float width);
  /**
   * Sets the front fill color of the text.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   */
  void set_front_fill_color(float red, float green, float blue, float alpha);
  /**
   * Sets the back fill color of the text.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   */
  void set_back_fill_color(float red, float green, float blue, float alpha);
  /**
   * Sets the text color.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   */
  void set_text_color(float red, float green, float blue, float alpha);
  /**
   * Sets the frame's roundness.
   * @param float $roundness
   */
  void set_roundness(float roundness);
};

} // namespace scripting

#endif

/* EOF */
