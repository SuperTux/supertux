//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include "fireworks.h"
#include "resources.h"
#include "sector.h"
#include "camera.h"
#include "gameobjs.h"
#include "main.h"
#include "video/drawing_context.h"
#include "audio/sound_manager.h"

Fireworks::Fireworks()
{
  timer.start(.2);
}

Fireworks::~Fireworks()
{
}

void
Fireworks::action(float )
{
    if(timer.check()) {
        Sector* sector = Sector::current();
        Vector pos = sector->camera->get_translation();
        pos += Vector(SCREEN_WIDTH * ((float) rand() / RAND_MAX),
                      SCREEN_HEIGHT/2 * ((float) rand() / RAND_MAX));

        int red = rand() % 255;
        int green = rand() % red;
        sector->add_object(new Particles(pos, 0, 360, Vector(140, 140),
                Vector(0, 0), 45, Color(red, green, 0), 3, 1.3,
                LAYER_FOREGROUND1+1));
        sound_manager->play_sound("fireworks");
        timer.start(((float) rand() / RAND_MAX) + .5);
    }
}

void
Fireworks::draw(DrawingContext& )
{
}
