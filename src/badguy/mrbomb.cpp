//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "audio/sound_manager.hpp"
#include "badguy/bomb.hpp"
#include "badguy/mrbomb.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"

MrBomb::MrBomb(const Reader& reader) :
  WalkingBadguy(reader, "images/creatures/mr_bomb/mr_bomb.sprite", "left", "right"),
  grabbed()
{
  walk_speed = 80;
  max_drop_height = 16;
  grabbed = false;

  //Prevent stutter when Tux jumps on Mr Bomb
  SoundManager::current()->preload("sounds/explosion.wav");

  //Check if we need another sprite
  if( !reader.get( "sprite", sprite_name ) ){
    return;
  }
  if( sprite_name == "" ){
    sprite_name = "images/creatures/mr_bomb/mr_bomb.sprite";
    return;
  }
  //Replace sprite
  sprite = SpriteManager::current()->create( sprite_name );
}

/* MrBomb created by a dispenser always gets default sprite atm.*/
MrBomb::MrBomb(const Vector& pos, Direction d) :
  WalkingBadguy(pos, d, "images/creatures/mr_bomb/mr_bomb.sprite", "left", "right"),
  grabbed()
{
  walk_speed = 80;
  max_drop_height = 16;
  grabbed = false;
  SoundManager::current()->preload("sounds/explosion.wav");
}

HitResponse
MrBomb::collision(GameObject& object, const CollisionHit& hit)
{
  if(grabbed)
    return FORCE_MOVE;
  return WalkingBadguy::collision(object, hit);
}

HitResponse
MrBomb::collision_player(Player& player, const CollisionHit& hit)
{
  if(grabbed)
    return FORCE_MOVE;
  return WalkingBadguy::collision_player(player, hit);
}

bool
MrBomb::collision_squished(GameObject& object)
{
  Player* player = dynamic_cast<Player*>(&object);
  if(player && player->is_invincible()) {
    player->bounce(*this);
    kill_fall();
    return true;
  }
  if(is_valid()) {
    remove_me();
    Sector::current()->add_object(std::make_shared<Bomb>(get_pos(), dir, sprite_name));
  }
  kill_squished(object);
  return true;
}

void
MrBomb::active_update(float elapsed_time)
{
  if(grabbed)
    return;
  WalkingBadguy::active_update(elapsed_time);
}

void
MrBomb::kill_fall()
{
  if(is_valid()) {
    remove_me();
    auto explosion = std::make_shared<Explosion>(get_bbox().get_middle());
    Sector::current()->add_object(explosion);
  }

  run_dead_script();
}

void
MrBomb::grab(MovingObject&, const Vector& pos, Direction dir_)
{
  assert(frozen);
  movement = pos - get_pos();
  this->dir = dir_;
  sprite->set_action(dir_ == LEFT ? "iced-left" : "iced-right");
  set_colgroup_active(COLGROUP_DISABLED);
  grabbed = true;
}

void
MrBomb::ungrab(MovingObject& , Direction dir_)
{
  this->dir = dir_;
  set_colgroup_active(COLGROUP_MOVING);
  grabbed = false;
}

bool
MrBomb::is_freezable() const
{
  return true;
}

bool
MrBomb::is_portable() const
{
  return frozen;
}


ObjectSettings
MrBomb::get_settings() {
  ObjectSettings result(_("Mr. Bomb"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}

/* EOF */
