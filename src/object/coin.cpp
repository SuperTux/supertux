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

//#include <config.h>

#include "audio/sound_manager.hpp"
//#include "audio/sound_source.hpp"
//#include "math/random_generator.hpp"
#include "object/coin.hpp"
#include "object/gameobjs.hpp"
#include "object/player.hpp"
//#include "sprite/sprite_manager.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
//#include "supertux/player_status.hpp"
//#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
//#include "supertux/statistics.hpp"
//#include "supertux/timer.hpp"
//#include "video/drawing_context.hpp"

Coin::Coin(const Vector& pos)
        : MovingSprite(pos, "images/objects/coin/coin.sprite", LAYER_TILES, COLGROUP_TOUCHABLE)
{
  sound_manager->preload("sounds/coin.wav");
}

Coin::Coin(const lisp::Lisp& reader)
        : MovingSprite(reader, "images/objects/coin/coin.sprite", LAYER_TILES, COLGROUP_TOUCHABLE)
{
  sound_manager->preload("sounds/coin.wav");
}

void
Coin::collect()
{
  // TODO: commented out musical code. Maybe fork this for a special "MusicalCoin" object?
  /*
  static Timer sound_timer;
  static int pitch_one = 128;
  static float last_pitch = 1;
  float pitch = 1;

  int tile = static_cast<int>(get_pos().y / 32);

  if (!sound_timer.started()) {
    pitch_one = tile;
    pitch = 1;
    last_pitch = 1;
  }
  else if (sound_timer.get_timegone() < 0.02) {
    pitch = last_pitch;
  }
  else
  {
    switch ((pitch_one - tile) % 7) {
      case -6:
        pitch = 1.0/2;
        break;
      case -5:
        pitch = 5.0/8;
        break;
      case -4:
        pitch = 4.0/6;
        break;
      case -3:
        pitch = 3.0/4;
        break;
      case -2:
        pitch = 5.0/6;
        break;
      case -1:
        pitch = 9.0/10;
        break;
      case 0:
        pitch = 1.0;
        break;
      case 1:
        pitch = 9.0/8;
        break;
      case 2:
        pitch = 5.0/4;
        break;
      case 3:
        pitch = 4.0/3;
        break;
      case 4:
        pitch = 3.0/2;
        break;
      case 5:
        pitch = 5.0/3;
        break;
      case 6:
        pitch = 9.0/5;
        break;
    }
    last_pitch = pitch;
  }
  sound_timer.start(1);

  SoundSource* soundSource = sound_manager->create_sound_source("sounds/coin.wav");
  soundSource->set_position(get_pos());
  soundSource->set_pitch(pitch);
  soundSource->play();
  sound_manager->manage_source(soundSource);
*/
  Sector::current()->player->get_status()->add_coins(1);
  Sector::current()->add_object(new BouncyCoin(get_pos()));
  Sector::current()->get_level()->stats.coins++;
  remove_me();
}

HitResponse
Coin::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if(player == 0)
    return ABORT_MOVE;

  collect();
  return ABORT_MOVE;
}

IMPLEMENT_FACTORY(Coin, "coin");
