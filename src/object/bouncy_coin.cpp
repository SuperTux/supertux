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

#include "object/bouncy_coin.hpp"

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

/** this controls the time over which a bouncy coin fades */
static const float FADE_TIME = .2f;
/** this is the total life time of a bouncy coin */
static const float LIFE_TIME = .5f;

BouncyCoin::BouncyCoin(const Vector& pos, bool emerge, const std::string& sprite_path) :
  sprite(),
  position(pos),
  timer(),
  emerge_distance(0)
{
  timer.start(LIFE_TIME);
  sprite = SpriteManager::current()->create(sprite_path);

  if(emerge) {
    emerge_distance = sprite->get_height();
  }
}

BouncyCoin::~BouncyCoin()
{
}

void
BouncyCoin::update(float elapsed_time)
{
  float dist = -200 * elapsed_time;
  position.y += dist;
  emerge_distance += dist;

  if(timer.check())
    remove_me();
}

void
BouncyCoin::draw(DrawingContext& context)
{
  float time_left = timer.get_timeleft();
  bool fading = time_left < FADE_TIME;
  if(fading) {
    float alpha = time_left/FADE_TIME;
    context.push_transform();
    context.set_alpha(alpha);
  }

  int layer;
  if(emerge_distance > 0) {
    layer = LAYER_OBJECTS - 5;
  } else {
    layer = LAYER_OBJECTS + 5;
  }
  sprite->draw(context, position, layer);

  if(fading) {
    context.pop_transform();
  }
}

/* EOF */
