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

#ifndef HEADER_SUPERTUX_OBJECT_TEXT_OBJECT_HPP
#define HEADER_SUPERTUX_OBJECT_TEXT_OBJECT_HPP

#include "supertux/game_object.hpp"

#include "math/anchor_point.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font_ptr.hpp"

class DrawingContext;
class ReaderMapping;

/** A text object intended for scripts that want to tell a story */
class TextObject final : public GameObject
{
  static Color default_color;

public:
  static void register_class(ssq::VM& vm);

public:
  TextObject(const std::string& name = "");
  ~TextObject() override;

  static std::string class_name() { return "textobject"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "TextObject"; }
  static std::string display_name() { return _("Text"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual const std::string get_icon_path() const override { return "images/engine/editor/textarray.png"; }

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;
  virtual bool is_saveable() const override { return false; }

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
   * @deprecated Use the ""visible"" property instead! (Does not apply for usage from a ""TextArray"".)
   * Shows or hides the text abruptly (drastic counterpart to ""fade_in()"" and ""fade_out()"").
   * @param bool $visible
   */
  void set_visible(bool visible);
  /**
   * @deprecated Use the ""centered"" property instead! (Does not apply for usage from a ""TextArray"".)
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
  float get_x() const;
  /**
   * Returns the Y offset of the text, relative to the anchor point.
   */
  float get_y() const;
#ifdef DOXYGEN_SCRIPTING
  /**
   * @deprecated Use ""get_x()"" instead!
   * Returns the X offset of the text, relative to the anchor point.
   */
  float get_pos_x() const;
  /**
   * @deprecated Use ""get_y()"" instead!
   * Returns the Y offset of the text, relative to the anchor point.
   */
  float get_pos_y() const;
#endif
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
   * @deprecated Use the ""wrap_width"" property instead! (Does not apply for usage from a ""TextArray"".)
   * Gets the text wrap width of the text.
   */
  float get_wrap_width() const;
  /**
   * @deprecated Use the ""wrap_width"" property instead! (Does not apply for usage from a ""TextArray"".)
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
   * @deprecated Use the ""roundness"" property instead! (Does not apply for usage from a ""TextArray"".)
   * Sets the frame's roundness.
   * @param float $roundness
   */
  void set_roundness(float roundness);

  void set_anchor_point(AnchorPoint anchor) { m_anchor = anchor; }
  void set_anchor_offset(const Vector& offset) { m_anchor_offset = offset; }

  void set_pos(const Vector& pos) { m_pos = pos; }
  const Vector& get_pos() const { return m_pos; }

private:
  void wrap_text();

private:
  FontPtr m_font;
  std::string m_text;
  std::string m_wrapped_text;
  float m_fade_progress;
  float m_fadetime;
  bool m_visible;
  bool m_centered;
  AnchorPoint m_anchor;
  Vector m_anchor_offset;
  Vector m_pos;
  float m_wrap_width;
  Color m_front_fill_color;
  Color m_back_fill_color;
  Color m_text_color;
  float m_roundness;
  bool m_growing_in;
  bool m_growing_out;
  bool m_fading_in;
  bool m_fading_out;
  bool m_grower;
  bool m_fader;

private:
  TextObject(const TextObject&) = delete;
  TextObject& operator=(const TextObject&) = delete;
};

#endif

/* EOF */
