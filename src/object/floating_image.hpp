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

#include "supertux/game_object.hpp"
#include "math/vector.hpp"
#include "object/anchor_point.hpp"
#include <memory>

class Sprite;

class FloatingImage : public GameObject
{
public:
  FloatingImage(const std::string& sprite);
  virtual ~FloatingImage();

  void set_layer(int layer) {
    this->layer = layer;
  }

  int get_layer() const {
    return layer;
  }

  void set_pos(const Vector& pos) {
    this->pos = pos;
  }
  const Vector& get_pos() const {
    return pos;
  }

  void set_anchor_point(AnchorPoint anchor) {
    this->anchor = anchor;
  }
  AnchorPoint get_anchor_point() const {
    return anchor;
  }

  void set_visible(bool visible) {
    this->visible = visible;
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
  std::auto_ptr<Sprite> sprite;
  int layer;
  bool visible;
  AnchorPoint anchor;
  Vector pos;
  float fading;
  float fadetime;
};

#endif
