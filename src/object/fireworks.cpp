#include <config.h>

#include "fireworks.h"
#include "resources.h"
#include "sector.h"
#include "camera.h"
#include "gameobjs.h"
#include "app/globals.h"
#include "video/drawing_context.h"
#include "audio/sound_manager.h"

using namespace SuperTux;

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
        pos += Vector(screen->w * ((float) rand() / RAND_MAX),
                      screen->h/2 * ((float) rand() / RAND_MAX));

        int red = rand() % 255;
        int green = rand() % red;
        sector->add_object(new Particles(pos, 0, 360, Vector(140, 140),
                Vector(0, 0), 45, Color(red, green, 0), 3, 1.3,
                LAYER_FOREGROUND1+1));
        SoundManager::get()->play_sound(IDToSound(SND_FIREWORKS));
        timer.start(((float) rand() / RAND_MAX) + .5);
    }
}

void
Fireworks::draw(DrawingContext& )
{
}
