#include <config.h>

#include <float.h>
#include "yeti.h"
#include "object/camera.h"
#include "yeti_stalactite.h"

static const float JUMP_VEL1 = 250;
static const float JUMP_VEL2 = 700;
static const float RUN_SPEED = 300;
static const float JUMP_TIME = 1.6;
static const float ANGRY_JUMP_WAIT = .5;
static const float STUN_TIME = 2;
static const int INITIAL_HITPOINTS = 3;
static const int INITIAL_BULLET_HP = 10;

Yeti::Yeti(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(80, 120);
  sprite = sprite_manager->create("yeti");
  state = INIT;
  side = LEFT;
  hitpoints = INITIAL_HITPOINTS;
  bullet_hitpoints = INITIAL_BULLET_HP;
  sound_gna = SoundManager::get()->load_sound(
      get_resource_filename("sounds/yeti_gna.wav"));
  sound_roar = SoundManager::get()->load_sound(
      get_resource_filename("sounds/yeti_roar.wav"));
  jump_time_left = 0.0f;
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
      if(jump_timer.check())
        physic.set_velocity_y(JUMP_VEL2);
      break;
    case GO_LEFT:
      physic.set_velocity_x(-RUN_SPEED);
      if(jump_timer.check())
        physic.set_velocity_y(JUMP_VEL2);
      break;
    case ANGRY_JUMPING:
      if(jump_timer.check()) {
        // jump
        SoundManager::get()->play_sound(sound_gna);
        physic.set_velocity_y(JUMP_VEL1);
      }
      break;

    case STUNNED:
        if (stun_timer.check()) {
            go_right();
        }
      
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
  jump_timer.start(JUMP_TIME);
}

void
Yeti::go_left()
{
  physic.set_velocity_y(JUMP_VEL1);
  physic.set_velocity_x(-RUN_SPEED);
  state = GO_LEFT;
  jump_timer.start(JUMP_TIME);
}

void
Yeti::angry_jumping()
{
  jumpcount = 0;
  jump_timer.start(ANGRY_JUMP_WAIT);
  state = ANGRY_JUMPING;
  physic.set_velocity_x(0);
}

void
Yeti::stun()
{
  physic.set_acceleration(0.0f, 0.0f);
  physic.set_velocity(0.0f, 0.0f);
  jump_time_left = jump_timer.get_timeleft();
  jump_timer.stop();
  stun_timer.start(STUN_TIME);
  state = STUNNED;
}

HitResponse
Yeti::collision_player(Player& player, const CollisionHit& hit)
{     
  if(player.is_invincible()) {
    kill_fall();
    return ABORT_MOVE;
  } 
  if(hit.normal.y > .9) {
    //TODO: fix inaccuracy (tux sometimes dies even if badguy was hit)
    //      give badguys some invincible time (prevent them from being hit multiple times)
    hitpoints--;
    bullet_hitpoints--;
    if(collision_squished(player))
      return ABORT_MOVE;
    else if (hitpoints <= 0) {
      bullet_hitpoints = 0;
      player.kill(Player::SHRINK);
      return FORCE_MOVE;
    }
  }

  if (state == STUNNED)
    return ABORT_MOVE;

  player.kill(Player::SHRINK);
  return FORCE_MOVE;
}

HitResponse
Yeti::collision_badguy(BadGuy& badguy, const CollisionHit&)
{
  YetiStalactite* yeti_stal = dynamic_cast<YetiStalactite*>(&badguy);

  if (state == STUNNED && yeti_stal && yeti_stal->is_harmful())
  {
    kill_fall();
  }

  return FORCE_MOVE;
} 
  

bool
Yeti::collision_squished(Player& player)
{
  // we don't use the player object, even though it was given to us
  (void)player;

  // stun yeti
  stun();

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
    } else if(state == GO_LEFT && !jump_timer.started()) {
      side = LEFT;
      angry_jumping();
    } else if(state == GO_RIGHT && !jump_timer.started()) {
      side = RIGHT;
      angry_jumping();
    } else if(state == ANGRY_JUMPING) {
      if(!jump_timer.started()) {
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
          jump_timer.start(ANGRY_JUMP_WAIT);
        }
      }
    }
  }
  
  return CONTINUE;
}

void
Yeti::kill_fall()
{
  SoundManager::get()->play_sound(sound_roar);
  bullet_hitpoints--;
  if (bullet_hitpoints <= 0) {
    SoundManager::get()->play_sound(IDToSound(SND_FALL), this,
       Sector::current()->player->get_pos());
    physic.set_velocity_y(0);
    physic.enable_gravity(true);
    set_state(STATE_FALLING);
  }
}

IMPLEMENT_FACTORY(Yeti, "yeti")
