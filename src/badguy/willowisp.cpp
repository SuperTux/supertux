//  SuperTux - "Will-O-Wisp" Badguy
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "badguy/willowisp.hpp"

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "object/lantern.hpp"
#include "object/path_walker.hpp"
#include "object/player.hpp"
#include "scripting/squirrel_util.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"

static const float FLYSPEED = 64; /**< speed in px per second */
static const float TRACK_RANGE = 384; /**< at what distance to start tracking the player */
static const float VANISH_RANGE = 512; /**< at what distance to stop tracking and vanish */
static const std::string SOUNDFILE = "sounds/willowisp.wav";

WillOWisp::WillOWisp(const Reader& reader) :
  BadGuy(reader, "images/creatures/willowisp/willowisp.sprite", LAYER_FLOATINGOBJECTS),
  mystate(STATE_IDLE),
  target_sector("main"),
  target_spawnpoint("main"),
  hit_script(),
  sound_source(),
  path(),
  walker(),
  flyspeed(),
  track_range(),
  vanish_range(),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  bool running = false;
  flyspeed     = FLYSPEED;
  track_range  = TRACK_RANGE;
  vanish_range = VANISH_RANGE;

  reader.get("sector", target_sector);
  reader.get("spawnpoint", target_spawnpoint);
  reader.get("name", name);
  reader.get("flyspeed", flyspeed);
  reader.get("track-range", track_range);
  reader.get("vanish-range", vanish_range);
  reader.get("hit-script", hit_script);
  reader.get("running", running);

  const lisp::Lisp* pathLisp = reader.get_lisp("path");
  if(pathLisp != NULL) {
    path.reset(new Path());
    path->read(*pathLisp);
    walker.reset(new PathWalker(path.get(), running));
    if(running)
      mystate = STATE_PATHMOVING_TRACK;
  }

  countMe = false;
  SoundManager::current()->preload(SOUNDFILE);
  SoundManager::current()->preload("sounds/warp.wav");

  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.0f, 0.2f, 0.0f));

  sprite->set_action("idle");
}

void
WillOWisp::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), layer);

  context.push_target();
  context.set_target(DrawingContext::LIGHTMAP);

  sprite->draw(context, get_pos(), layer);
  lightsprite->draw(context, get_bbox().get_middle(), 0);

  context.pop_target();
}

void
WillOWisp::active_update(float elapsed_time)
{
  Player* player = get_nearest_player();
  if (!player) return;
  Vector p1 = this->get_pos() + (this->get_bbox().p2 - this->get_bbox().p1) / 2;
  Vector p2 = player->get_pos() + (player->get_bbox().p2 - player->get_bbox().p1) / 2;
  Vector dist = (p2 - p1);

  switch(mystate) {
    case STATE_STOPPED:
      break;

    case STATE_IDLE:
      if (dist.norm() <= track_range) {
        mystate = STATE_TRACKING;
      }
      break;

    case STATE_TRACKING:
      if (dist.norm() > vanish_range) {
        vanish();
      } else if (dist.norm() >= 1) {
        Vector dir_ = dist.unit();
        movement = dir_ * elapsed_time * flyspeed;
      } else {
        /* We somehow landed right on top of the player without colliding.
         * Sit tight and avoid a division by zero. */
      }
      sound_source->set_position(get_pos());
      break;

    case STATE_WARPING:
      if(sprite->animation_done()) {
        remove_me();
      }

    case STATE_VANISHING: {
      Vector dir_ = dist.unit();
      movement = dir_ * elapsed_time * flyspeed;
      if(sprite->animation_done()) {
        remove_me();
      }
      break;
    }

    case STATE_PATHMOVING:
    case STATE_PATHMOVING_TRACK:
      if(walker.get() == NULL)
        return;
      movement = walker->advance(elapsed_time) - get_pos();
      if(mystate == STATE_PATHMOVING_TRACK && dist.norm() <= track_range) {
        mystate = STATE_TRACKING;
      }
      break;

    default:
      assert(false);
  }
}

void
WillOWisp::activate()
{
  sound_source = SoundManager::current()->create_sound_source(SOUNDFILE);
  sound_source->set_position(get_pos());
  sound_source->set_looping(true);
  sound_source->set_gain(2.0);
  sound_source->set_reference_distance(32);
  sound_source->play();
}

void
WillOWisp::deactivate()
{
  sound_source.reset(NULL);

  switch (mystate) {
    case STATE_STOPPED:
    case STATE_IDLE:
    case STATE_PATHMOVING:
    case STATE_PATHMOVING_TRACK:
      break;
    case STATE_TRACKING:
      mystate = STATE_IDLE;
      break;
    case STATE_WARPING:
    case STATE_VANISHING:
      remove_me();
      break;
  }
}

void
WillOWisp::vanish()
{
  mystate = STATE_VANISHING;
  sprite->set_action("vanishing", 1);
  set_colgroup_active(COLGROUP_DISABLED);
}

bool
WillOWisp::collides(GameObject& other, const CollisionHit& ) {
  Lantern* lantern = dynamic_cast<Lantern*>(&other);

  if (lantern && lantern->is_open())
    return true;

  if (dynamic_cast<Player*>(&other))
    return true;

  return false;
}

HitResponse
WillOWisp::collision_player(Player& player, const CollisionHit& ) {
  if(player.is_invincible())
    return ABORT_MOVE;

  if (mystate != STATE_TRACKING)
    return ABORT_MOVE;

  mystate = STATE_WARPING;
  sprite->set_action("warping", 1);

  if(hit_script != "") {
    std::istringstream stream(hit_script);
    Sector::current()->run_script(stream, "hit-script");
  } else {
    GameSession::current()->respawn(target_sector, target_spawnpoint);
  }
  SoundManager::current()->play("sounds/warp.wav");

  return CONTINUE;
}

void
WillOWisp::goto_node(int node_no)
{
  walker->goto_node(node_no);
  if(mystate != STATE_PATHMOVING && mystate != STATE_PATHMOVING_TRACK) {
    mystate = STATE_PATHMOVING;
  }
}

void
WillOWisp::start_moving()
{
  walker->start_moving();
}

void
WillOWisp::stop_moving()
{
  walker->stop_moving();
}

void
WillOWisp::set_state(const std::string& new_state)
{
  if(new_state == "stopped") {
    mystate = STATE_STOPPED;
  } else if(new_state == "idle") {
    mystate = STATE_IDLE;
  } else if(new_state == "move_path") {
    mystate = STATE_PATHMOVING;
    walker->start_moving();
  } else if(new_state == "move_path_track") {
    mystate = STATE_PATHMOVING_TRACK;
    walker->start_moving();
  } else if(new_state == "normal") {
    mystate = STATE_IDLE;
  } else if(new_state == "vanish") {
    vanish();
  } else {
    std::ostringstream msg;
    msg << "Can't set unknown willowisp state '" << new_state << "', should "
      "be stopped, move_path, move_path_track or normal";
    throw new std::runtime_error(msg.str());
  }
}

void
WillOWisp::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty())
    return;

  std::cout << "[DEBUG] Expose me '" << name << "'\n";
  scripting::WillOWisp* _this = static_cast<scripting::WillOWisp*> (this);
  expose_object(vm, table_idx, _this, name);
}

void
WillOWisp::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty())
    return;

  std::cout << "[DEBUG] UnExpose me '" << name << "'\n";
  scripting::unexpose_object(vm, table_idx, name);
}

/* EOF */
