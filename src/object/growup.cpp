#include <config.h>

#include <math.h>
#include "growup.h"
#include "defines.h"
#include "resources.h"
#include "camera.h"
#include "sector.h"
#include "player.h"
#include "app/globals.h"
#include "special/sprite_manager.h"

GrowUp::GrowUp(const Vector& pos)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  
  sprite = sprite_manager->create("egg");
  physic.enable_gravity(true);
  physic.set_velocity_x(100);
}

GrowUp::~GrowUp()
{
  delete sprite;
}

void
GrowUp::action(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
}

HitResponse
GrowUp::collision(GameObject& other, const CollisionHit& hit)
{
  if(other.get_flags() & FLAG_SOLID) {
    if(fabsf(hit.normal.y) > .5) { // roof
      physic.set_velocity_y(0);
    } else { // bumped left or right
      physic.set_velocity_x(-physic.get_velocity_x());
    }

    return CONTINUE;
  }
  
  Player* player = dynamic_cast<Player*>(&other);
  if(player != 0) {
    player->grow();
    SoundManager::get()->play_sound(IDToSound(SND_EXCELLENT));
    remove_me();
    
    return ABORT_MOVE;
  }

  return FORCE_MOVE;
}

void
GrowUp::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

