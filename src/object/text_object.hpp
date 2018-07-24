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

#include "object/anchor_point.hpp"
#include "scripting/exposed_object.hpp"
#include "scripting/text.hpp"
#include "supertux/game_object.hpp"
#include "video/color.hpp"
#include "video/font_ptr.hpp"

/** A text object intended for scripts that want to tell a story */
class TextObject : public GameObject,
                   public ExposedObject<TextObject, scripting::Text>
{
  static Color default_color;
public:
  TextObject(const std::string& name = std::string());
  virtual ~TextObject();
  virtual bool is_saveable() const {
    return false;
  }

  void set_text(const std::string& text);
  void set_font(const std::string& name);
  void fade_in(float fadetime);
  void fade_out(float fadetime);
  void set_visible(bool visible);
  void set_centered(bool centered);
  bool is_visible();

  void set_anchor_point(AnchorPoint anchor_) {
    anchor = anchor_;
  }
  AnchorPoint get_anchor_point() const {
    return anchor;
  }

  void set_pos(const Vector& pos_) {
    pos = pos_;
  }
  const Vector& get_pos() const {
    return pos;
  }
  float get_pos_x() const {
    return pos.x;
  }
  float get_pos_y() const {
    return pos.y;
  }

  void set_anchor_point(int anchor_) {
    set_anchor_point((AnchorPoint) anchor_);
  }
  int get_anchor_point() {
    return (int)anchor;
  }

  void draw(DrawingContext& context);
  void update(float elapsed_time);

private:
  FontPtr font;
  std::string text;
  float fading;
  float fadetime;
  bool visible;
  bool centered;
  AnchorPoint anchor;
  Vector pos;

private:
  TextObject(const TextObject&);
  TextObject& operator=(const TextObject&);
};

#endif

/* EOF */
