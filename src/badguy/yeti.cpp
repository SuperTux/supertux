#include <config.h>

#include <float.h>
#include "yeti.h"
#include "object/camera.h"
#include "yeti_stalactite.h"

static const float JUMP_VEL1 = 250;
static const float JUMP_VEL2 = 700;
static const float RUN_SPEED = 350;
static const float JUMP_TIME = 1.6;
static const float ANGRY_JUMP_WAIT = .5;

Yeti::Yeti(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(80, 120);
  sprite = sprite_manager->create("yeti");
  state = INIT;
  side = LEFT;
  sound_gna = SoundManager::get()->load_sound(
      get_resource_filename("sounds/yeti_gna.wav"));
}

Yeti::~Yeti()
{
  Mix_FreeChunk(sound_gna);
}

void
Yeti::active_action(float elapsed_time)
{
  switch(state) {
    case INIT:
      break;
    case GO_RIGHT:
      physic.set_velocity_x(RUN_SPEED);
      if(timer.check())
        physic.set_velocity_y(JUMP_VEL2);
      break;
    case GO_LEFT:
      physic.set_velocity_x(-RUN_SPEED);
      if(timer.check())
        physic.set_velocity_y(JUMP_VEL2);
      break;
    case ANGRY_JUMPING:
      if(timer.check()) {
        // jump
        SoundManager::get()->play_sound(sound_gna);
        physic.set_velocity_y(JUMP_VEL1);
      }
      break;
    default:
      break;
  }

  movement = physic.get_movement(elapsed_time);
}

void
Yeti::go_right()
{
  // jump and move right
  physic.set_velocity_y(JUMP_VEL1);
  physic.set_velocity_x(RUN_SPEED);
  state = GO_RIGHT;
  timer.start(JUMP_TIME);
}

void
Yeti::go_left()
{
  physic.set_velocity_y(JUMP_VEL1);
  physic.set_velocity_x(-RUN_SPEED);
  state = GO_LEFT;
  timer.start(JUMP_TIME);
}

void
Yeti::angry_jumping()
{
  jumpcount = 0;
  timer.start(ANGRY_JUMP_WAIT);
  state = ANGRY_JUMPING;
  physic.set_velocity_x(0);
}

bool
Yeti::collision_squished(Player& player)
{
  kill_squished(player);
  return true;
}

void
Yeti::write(lisp::Writer& )
{
}

void
Yeti::drop_stalactite()
{
  YetiStalactite* nearest = 0;
  float dist = FLT_MAX;

  Sector* sector = Sector::current();
  for(Sector::GameObjects::iterator i = sector->gameobjects.begin();
      i != sector->gameobjects.end(); ++i) {
    YetiStalactite* stalactite = dynamic_cast<YetiStalactite*> (*i);
    if(stalactite && stalactite->is_hanging()) {
      float sdist 
        = fabsf(stalactite->get_pos().x - sector->player->get_pos().x);
      if(sdist < dist) {
        nearest = stalactite;
        dist = sdist;
      }
    }
  }

  if(nearest)
    nearest->start_shaking();
}

HitResponse
Yeti::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
    if(state == INIT) {
      go_right();
    } else if(state == GO_LEFT && !timer.started()) {
      side = LEFT;
      angry_jumping();
    } else if(state == GO_RIGHT && !timer.started()) {
      side = RIGHT;
      angry_jumping();
    } else if(state == ANGRY_JUMPING) {
      if(!timer.started()) {
        // we just landed
        jumpcount++;
        // make a stalactite falling down and shake camera a bit
        Sector::current()->camera->shake(.1, 0, 10);
        drop_stalactite();
        
        // go to other side after 3 jumps
        if(jumpcount == 3) {
          if(side == LEFT)
            go_right();
          else
            go_left();
        } else {
          // jump again
          timer.start(ANGRY_JUMP_WAIT);
        }
      }
    }
  }
  
  return CONTINUE;
}

IMPLEMENT_FACTORY(Yeti, "yeti")
