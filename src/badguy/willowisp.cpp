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
#include "editor/editor.hpp"
#include "object/lantern.hpp"
#include "object/path_walker.hpp"
#include "object/player.hpp"
#include "scripting/squirrel_util.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

static const float FLYSPEED = 64; /**< speed in px per second */
static const float TRACK_RANGE = 384; /**< at what distance to start tracking the player */
static const float VANISH_RANGE = 512; /**< at what distance to stop tracking and vanish */
static const std::string SOUNDFILE = "sounds/willowisp.wav";

WillOWisp::WillOWisp(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/willowisp/willowisp.sprite", LAYER_FLOATINGOBJECTS,
         "images/objects/lightmap_light/lightmap_light-small.sprite"),
  ExposedObject<WillOWisp, scripting::WillOWisp>(this),
  PathObject(),
  mystate(STATE_IDLE),
  target_sector(),
  target_spawnpoint(),
  hit_script(),
  sound_source(),
  flyspeed(),
  track_range(),
  vanish_range()
{
  reader.get("sector", target_sector, "main");
  reader.get("spawnpoint", target_spawnpoint, "main");
  reader.get("name", name, "");
  reader.get("flyspeed", flyspeed, FLYSPEED);
  reader.get("track-range", track_range, TRACK_RANGE);
  reader.get("vanish-range", vanish_range, VANISH_RANGE);
  reader.get("hit-script", hit_script, "");

  bool running;
  if ( !reader.get("running", running)) running = false;

  ReaderMapping path_mapping;
  if(reader.get("path", path_mapping)) {
    path.reset(new Path());
    path->read(path_mapping);
    walker.reset(new PathWalker(path.get(), running));
    if(running)
      mystate = STATE_PATHMOVING_TRACK;
  }

  countMe = false;
  SoundManager::current()->preload(SOUNDFILE);
  SoundManager::current()->preload("sounds/warp.wav");

  lightsprite->set_color(Color(0.0f, 0.2f, 0.0f));
  glowing = true;

  sprite->set_action("idle");
}

void
WillOWisp::save(Writer& writer) {
  BadGuy::save(writer);
  writer.write("running", mystate == STATE_PATHMOVING_TRACK);
}

void
WillOWisp::active_update(float elapsed_time)
{
  if (Editor::is_active() && path.get() && path->is_valid()) {
      set_pos(walker->advance(elapsed_time));
      return;
  }

  auto player = get_nearest_player();
  if (!player) return;
  Vector p1 = bbox.get_middle();
  Vector p2 = player->get_bbox().get_middle();
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
  if(Editor::is_active())
    return;

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
WillOWisp::collides(GameObject& other, const CollisionHit& ) const {
  auto lantern = dynamic_cast<Lantern*>(&other);

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

  if(!hit_script.empty()) {
    Sector::current()->run_script(hit_script, "hit-script");
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
    log_warning << "Can't set unknown willowisp state '" << new_state << std::endl;
  }
}

ObjectSettings
WillOWisp::get_settings() {
  ObjectSettings result(_("Will 'o' wisp"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Sector"), &target_sector, "sector"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Spawnpoint"), &target_spawnpoint, "spawnpoint"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Hit script"), &hit_script, "hit-script"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Track range"), &track_range, "track-range"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Vanish range"), &vanish_range, "vanish-range"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Fly speed"), &flyspeed, "flyspeed"));
  return result;
}

void WillOWisp::stop_looping_sounds()
{
  if (sound_source) {
    sound_source->stop();
  }
}

void WillOWisp::play_looping_sounds()
{
  if (sound_source) {
    sound_source->play();
  }
}

void
WillOWisp::move_to(const Vector& pos)
{
  Vector shift = pos - bbox.p1;
  if (path) {
    path->move_by(shift);
  }
  set_pos(pos);
}

/* EOF */
