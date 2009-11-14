//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "fireworks.hpp"
#include "resources.hpp"
#include "sector.hpp"
#include "camera.hpp"
#include "particles.hpp"
#include "main.hpp"
#include "video/drawing_context.hpp"
#include "audio/sound_manager.hpp"
#include "random_generator.hpp"

Fireworks::Fireworks()
{
  timer.start(.2f);
  sound_manager->preload("sounds/fireworks.wav");
}

Fireworks::~Fireworks()
{
}

void
Fireworks::update(float )
{
    if(timer.check()) {
        Sector* sector = Sector::current();
        Vector pos = sector->camera->get_translation();
        pos += Vector(systemRandom.randf(SCREEN_WIDTH),
                      systemRandom.randf(SCREEN_HEIGHT/2));

        float red = systemRandom.randf(1.0);
        float green = systemRandom.randf(1.0);
        //float red = 0.7;
        //float green = 0.9;
        (void) red;
        (void) green;
        sector->add_object(new Particles(pos, 0, 360, Vector(140, 140),
                Vector(0, 0), 45, Color(red, green, 0), 3, 1.3f,
                LAYER_FOREGROUND1+1));
        sound_manager->play("sounds/fireworks.wav");
        timer.start(systemRandom.randf(1.0, 1.5));
    }
}

void
Fireworks::draw(DrawingContext& )
{
}
