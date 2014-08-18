//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmail.com>
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

#include "control/input_manager.hpp"
#include "scripting/squirrel_util.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/player_status.hpp"
#include "supertux/savegame.hpp"
#include "supertux/tile.hpp"
#include "worldmap/level.hpp"
#include "worldmap/tux.hpp"

namespace worldmap {

static const float TUXSPEED = 200;
static const float map_message_TIME = 2.8f;

Tux::Tux(WorldMap* worldmap_) :
  back_direction(),
  worldmap(worldmap_),
  sprite(),
  controller(),
  input_direction(),
  direction(),
  tile_pos(),
  offset(),
  moving(),
  ghost_mode()
{
  sprite = SpriteManager::current()->create("images/worldmap/common/tux.sprite");

  offset = 0;
  moving = false;
  direction = D_NONE;
  input_direction = D_NONE;

  ghost_mode = false;
}

Tux::~Tux()
{
}

void
Tux::draw(DrawingContext& context)
{
  switch (worldmap->get_savegame().get_player_status()->bonus) {
    case GROWUP_BONUS:
      sprite->set_action(moving ? "large-walking" : "large-stop");
      break;
    case FIRE_BONUS:
      sprite->set_action(moving ? "fire-walking" : "fire-stop");
      break;
    case ICE_BONUS:
      sprite->set_action(moving ? "ice-walking" : "ice-stop");
      break;
    case NO_BONUS:
      sprite->set_action(moving ? "small-walking" : "small-stop");
      break;
    default:
      log_debug << "Bonus type not handled in worldmap." << std::endl;
      sprite->set_action("large-stop");
      break;
  }

  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

Vector
Tux::get_pos()
{
  float x = tile_pos.x * 32;
  float y = tile_pos.y * 32;

  switch(direction)
  {
    case D_WEST:
      x -= offset - 32;
      break;
    case D_EAST:
      x += offset - 32;
      break;
    case D_NORTH:
      y -= offset - 32;
      break;
    case D_SOUTH:
      y += offset - 32;
      break;
    case D_NONE:
      break;
  }

  return Vector(x, y);
}

void
Tux::stop()
{
  offset = 0;
  direction = D_NONE;
  input_direction = D_NONE;
  moving = false;
}

void
Tux::set_direction(Direction dir)
{
  input_direction = dir;
}

void
Tux::set_ghost_mode(bool enabled)
{
  ghost_mode = enabled;
}

bool
Tux::get_ghost_mode()
{
  return ghost_mode;
}

void
Tux::tryStartWalking()
{
  if (moving)
    return;
  if (input_direction == D_NONE)
    return;

  LevelTile* level = worldmap->at_level();

  // We got a new direction, so lets start walking when possible
  Vector next_tile;
  if ((!level || level->solved || level->perfect)
      && worldmap->path_ok(input_direction, tile_pos, &next_tile)) {
    tile_pos = next_tile;
    moving = true;
    direction = input_direction;
    back_direction = reverse_dir(direction);
  } else if (ghost_mode || (input_direction == back_direction)) {
    moving = true;
    direction = input_direction;
    tile_pos = worldmap->get_next_tile(tile_pos, direction);
    back_direction = reverse_dir(direction);
  }
}

bool
Tux::canWalk(int tile_data, Direction dir)
{
  return ghost_mode ||
    ((tile_data & Tile::WORLDMAP_NORTH && dir == D_NORTH) ||
     (tile_data & Tile::WORLDMAP_SOUTH && dir == D_SOUTH) ||
     (tile_data & Tile::WORLDMAP_EAST  && dir == D_EAST) ||
     (tile_data & Tile::WORLDMAP_WEST  && dir == D_WEST));
}

void
Tux::tryContinueWalking(float elapsed_time)
{
  if (!moving)
    return;

  // Let tux walk
  offset += TUXSPEED * elapsed_time;

  // Do nothing if we have not yet reached the next tile
  if (offset <= 32)
    return;

  offset -= 32;

  SpriteChange* sprite_change = worldmap->at_sprite_change(tile_pos);
  if(sprite_change != NULL) {
    sprite = sprite_change->sprite->clone();
    sprite_change->clear_stay_action();
  }

  // if this is a special_tile with passive_message, display it
  SpecialTile* special_tile = worldmap->at_special_tile();
  if(special_tile)
  {
    // direction and the apply_action_ are opposites, since they "see"
    // directions in a different way
    if((direction == D_NORTH && special_tile->apply_action_south) ||
       (direction == D_SOUTH && special_tile->apply_action_north) ||
       (direction == D_WEST && special_tile->apply_action_east) ||
       (direction == D_EAST && special_tile->apply_action_west))
    {
      if(special_tile->passive_message) {
        worldmap->passive_message = special_tile->map_message;
        worldmap->passive_message_timer.start(map_message_TIME);
      } else if(special_tile->script != "") {
        try {
          std::istringstream in(special_tile->script);
          worldmap->run_script(in, "specialtile");
        } catch(std::exception& e) {
          log_warning << "Couldn't execute special tile script: " << e.what()
                      << std::endl;
        }
      }
    }
  }

  // check if we are at a Teleporter
  Teleporter* teleporter = worldmap->at_teleporter(tile_pos);

  // stop if we reached a level, a WORLDMAP_STOP tile, a teleporter or a special tile without a passive_message
  if ((worldmap->at_level())
      || (worldmap->tile_data_at(tile_pos) & Tile::WORLDMAP_STOP)
      || (special_tile && !special_tile->passive_message
          && special_tile->script == "")
      || (teleporter) || ghost_mode) {
    if(special_tile && !special_tile->map_message.empty()
       && !special_tile->passive_message)
      worldmap->passive_message_timer.start(0);
    stop();
    return;
  }

  // if user wants to change direction, try changing, else guess the direction in which to walk next
  const int tile_data = worldmap->tile_data_at(tile_pos);
  if ((direction != input_direction) && canWalk(tile_data, input_direction)) {
    direction = input_direction;
    back_direction = reverse_dir(direction);
  } else {
    Direction dir = D_NONE;
    if (tile_data & Tile::WORLDMAP_NORTH && back_direction != D_NORTH)
      dir = D_NORTH;
    else if (tile_data & Tile::WORLDMAP_SOUTH && back_direction != D_SOUTH)
      dir = D_SOUTH;
    else if (tile_data & Tile::WORLDMAP_EAST && back_direction != D_EAST)
      dir = D_EAST;
    else if (tile_data & Tile::WORLDMAP_WEST && back_direction != D_WEST)
      dir = D_WEST;

    if (dir == D_NONE) {
      // Should never be reached if tiledata is good
      log_warning << "Could not determine where to walk next" << std::endl;
      stop();
      return;
    }

    direction = dir;
    input_direction = direction;
    back_direction = reverse_dir(direction);
  }

  // Walk automatically to the next tile
  if(direction == D_NONE)
    return;

  Vector next_tile;
  if (!ghost_mode && !worldmap->path_ok(direction, tile_pos, &next_tile)) {
    log_debug << "Tilemap data is buggy" << std::endl;
    stop();
    return;
  }

  SpriteChange* next_sprite = worldmap->at_sprite_change(next_tile);
  if(next_sprite != NULL && next_sprite->change_on_touch) {
    sprite = next_sprite->sprite->clone();
    next_sprite->clear_stay_action();
  }
  SpriteChange* last_sprite = worldmap->at_sprite_change(tile_pos);
  if(last_sprite != NULL && next_sprite != NULL) {
    log_debug << "Old: " << tile_pos << " New: " << next_tile << std::endl;
    last_sprite->set_stay_action();
  }

  tile_pos = next_tile;
}

void
Tux::updateInputDirection()
{
  Controller* controller_ = InputManager::current()->get_controller();
  if(controller_->hold(Controller::UP))
    input_direction = D_NORTH;
  else if(controller_->hold(Controller::DOWN))
    input_direction = D_SOUTH;
  else if(controller_->hold(Controller::LEFT))
    input_direction = D_WEST;
  else if(controller_->hold(Controller::RIGHT))
    input_direction = D_EAST;
}

void
Tux::update(float elapsed_time)
{
  updateInputDirection();
  if (moving)
    tryContinueWalking(elapsed_time);
  else
    tryStartWalking();
}

void
Tux::setup()
{
  // check if we already touch a SpriteChange object
  SpriteChange* sprite_change = worldmap->at_sprite_change(tile_pos);
  if(sprite_change != NULL) {
    sprite = sprite_change->sprite->clone();
    sprite_change->clear_stay_action();
  }
}

} // namespace WorldmapNS

/* EOF */
