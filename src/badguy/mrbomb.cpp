#include <config.h>

#include "mrbomb.h"
#include "bomb.h"

static const float WALKSPEED = 80;

MrBomb::MrBomb(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("mrbomb");
  set_direction = false;
}

MrBomb::MrBomb(float pos_x, float pos_y, Direction d)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("mrbomb");
  set_direction = true;
  initial_direction = d;
}

void
MrBomb::write(lisp::Writer& writer)
{
  writer.start_list("mrbomb");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("mrbomb");
}

void
MrBomb::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
MrBomb::collision_squished(Player& player)
{
  remove_me();
  Sector::current()->add_object(new Bomb(get_pos(), dir));
  kill_squished(player);
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

HitResponse
MrBomb::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.x) > .8) { // left or right
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");    
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}
