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

#include "object/smoke_cloud.hpp"

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

SmokeCloud::SmokeCloud(const Vector& pos) :
  sprite(),
  timer(),
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
