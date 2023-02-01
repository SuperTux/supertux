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

#include "math/anchor_point.hpp"
#include "scripting/text.hpp"
#include "squirrel/exposed_object.hpp"
#include "supertux/game_object.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font_ptr.hpp"

class DrawingContext;
class ReaderMapping;

/** A text object intended for scripts that want to tell a story */
class TextObject final : public GameObject,
                         public ExposedObject<TextObject, scripting::Text>
{
  static Color default_color;

public:
  TextObject(const ReaderMapping& reader);
  TextObject(const std::string& name = std::string());
  ~TextObject() override;

  static std::string class_name() { return "textobject"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Text"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual const std::string get_icon_path() const override { return "images/engine/editor/textarray.png"; }

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;
  virtual bool is_singleton() const override { return true; }
  virtual bool is_saveable() const override { return false; }


  void set_text(const std::string& text);
  void set_font(const std::string& name);
  void grow_in(float fadetime);
  void grow_out(float fadetime);
  void fade_in(float fadetime);
  void fade_out(float fadetime);
  void set_visible(bool visible);
  void set_centered(bool centered);
  void set_front_fill_color(Color frontfill);
  void set_back_fill_color(Color backfill);
  void set_text_color(Color textcolor);
  void set_roundness(float roundness);
  bool is_visible();

  void set_anchor_point(AnchorPoint anchor) { m_anchor = anchor; }
  AnchorPoint get_anchor_point() const { return m_anchor; }
  void set_anchor_offset(float x, float y) { m_anchor_offset = Vector(x, y); }

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
