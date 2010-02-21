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

#include "object/broken_brick.hpp"

#include "math/random_generator.hpp"
#include "sprite/sprite.hpp"

BrokenBrick::BrokenBrick(SpritePtr sprite,
                         const Vector& pos, const Vector& nmovement) :
  timer(),
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
                    Vector(graphicsRandom.rand(16), graphicsRandom.rand(16)), Vector(16, 16),
                    position, LAYER_OBJECTS + 1);
}

/* EOF */
