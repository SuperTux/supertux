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

#include "audio/sound_manager.hpp"
#include "math/random_generator.hpp"
#include "object/camera.hpp"
#include "object/fireworks.hpp"
#include "object/particles.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"
#include "video/drawing_context.hpp"

Fireworks::Fireworks() :
  timer()
{
  timer.start(.2f);
  SoundManager::current()->preload("sounds/fireworks.wav");
}

void
Fireworks::update(float )
{
  if(timer.check()) {
    auto sector = Sector::current();
    Vector pos = sector->camera->get_translation();
    pos += Vector(graphicsRandom.randf(SCREEN_WIDTH),
                  graphicsRandom.randf(SCREEN_HEIGHT/2));

    float red = graphicsRandom.randf(1.0);
    float green = graphicsRandom.randf(1.0);
    //float red = 0.7;
    //float green = 0.9;
    (void) red;
    (void) green;
    sector->add_object(std::make_shared<Particles>(pos, 0, 360, 140, 140,
                                                   Vector(0, 0), 45, Color(red, green, 0), 3, 1.3f,
                                                   LAYER_FOREGROUND1+1));
    SoundManager::current()->play("sounds/fireworks.wav");
    timer.start(graphicsRandom.randf(1.0, 1.5));
  }
}

void
Fireworks::draw(DrawingContext& )
{
}

/* EOF */
