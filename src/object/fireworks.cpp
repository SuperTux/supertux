//  $Id$
//
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

Fireworks::Fireworks()
{
  timer.start(.2);
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
        pos += Vector(SCREEN_WIDTH * ((float) rand() / RAND_MAX),
                      SCREEN_HEIGHT/2 * ((float) rand() / RAND_MAX));

        float red = static_cast<float>(rand() % 255) / 255.0;
        float green = static_cast<float>(rand() % ((int) red*255)) / 255.0;
        sector->add_object(new Particles(pos, 0, 360, Vector(140, 140),
                Vector(0, 0), 45, Color(red, green, 0), 3, 1.3,
                LAYER_FOREGROUND1+1));
        sound_manager->play("sounds/fireworks.wav");
        timer.start(((float) rand() / RAND_MAX) + .5);
    }
}

void
Fireworks::draw(DrawingContext& )
{
}
