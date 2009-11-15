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

#include "object/gameobjs.hpp"

#include <stdio.h>

#include "math/random_generator.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/resources.hpp"

/** this controls the time over which a bouncy coin fades */
static const float FADE_TIME = .2f;
/** this is the total life time of a bouncy coin */
static const float LIFE_TIME = .5f;

BouncyCoin::BouncyCoin(const Vector& pos, bool emerge) :
  position(pos), 
  emerge_distance(0)
{
  timer.start(LIFE_TIME);
  sprite = sprite_manager->create("images/objects/coin/coin.sprite");

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

//---------------------------------------------------------------------------

BrokenBrick::BrokenBrick(std::auto_ptr<Sprite> sprite,
                         const Vector& pos, const Vector& nmovement) :
  sprite(sprite), 
  position(pos), 
  movement(nmovement)
{
  timer.start(.2f);
}

BrokenBrick::~BrokenBrick()
{
}

void
BrokenBrick::update(float elapsed_time)
{
  position += movement * elapsed_time;

  if (timer.check())
    remove_me();
}

void
BrokenBrick::draw(DrawingContext& context)
{
  sprite->draw_part(context,
                    Vector(systemRandom.rand(16), systemRandom.rand(16)), Vector(16, 16),
                    position, LAYER_OBJECTS + 1);
}

//---------------------------------------------------------------------------

FloatingText::FloatingText(const Vector& pos, const std::string& text_)
  : position(pos), text(text_)
{
  timer.start(.1f);
  position.x -= text.size() * 8;
}

FloatingText::FloatingText(const Vector& pos, int score)
  : position(pos)
{
  timer.start(.1f);

  // turn int into a string
  char str[10];
  snprintf(str, 10, "%d", score);
  text = str;

  position.x -= text.size() * 8;
}

void
FloatingText::update(float elapsed_time)
{
  position.y -= 1.4 * elapsed_time;

  if(timer.check())
    remove_me();
}

#define FADING_TIME .350

void
FloatingText::draw(DrawingContext& context)
{
  // make an alpha animation when disappearing
  int alpha;
  if(timer.get_timeleft() < FADING_TIME)
    alpha = int(timer.get_timeleft() * 255 / FADING_TIME);
  else
    alpha = 255;

  context.push_transform();
  context.set_alpha(alpha);

  context.draw_text(normal_font, text, position, ALIGN_LEFT, LAYER_OBJECTS+1, FloatingText::text_color);

  context.pop_transform();
}

SmokeCloud::SmokeCloud(const Vector& pos) :
  position(pos)
{
  timer.start(.3f);
  sprite = sprite_manager->create("images/objects/particles/stomp.sprite");
}

SmokeCloud::~SmokeCloud()
{
}

void
SmokeCloud::update(float elapsed_time)
{
  position.y -= 120 * elapsed_time;

  if(timer.check())
    remove_me();
}

void
SmokeCloud::draw(DrawingContext& context)
{
  sprite->draw(context, position, LAYER_OBJECTS+1);
}

/* EOF */
