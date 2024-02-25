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

#include "object/floating_image.hpp"

#include <simplesquirrel/class.hpp>
#include <simplesquirrel/vm.hpp>

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"

FloatingImage::FloatingImage(const std::string& spritefile) :
  sprite(SpriteManager::current()->create(spritefile)),
  layer(LAYER_FOREGROUND1 + 1),
  visible(false),
  anchor(ANCHOR_MIDDLE),
  pos(0.0f, 0.0f),
  fading(0),
  fadetime(0)
{
}

FloatingImage::~FloatingImage()
{
}

void
FloatingImage::update(float dt_sec)
{
  if (fading > 0) {
    fading -= dt_sec;
    if (fading <= 0) {
      fading = 0;
      visible = true;
    }
  } else if (fading < 0) {
    fading += dt_sec;
    if (fading >= 0) {
      fading = 0;
      visible = false;
    }
  }
}

void
FloatingImage::set_layer(int layer_)
{
  layer = layer_;
}

int
FloatingImage::get_layer() const
{
  return layer;
}

void
FloatingImage::set_pos(float x, float y)
{
  pos = Vector(x, y);
}

float
FloatingImage::get_x() const
{
  return pos.x;
}

float
FloatingImage::get_y() const
{
  return pos.y;
}

void
FloatingImage::set_anchor_point(int anchor_)
{
  anchor = static_cast<AnchorPoint>(anchor_);
}

int
FloatingImage::get_anchor_point() const
{
  return static_cast<int>(anchor);
}

bool
FloatingImage::get_visible() const
{
  return visible;
}

void
FloatingImage::set_visible(bool visible_)
{
  visible = visible_;
}

void
FloatingImage::set_action(const std::string& action)
{
  sprite->set_action(action);
}

std::string
FloatingImage::get_action() const
{
  return sprite->get_action();
}

void
FloatingImage::fade_in(float fadetime_)
{
  fadetime = fadetime_;
  fading = fadetime_;
}

void
FloatingImage::fade_out(float fadetime_)
{
  fadetime = fadetime_;
  fading = -fadetime_;
}

void
FloatingImage::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  if (fading > 0) {
    context.set_alpha((fadetime-fading) / fadetime);
  } else if (fading < 0) {
    context.set_alpha(-fading / fadetime);
  } else if (!visible) {
    context.pop_transform();
    return;
  }

  Vector spos = pos + get_anchor_pos(context.get_rect(),
                                     static_cast<float>(sprite->get_width()),
                                     static_cast<float>(sprite->get_height()),
                                     anchor);

  sprite->draw(context.color(), spos, layer);

  context.pop_transform();
}


void
FloatingImage::register_class(ssq::VM& vm)
{
  ssq::Class cls = vm.addClass("FloatingImage", [](const std::string& spritefile)
    {
      if (!Sector::current())
        throw std::runtime_error("Tried to create 'FloatingImage' without an active sector.");

      return &Sector::get().add<FloatingImage>(spritefile);
    },
    false /* Do not free pointer from Squirrel */,
    vm.findClass("GameObject"));

  cls.addFunc("set_layer", &FloatingImage::set_layer);
  cls.addFunc("get_layer", &FloatingImage::get_layer);
  cls.addFunc("set_pos", &FloatingImage::set_pos);
  cls.addFunc("get_x", &FloatingImage::get_x);
  cls.addFunc("get_y", &FloatingImage::get_y);
  cls.addFunc("get_pos_x", &FloatingImage::get_x);
  cls.addFunc("get_pos_y", &FloatingImage::get_y);
  cls.addFunc("set_anchor_point", &FloatingImage::set_anchor_point);
  cls.addFunc("get_anchor_point", &FloatingImage::get_anchor_point);
  cls.addFunc("set_visible", &FloatingImage::set_visible);
  cls.addFunc("get_visible", &FloatingImage::get_visible);
  cls.addFunc("set_action", &FloatingImage::set_action);
  cls.addFunc("get_action", &FloatingImage::get_action);
  cls.addFunc("fade_in", &FloatingImage::fade_in);
  cls.addFunc("fade_out", &FloatingImage::fade_out);

  cls.addVar("layer", &FloatingImage::layer);
  cls.addVar("visible", &FloatingImage::visible);
}

/* EOF */
