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

#ifndef HEADER_SUPERTUX_OBJECT_FLOATING_IMAGE_HPP
#define HEADER_SUPERTUX_OBJECT_FLOATING_IMAGE_HPP

#include "object/anchor_point.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"

class FloatingImage : public GameObject
{
public:
  FloatingImage(const std::string& sprite);
  virtual ~FloatingImage();
  virtual bool is_saveable() const {
    return false;
  }

  void set_layer(int layer_) {
    this->layer = layer_;
  }

  int get_layer() const {
    return layer;
  }

  void set_pos(const Vector& pos_) {
    this->pos = pos_;
  }
  const Vector& get_pos() const {
    return pos;
  }

  void set_anchor_point(AnchorPoint anchor_) {
    this->anchor = anchor_;
  }
  AnchorPoint get_anchor_point() const {
    return anchor;
  }

  void set_visible(bool visible_) {
    this->visible = visible_;
  }
  bool get_visible() const {
    return visible;
  }

  void set_action(const std::string& action);
  std::string get_action();

  void fade_in(float fadetime);
  void fade_out(float fadetime);

  void update(float elapsed_time);
  void draw(DrawingContext& context);

private:
  SpritePtr sprite;
  int layer;
  bool visible;
  AnchorPoint anchor;
  Vector pos;
  float fading;
  float fadetime;
};

#endif

/* EOF */
