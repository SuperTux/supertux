#include <config.h>

#include "star.h"
#include "resources.h"
#include "player.h"
#include "player_status.h"
#include "special/sprite_manager.h"
#include "video/drawing_context.h"

static const float INITIALJUMP = 400;
static const float SPEED = 150;
static const float JUMPSPEED = 300;

Star::Star(const Vector& pos)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("star");
  physic.set_velocity(SPEED, INITIALJUMP);
}

Star::~Star()
{
  delete sprite;
}

void
Star::action(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
}

void
Star::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

HitResponse
Star::collision(GameObject& other, const CollisionHit& hit)
{
  if(other.get_flags() & FLAG_SOLID) {
    if(hit.normal.y < -.5) { // ground
      physic.set_velocity_y(JUMPSPEED);
    } else if(hit.normal.y > .5) { // roof
      physic.set_velocity_y(0);
    } else { // bumped left or right
      physic.set_velocity_x(-physic.get_velocity_x());
    }

    return CONTINUE;
  }
  
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    player->make_invincible();
    remove_me();
    return ABORT_MOVE;
  }

  return FORCE_MOVE;
}

