#include <config.h>

#include "mrbomb.h"
#include "bomb.h"

static const float WALKSPEED = 80;

MrBomb::MrBomb(LispReader& reader)
{
  reader.read_float("x", start_position.x);
  reader.read_float("y", start_position.y);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("mrbomb");
}

MrBomb::MrBomb(float pos_x, float pos_y)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("mrbomb");
}

void
MrBomb::write(LispWriter& writer)
{
  writer.start_list("mrbomb");

  writer.write_float("x", get_pos().x);
  writer.write_float("y", get_pos().y);

  writer.end_list("mrbomb");
}

void
MrBomb::activate()
{
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
MrBomb::collision_squished(Player& player)
{
  remove_me();
  Sector::current()->add_object(new Bomb(get_pos(), dir));
  player.bounce(*this);
  return true;
}

HitResponse
MrBomb::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
  } else { // hit right or left
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

