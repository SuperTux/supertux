//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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

#include "badguy/kamikazesnowball.hpp"

#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "util/gettext.hpp"

/*
 * Kamikaze Snowball will fly in one direction until he hits something.
 * On impact he is destroyed, trying to kill what he hit or hit him.
 */
namespace{
  static const float KAMIKAZE_SPEED = 200;
  static const float LEAFSHOT_SPEED = 400;
  const std::string SPLAT_SOUND = "sounds/splat.wav";
}

KamikazeSnowball::KamikazeSnowball(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/snowball/kamikaze-snowball.sprite")
{
  SoundManager::current()->preload(SPLAT_SOUND);
  set_action (dir == LEFT ? "left" : "right", /* loops = */ -1);
}

KamikazeSnowball::KamikazeSnowball(const Vector& pos, Direction d)
  : BadGuy(pos, d, "images/creatures/snowball/kamikaze-snowball.sprite")
{
  SoundManager::current()->preload(SPLAT_SOUND);
  set_action (dir == LEFT ? "left" : "right", /* loops = */ -1);
}

void
KamikazeSnowball::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -KAMIKAZE_SPEED : KAMIKAZE_SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
KamikazeSnowball::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}

void
KamikazeSnowball::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  }
  if(hit.left || hit.right) {
    kill_collision();
  }
}

void
KamikazeSnowball::kill_collision()
{
  sprite->set_action(dir == LEFT ? "collision-left" : "collision-right");
  SoundManager::current()->play(SPLAT_SOUND, get_pos());
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  physic.enable_gravity(true);
  set_state(STATE_FALLING);

  run_dead_script();
}

HitResponse
KamikazeSnowball::collision_player(Player& player, const CollisionHit& hit)
{
  //Hack to tell if we should die
  HitResponse response = BadGuy::collision_player(player, hit);
  if(response == FORCE_MOVE) {
    kill_collision();
  }

  return ABORT_MOVE;
}

LeafShot::LeafShot(const ReaderMapping& reader) :
  KamikazeSnowball(reader)
{
  sprite = SpriteManager::current()->create("images/creatures/leafshot/leafshot.sprite");
}

ObjectSettings
KamikazeSnowball::get_settings() {
  ObjectSettings result(_("Kamikaze snowball"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}

void
LeafShot::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -LEAFSHOT_SPEED : LEAFSHOT_SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
LeafShot::is_freezable() const
{
  return true;
}

/* EOF */
