#include <config.h>

#include "mriceblock.h"

static const float WALKSPEED = 80;
static const float KICKSPEED = 500;
static const int MAXSQUISHES = 10;

MrIceBlock::MrIceBlock(const lisp::Lisp& reader)
  : ice_state(ICESTATE_NORMAL), squishcount(0)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("mriceblock");
  set_direction = false;
}

MrIceBlock::MrIceBlock(float pos_x, float pos_y, Direction d)
  : ice_state(ICESTATE_NORMAL), squishcount(0)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("mriceblock");
  set_direction = true;
  initial_direction = d;
}

void
MrIceBlock::write(lisp::Writer& writer)
{
  writer.start_list("mriceblock");

  writer.write_float("x", get_pos().x);
  writer.write_float("y", get_pos().y);

  writer.end_list("mriceblock");
}

void
MrIceBlock::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

void
MrIceBlock::active_action(float elapsed_time)
{
  if(ice_state == ICESTATE_FLAT && flat_timer.check()) {
    printf("unflat.\n");
    ice_state = ICESTATE_NORMAL;
    physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
    sprite->set_action(dir == LEFT ? "left" : "right");
  }
  BadGuy::active_action(elapsed_time);
}

HitResponse
MrIceBlock::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // floor or roof
    physic.set_velocity_y(0);
    return CONTINUE;
  }
  // hit left or right
  switch(ice_state) {
    case ICESTATE_NORMAL:
      dir = dir == LEFT ? RIGHT : LEFT;
      sprite->set_action(dir == LEFT ? "left" : "right");
      physic.set_velocity_x(-physic.get_velocity_x());       
      break;
    case ICESTATE_KICKED:
      dir = dir == LEFT ? RIGHT : LEFT;
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      physic.set_velocity_x(-physic.get_velocity_x());
      SoundManager::get()->play_sound(IDToSound(SND_RICOCHET), get_pos(),
          Sector::current()->player->get_pos());
      break;
    case ICESTATE_FLAT:
      physic.set_velocity_x(0);
      break;
  }

  return CONTINUE;
}

bool
MrIceBlock::collision_squished(Player& player)
{
  switch(ice_state) {
    case ICESTATE_KICKED:
    case ICESTATE_NORMAL:
      squishcount++;
      if(squishcount >= MAXSQUISHES) {
        kill_fall();
        return true;
      }

      // flatten
      SoundManager::get()->play_sound(IDToSound(SND_STOMP), get_pos(),
          player.get_pos());
      physic.set_velocity_x(0);
      physic.set_velocity_y(0); 
      
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      flat_timer.start(4);
      ice_state = ICESTATE_FLAT;
      printf("flat.\n");
      break;
    case ICESTATE_FLAT:
      // kick
      SoundManager::get()->play_sound(IDToSound(SND_KICK), this,
          player.get_pos());

      if(player.get_pos().x < get_pos().x) {
        dir = RIGHT;
      } else {
        dir = LEFT;
      }
      physic.set_velocity_x(dir == LEFT ? -KICKSPEED : KICKSPEED);
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      ice_state = ICESTATE_KICKED;
      printf("kicked.\n");
      break;
  }

  player.bounce(*this);
  return true;
}

