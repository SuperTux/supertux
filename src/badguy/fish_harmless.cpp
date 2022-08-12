//  SuperTux
//  Copyright (C) 2022 Daniel Ward <weluvgoatz@gmail.com>
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

#include "badguy/fish_harmless.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"

FishHarmless::FishHarmless(const ReaderMapping& reader) :
  FishSwimming(reader, "images/creatures/fish/ice/goldfish.sprite")
{
  SoundManager::current()->preload("sounds/trampoline.wav");
}

HitResponse
FishHarmless::collision_player(Player& player, const CollisionHit& hit)
{
  player.get_physic().set_velocity_x(hit.left ? -300.f : hit.right ? 300.f : player.get_physic().get_velocity_x());
  player.get_physic().set_velocity_y(hit.top ? -300.f : hit.bottom ? 300.f : player.get_physic().get_velocity_y());
  SoundManager::current()->play("sounds/trampoline.wav", get_pos());

  return FORCE_MOVE;
}

/* EOF */
