#include <config.h>

#include "mrtree.h"

static const float WALKSPEED = 50;

MrTree::MrTree(const lisp::Lisp& reader)
  : mystate(STATE_BIG)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(84.8, 95.8);
  sprite = sprite_manager->create("mrtree");
}

void
MrTree::write(lisp::Writer& writer)
{
  writer.start_list("mrtree");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("mrtree");
}

void
MrTree::activate()
{
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  if(mystate == STATE_BIG)
    sprite->set_action(dir == LEFT ? "left" : "right");
  else
    sprite->set_action(dir == LEFT ? "small-left" : "small-right");
}

bool
MrTree::collision_squished(Player& player)
{
  if(mystate == STATE_BIG) {
    mystate = STATE_NORMAL;
    activate();

    SoundManager::get()->play_sound(IDToSound(SND_SQUISH), get_pos(),
            player.get_pos());      
    player.bounce(*this);
  } else {
    sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
    kill_squished(player);
  }
  
  return true;
}

HitResponse
MrTree::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) {
    physic.set_velocity_y(0);
  } else {
    dir = dir == LEFT ? RIGHT : LEFT;
    activate();
  }

  return CONTINUE;
}

HitResponse
MrTree::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.x) > .8) { // left or right hit
    dir = dir == LEFT ? RIGHT : LEFT;
    activate();
  }

  return CONTINUE;
}

IMPLEMENT_FACTORY(MrTree, "mrtree")
  
