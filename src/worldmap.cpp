//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <unistd.h>

#include "app/globals.h"
#include "app/gettext.h"
#include "app/setup.h"
#include "video/surface.h"
#include "video/screen.h"
#include "video/drawing_context.h"
#include "special/frame_rate.h"
#include "audio/sound_manager.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "lisp/list_iterator.h"
#include "lisp/writer.h"
#include "gameloop.h"
#include "sector.h"
#include "worldmap.h"
#include "resources.h"
#include "misc.h"
#include "scene.h"

#define map_message_TIME 2.8

Menu* worldmap_menu  = 0;

namespace WorldMapNS {

Direction reverse_dir(Direction direction)
{
  switch(direction)
    {
    case D_WEST:
      return D_EAST;
    case D_EAST:
      return D_WEST;
    case D_NORTH:
      return D_SOUTH;
    case D_SOUTH:
      return D_NORTH;
    case D_NONE:
      return D_NONE;
    }
  return D_NONE;
}

std::string
direction_to_string(Direction direction)
{
  switch(direction)
    {
    case D_WEST:
      return "west";
    case D_EAST:
      return "east";
    case D_NORTH:
      return "north";
    case D_SOUTH:
      return "south";
    default:
      return "none";
    }
}

Direction
string_to_direction(const std::string& directory)
{
  if (directory == "west")
    return D_WEST;
  else if (directory == "east")
    return D_EAST;
  else if (directory == "north")
    return D_NORTH;
  else if (directory == "south")
    return D_SOUTH;
  else
    return D_NONE;
}

//---------------------------------------------------------------------------

Tux::Tux(WorldMap* worldmap_)
  : worldmap(worldmap_)
{
  largetux_sprite = new Surface(datadir +  "/images/worldmap/tux.png", true);
  firetux_sprite = new Surface(datadir +  "/images/worldmap/firetux.png", true);
  smalltux_sprite = new Surface(datadir +  "/images/worldmap/smalltux.png", true);

  offset = 0;
  moving = false;
  tile_pos.x = worldmap->get_start_x();
  tile_pos.y = worldmap->get_start_y();
  direction = D_NONE;
  input_direction = D_NONE;
}

Tux::~Tux()
{
  delete smalltux_sprite;
  delete firetux_sprite;
  delete largetux_sprite;
}

void
Tux::draw(DrawingContext& context, const Vector& offset)
{
  Vector pos = get_pos();
  switch (player_status.bonus)
    {
    case PlayerStatus::GROWUP_BONUS:
      context.draw_surface(largetux_sprite,
          Vector(pos.x + offset.x, pos.y + offset.y - 10), LAYER_OBJECTS);
      break;
    case PlayerStatus::FLOWER_BONUS:
      context.draw_surface(firetux_sprite,
          Vector(pos.x + offset.x, pos.y + offset.y - 10), LAYER_OBJECTS);
      break;
    case PlayerStatus::NO_BONUS:
      context.draw_surface(smalltux_sprite,
          Vector(pos.x + offset.x, pos.y + offset.y - 10), LAYER_OBJECTS);
      break;
    }
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
  
  return Vector((int)x, (int)y); 
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
Tux::action(float delta)
{
  if (!moving)
    {
      if (input_direction != D_NONE)
        { 
          WorldMap::Level* level = worldmap->at_level();

          // We got a new direction, so lets start walking when possible
          Vector next_tile;
          if ((!level || level->solved)
              && worldmap->path_ok(input_direction, tile_pos, &next_tile))
            {
              tile_pos = next_tile;
              moving = true;
              direction = input_direction;
              back_direction = reverse_dir(direction);
            }
          else if (input_direction == back_direction)
            {
              moving = true;
              direction = input_direction;
              tile_pos = worldmap->get_next_tile(tile_pos, direction);
              back_direction = reverse_dir(direction);
            }
        }
    }
  else
    {
      // Let tux walk a few pixels (20 pixel/sec)
      offset += 20.0f * delta;

      if (offset > 32)
        { // We reached the next tile, so we check what to do now
          offset -= 32;

          WorldMap::SpecialTile* special_tile = worldmap->at_special_tile();
          if(special_tile && special_tile->passive_message)
            {  // direction and the apply_action_ are opposites, since they "see"
               // directions in a different way
            if((direction == D_NORTH && special_tile->apply_action_south) ||
               (direction == D_SOUTH && special_tile->apply_action_north) ||
               (direction == D_WEST && special_tile->apply_action_east) ||
               (direction == D_EAST && special_tile->apply_action_west))
              {
              worldmap->passive_message = special_tile->map_message;
              worldmap->passive_message_timer.start(map_message_TIME);
              }
            }

          if (worldmap->at(tile_pos)->getData() & Tile::WORLDMAP_STOP ||
             (special_tile && !special_tile->passive_message) ||
              worldmap->at_level())
            {
              if(special_tile && !special_tile->map_message.empty() &&
                !special_tile->passive_message)
                worldmap->passive_message_timer.start(0);
              stop();
            }
          else
            {
              const Tile* tile = worldmap->at(tile_pos);
              if (direction != input_direction)
                { 
                  // Turn to a new direction
                  const Tile* tile = worldmap->at(tile_pos);

                  if((tile->getData() & Tile::WORLDMAP_NORTH 
                      && input_direction == D_NORTH) ||
                     (tile->getData() & Tile::WORLDMAP_SOUTH
                      && input_direction == D_SOUTH) ||
                     (tile->getData() & Tile::WORLDMAP_EAST
                      && input_direction == D_EAST) ||
                     (tile->getData() & Tile::WORLDMAP_WEST
                      && input_direction == D_WEST))
                    {  // player has changed direction during auto-movement
                      direction = input_direction;
                      back_direction = reverse_dir(direction);
                    }
                  else
                    {  // player has changed to impossible tile
                      back_direction = reverse_dir(direction);
                      stop();
                    }
                }
              else
                {
                Direction dir = D_NONE;
              
                if (tile->getData() & Tile::WORLDMAP_NORTH
                    && back_direction != D_NORTH)
                  dir = D_NORTH;
                else if (tile->getData() & Tile::WORLDMAP_SOUTH
                    && back_direction != D_SOUTH)
                  dir = D_SOUTH;
                else if (tile->getData() & Tile::WORLDMAP_EAST
                    && back_direction != D_EAST)
                  dir = D_EAST;
                else if (tile->getData() & Tile::WORLDMAP_WEST
                    && back_direction != D_WEST)
                  dir = D_WEST;

                if (dir != D_NONE)
                  {
                  direction = dir;
                  input_direction = direction;
                  back_direction = reverse_dir(direction);
                  }
                else
                  {
                  // Should never be reached if tiledata is good
                  stop();
                  return;
                  }
                }

              // Walk automatically to the next tile
              if(direction != D_NONE)
                {
                Vector next_tile;
                if (worldmap->path_ok(direction, tile_pos, &next_tile))
                  {
                  tile_pos = next_tile;
                  }
                else
                  {
                  puts("Tilemap data is buggy");
                  stop();
                  }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------

WorldMap::WorldMap()
{
  tile_manager = new TileManager("images/worldmap/antarctica.stwt");
  
  width  = 20;
  height = 15;
  
  start_x = 4;
  start_y = 5;

  tux = new Tux(this);
  
  leveldot_green = new Surface(datadir +  "/images/worldmap/leveldot_green.png", true);
  leveldot_red = new Surface(datadir +  "/images/worldmap/leveldot_red.png", true);
  messagedot   = new Surface(datadir +  "/images/worldmap/messagedot.png", true);
  teleporterdot   = new Surface(datadir +  "/images/worldmap/teleporterdot.png", true);

  enter_level = false;

  name = "<no title>";
  music = "salcon.mod";

  total_stats.reset();
}

WorldMap::~WorldMap()
{
  delete tux;
  delete tile_manager;

  delete leveldot_green;
  delete leveldot_red;
  delete messagedot;
  delete teleporterdot;
}

// Don't forget to set map_filename before calling this
void
WorldMap::load_map()
{
  levels_path = FileSystem::dirname(map_filename);

  try {
    lisp::Parser parser;
    std::string filename = get_resource_filename(map_filename);
    std::auto_ptr<lisp::Lisp> root (parser.parse(filename));

    const lisp::Lisp* lisp = root->get_lisp("supertux-worldmap");
    if(!lisp)
      throw new std::runtime_error("file isn't a supertux-worldmap file.");

    lisp::ListIterator iter(lisp);
    while(iter.next()) {
      if(iter.item() == "tilemap") {
        if(tilemap.size() > 0)
          throw new std::runtime_error("multiple tilemaps specified");
        
        const lisp::Lisp* tilemap_lisp = iter.lisp();
        tilemap_lisp->get("width",  width);
        tilemap_lisp->get("height", height);
        tilemap_lisp->get_vector("data", tilemap);
      } else if(iter.item() == "properties") {
        const lisp::Lisp* props = iter.lisp();
        props->get("name", name);
        props->get("music", music);
        props->get("start_pos_x", start_x);
        props->get("start_pos_y", start_y);
      } else if(iter.item() == "special-tiles") {
        parse_special_tiles(iter.lisp());
      } else {
        std::cerr << "Unknown token '" << iter.item() << "' in worldmap.\n";
      }
    }

    delete tux;
    tux = new Tux(this);
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when parsing worldmap '" << map_filename << "': " <<
      e.what();
    throw std::runtime_error(msg.str());
  }
}

void
WorldMap::parse_special_tiles(const lisp::Lisp* lisp)
{
  lisp::ListIterator iter(lisp);
  while(iter.next()) {
    if(iter.item() == "special-tile") {
      SpecialTile special_tile;

      const lisp::Lisp* lisp = iter.lisp();
      lisp->get("x", special_tile.pos.x);
      lisp->get("y", special_tile.pos.y);
      lisp->get("map-message", special_tile.map_message);
      special_tile.passive_message = false;
      lisp->get("passive-message", special_tile.passive_message);
      special_tile.teleport_dest = Vector(-1,-1);
      lisp->get("teleport-to-x", special_tile.teleport_dest.x);
      lisp->get("teleport-to-y", special_tile.teleport_dest.y);
      special_tile.invisible = false;
      lisp->get("invisible-tile", special_tile.invisible);

      special_tile.apply_action_north = true;
      special_tile.apply_action_south = true;
      special_tile.apply_action_east = true;
      special_tile.apply_action_west = true;

      std::string apply_direction;
      lisp->get("apply-to-direction", apply_direction);
      if(!apply_direction.empty()) {
        special_tile.apply_action_north = false;
        special_tile.apply_action_south = false;
        special_tile.apply_action_east = false;
        special_tile.apply_action_west = false;
        if(apply_direction.find("north") != std::string::npos)
          special_tile.apply_action_north = true;
        if(apply_direction.find("south") != std::string::npos)
          special_tile.apply_action_south = true;
        if(apply_direction.find("east") != std::string::npos)
          special_tile.apply_action_east = true;
        if(apply_direction.find("west") != std::string::npos)
          special_tile.apply_action_west = true;
      }
      
      special_tiles.push_back(special_tile);
    } else if(iter.item() == "level") {
      Level level;

      lisp::Lisp* level_lisp = iter.lisp();
      level.solved = false;
                      
      level.north = true;
      level.east  = true;
      level.south = true;
      level.west  = true;

      level_lisp->get("extro-filename", level.extro_filename);
      level_lisp->get("next-worldmap", level.next_worldmap);

      level.quit_worldmap = false;
      level_lisp->get("quit-worldmap", level.quit_worldmap);

      level_lisp->get("name", level.name);
      level_lisp->get("x", level.pos.x);
      level_lisp->get("y", level.pos.y);

      level.auto_path = true;
      level_lisp->get("auto-path", level.auto_path);

      level.vertical_flip = false;
      level_lisp->get("vertical-flip", level.vertical_flip);

      levels.push_back(level);
    } else {
      std::cerr << "Unknown token '" << iter.item() <<
        "' in worldmap special-tiles list.";
    }
  }
}

void
WorldMap::get_level_title(Level& level)
{
  /** get special_tile's title */
  level.title = "<no title>";

  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (
        parser.parse(get_resource_filename(levels_path + level.name)));

    const lisp::Lisp* level_lisp = root->get_lisp("supertux-level");
    if(!level_lisp)
      return;
    
    level_lisp->get("name", level.title);
  } catch(std::exception& e) {
    std::cerr << "Problem when reading leveltitle: " << e.what() << "\n";
    return;
  }
}

void WorldMap::calculate_total_stats()
{
  total_stats.reset();
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
    if (i->solved)
      {
      total_stats += i->statistics;
      }
    }
}

void
WorldMap::on_escape_press()
{
  // Show or hide the menu
  if(!Menu::current())
    {
    Menu::set_current(worldmap_menu); 
    tux->set_direction(D_NONE);  // stop tux movement when menu is called
    }
  else
    Menu::set_current(0); 
}

void
WorldMap::get_input()
{
  enter_level = false;
  SDLKey key;

  SDL_Event event;
  while (SDL_PollEvent(&event))
    {
      if (Menu::current())
        {
          Menu::current()->event(event);
        }
      else
        {
          switch(event.type)
            {
            case SDL_QUIT:
              Termination::abort("Received window close", "");
              break;
          
            case SDL_KEYDOWN:
              key = event.key.keysym.sym;

              if(key == SDLK_ESCAPE)
                on_escape_press();
              else if(key == SDLK_RETURN || key == keymap.power)
                enter_level = true;
              else if(key == SDLK_LEFT || key == keymap.power)
                tux->set_direction(D_WEST);
              else if(key == SDLK_RIGHT || key == keymap.right)
                tux->set_direction(D_EAST);
              else if(key == SDLK_UP || key == keymap.up ||
                key == keymap.jump)
                  // there might be ppl that use jump as up key
                tux->set_direction(D_NORTH);
              else if(key == SDLK_DOWN || key == keymap.down)
                tux->set_direction(D_SOUTH);
              break;

            case SDL_JOYHATMOTION:
              if(event.jhat.value & SDL_HAT_UP) {
                tux->set_direction(D_NORTH);
              } else if(event.jhat.value & SDL_HAT_DOWN) {
                tux->set_direction(D_SOUTH);
              } else if(event.jhat.value & SDL_HAT_LEFT) {
                tux->set_direction(D_WEST);
              } else if(event.jhat.value & SDL_HAT_RIGHT) {
                tux->set_direction(D_EAST);
              }
              break;
          
            case SDL_JOYAXISMOTION:
              if (event.jaxis.axis == joystick_keymap.x_axis)
                {
                  if (event.jaxis.value < -joystick_keymap.dead_zone)
                    tux->set_direction(D_WEST);
                  else if (event.jaxis.value > joystick_keymap.dead_zone)
                    tux->set_direction(D_EAST);
                }
              else if (event.jaxis.axis == joystick_keymap.y_axis)
                {
                  if (event.jaxis.value > joystick_keymap.dead_zone)
                    tux->set_direction(D_SOUTH);
                  else if (event.jaxis.value < -joystick_keymap.dead_zone)
                    tux->set_direction(D_NORTH);
                }
              break;

            case SDL_JOYBUTTONDOWN:
              if (event.jbutton.button == joystick_keymap.b_button)
                enter_level = true;
              else if (event.jbutton.button == joystick_keymap.start_button)
                on_escape_press();
              break;

            default:
              break;
            }
        }
    }
}

Vector
WorldMap::get_next_tile(Vector pos, Direction direction)
{
  switch(direction)
    {
    case D_WEST:
      pos.x -= 1;
      break;
    case D_EAST:
      pos.x += 1;
      break;
    case D_NORTH:
      pos.y -= 1;
      break;
    case D_SOUTH:
      pos.y += 1;
      break;
    case D_NONE:
      break;
    }
  return pos;
}

bool
WorldMap::path_ok(Direction direction, Vector old_pos, Vector* new_pos)
{
  *new_pos = get_next_tile(old_pos, direction);

  if (!(new_pos->x >= 0 && new_pos->x < width
        && new_pos->y >= 0 && new_pos->y < height))
    { // New position is outsite the tilemap
      return false;
    }
  else
    { // Check if the tile allows us to go to new_pos
      switch(direction)
        {
        case D_WEST:
          return (at(old_pos)->getData() & Tile::WORLDMAP_WEST
              && at(*new_pos)->getData() & Tile::WORLDMAP_EAST);

        case D_EAST:
          return (at(old_pos)->getData() & Tile::WORLDMAP_EAST
              && at(*new_pos)->getData() & Tile::WORLDMAP_WEST);

        case D_NORTH:
          return (at(old_pos)->getData() & Tile::WORLDMAP_NORTH
              && at(*new_pos)->getData() & Tile::WORLDMAP_SOUTH);

        case D_SOUTH:
          return (at(old_pos)->getData() & Tile::WORLDMAP_SOUTH
              && at(*new_pos)->getData() & Tile::WORLDMAP_NORTH);

        case D_NONE:
          assert(!"path_ok() can't work if direction is NONE");
        }
      return false;
    }
}

void
WorldMap::update(float delta)
{
  if (enter_level && !tux->is_moving())
    {
      /* Check special tile action */
      SpecialTile* special_tile = at_special_tile();
      if(special_tile)
        {
        if (special_tile->teleport_dest != Vector(-1,-1))
          {
          // TODO: an animation, camera scrolling or a fading would be a nice touch
          SoundManager::get()->play_sound(IDToSound(SND_WARP));
          tux->back_direction = D_NONE;
          tux->set_tile_pos(special_tile->teleport_dest);
          SDL_Delay(1000);
          }
        }

      /* Check level action */
      bool level_finished = true;
      Level* level = at_level();
      if (!level)
        {
        std::cout << "No level to enter at: "
          << tux->get_tile_pos().x << ", " << tux->get_tile_pos().y
          << std::endl;
        return;
        }


      if (level->pos == tux->get_tile_pos())
        {
          PlayerStatus old_player_status = player_status;

          std::cout << "Enter the current level: " << level->name << std::endl;
          // do a shriking fade to the level
          shrink_fade(Vector((level->pos.x*32 + 16 + offset.x),
                             (level->pos.y*32 + 16 + offset.y)), 500);
          GameSession session(get_resource_filename(levels_path + level->name),
                              ST_GL_LOAD_LEVEL_FILE, &level->statistics);

          switch (session.run())
            {
            case GameSession::ES_LEVEL_FINISHED:
              {
                level_finished = true;
                bool old_level_state = level->solved;
                level->solved = true;

                // deal with statistics
                level->statistics.merge(global_stats);
                calculate_total_stats();

                if (session.get_current_sector()->player->got_power !=
                      session.get_current_sector()->player->NONE_POWER)
                  player_status.bonus = PlayerStatus::FLOWER_BONUS;
                else if (session.get_current_sector()->player->size == BIG)
                  player_status.bonus = PlayerStatus::GROWUP_BONUS;
                else
                  player_status.bonus = PlayerStatus::NO_BONUS;

                if (old_level_state != level->solved && level->auto_path)
                  { // Try to detect the next direction to which we should walk
                    // FIXME: Mostly a hack
                    Direction dir = D_NONE;
                
                    const Tile* tile = at(tux->get_tile_pos());

                    if (tile->getData() & Tile::WORLDMAP_NORTH
                        && tux->back_direction != D_NORTH)
                      dir = D_NORTH;
                    else if (tile->getData() & Tile::WORLDMAP_SOUTH
                        && tux->back_direction != D_SOUTH)
                      dir = D_SOUTH;
                    else if (tile->getData() & Tile::WORLDMAP_EAST
                        && tux->back_direction != D_EAST)
                      dir = D_EAST;
                    else if (tile->getData() & Tile::WORLDMAP_WEST
                        && tux->back_direction != D_WEST)
                      dir = D_WEST;

                    if (dir != D_NONE)
                      {
                        tux->set_direction(dir);
                        //tux->update(delta);
                      }

                    std::cout << "Walk to dir: " << dir << std::endl;
                  }
              }

              break;
            case GameSession::ES_LEVEL_ABORT:
              level_finished = false;
              /* In case the player's abort the level, keep it using the old
                  status. But the minimum lives and no bonus. */
              player_status.distros = old_player_status.distros;
              player_status.lives = std::min(old_player_status.lives, player_status.lives);
              player_status.bonus = player_status.NO_BONUS;

              break;
            case GameSession::ES_GAME_OVER:
              {
              level_finished = false;
              /* draw an end screen */
              /* TODO: in the future, this should make a dialog a la SuperMario, asking
              if the player wants to restart the world map with no score and from
              level 1 */
              char str[80];

              DrawingContext context;
              context.draw_gradient(Color (200,240,220), Color(200,200,220),
                  LAYER_BACKGROUND0);

              context.draw_text(blue_text, _("GAMEOVER"), 
                  Vector(screen->w/2, 200), CENTER_ALLIGN, LAYER_FOREGROUND1);

              sprintf(str, _("COINS: %d"), player_status.distros);
              context.draw_text(gold_text, str,
                  Vector(screen->w/2, screen->w - 32), CENTER_ALLIGN,
                  LAYER_FOREGROUND1);

              total_stats.draw_message_info(context, _("Total Statistics"));

              context.do_drawing();

              SDL_Event event;
              wait_for_event(event,2000,6000,true);

              quit = true;
              player_status.reset();
              break;
              }
            case GameSession::ES_NONE:
              assert(false);
              // Should never be reached 
              break;
            }

          SoundManager::get()->play_music(song);
          Menu::set_current(0);
          if (!savegame_file.empty())
            savegame(savegame_file);
        }
      /* The porpose of the next checking is that if the player lost
         the level (in case there is one), don't show anything */
      if(level_finished)
        {
        if (!level->extro_filename.empty())
          {
          // Display a text file
          display_text_file(level->extro_filename, SCROLL_SPEED_MESSAGE,
              white_big_text , white_text, white_small_text, blue_text );
          }

        if (!level->next_worldmap.empty())
          {
          // Load given worldmap
          loadmap(level->next_worldmap);
          }
        if (level->quit_worldmap)
          quit = true;
        }
    }
  else
    {
      tux->action(delta);
//      tux->set_direction(input_direction);
    }
  
  Menu* menu = Menu::current();
  if(menu)
    {
      menu->action();

      if(menu == worldmap_menu)
        {
          switch (worldmap_menu->check())
            {
            case MNID_RETURNWORLDMAP: // Return to game
              break;
            case MNID_QUITWORLDMAP: // Quit Worldmap
              quit = true;
              break;
            }
        }
      else if(menu == options_menu)
        {
          process_options_menu();
        }
    }
}

const Tile*
WorldMap::at(Vector p)
{
  assert(p.x >= 0 
         && p.x < width
         && p.y >= 0
         && p.y < height);

  int x = int(p.x);
  int y = int(p.y);
  return tile_manager->get(tilemap[width * y + x]);
}

WorldMap::Level*
WorldMap::at_level()
{
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      if (i->pos == tux->get_tile_pos())
        return &*i; 
    }

  return 0;
}

WorldMap::SpecialTile*
WorldMap::at_special_tile()
{
  for(SpecialTiles::iterator i = special_tiles.begin(); i != special_tiles.end(); ++i)
    {
      if (i->pos == tux->get_tile_pos())
        return &*i; 
    }

  return 0;
}


void
WorldMap::draw(DrawingContext& context, const Vector& offset)
{
  for(int y = 0; y < height; ++y)
    for(int x = 0; x < width; ++x)
      {
        const Tile* tile = at(Vector(x, y));
        tile->draw(context, Vector(x*32 + offset.x, y*32 + offset.y),
            LAYER_TILES);
      }

  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      if (i->solved)
        context.draw_surface(leveldot_green,
            Vector(i->pos.x*32 + offset.x, i->pos.y*32 + offset.y), LAYER_TILES+1);
      else
        context.draw_surface(leveldot_red,
            Vector(i->pos.x*32 + offset.x, i->pos.y*32 + offset.y), LAYER_TILES+1);
    }

  for(SpecialTiles::iterator i = special_tiles.begin(); i != special_tiles.end(); ++i)
    {
      if(i->invisible)
        continue;

      if (i->teleport_dest != Vector(-1, -1))
        context.draw_surface(teleporterdot,
                Vector(i->pos.x*32 + offset.x, i->pos.y*32 + offset.y), LAYER_TILES+1);

      else if (!i->map_message.empty() && !i->passive_message)
        context.draw_surface(messagedot,
                Vector(i->pos.x*32 + offset.x, i->pos.y*32 + offset.y), LAYER_TILES+1);
    }

  tux->draw(context, offset);
  draw_status(context);
}

void
WorldMap::draw_status(DrawingContext& context)
{
  char str[80];
  sprintf(str, " %d", total_stats.get_points(SCORE_STAT));

  context.draw_text(white_text, _("SCORE"), Vector(0, 0), LEFT_ALLIGN, LAYER_FOREGROUND1);
  context.draw_text(gold_text, str, Vector(96, 0), LEFT_ALLIGN, LAYER_FOREGROUND1);

  sprintf(str, "%d", player_status.distros);
  context.draw_text(white_text, _("COINS"), Vector(screen->w/2 - 16*5, 0),
      LEFT_ALLIGN, LAYER_FOREGROUND1);
  context.draw_text(gold_text, str, Vector(screen->w/2 + (16*5)/2, 0),
        LEFT_ALLIGN, LAYER_FOREGROUND1);

  if (player_status.lives >= 5)
    {
      sprintf(str, "%dx", player_status.lives);
      context.draw_text(gold_text, str, 
          Vector(screen->w - gold_text->get_text_width(str) - tux_life->w, 0),
          LEFT_ALLIGN, LAYER_FOREGROUND1);
      context.draw_surface(tux_life, Vector(screen->w -
            gold_text->get_text_width("9"), 0), LEFT_ALLIGN, LAYER_FOREGROUND1);
    }
  else
    {
      for(int i= 0; i < player_status.lives; ++i)
        context.draw_surface(tux_life,
            Vector(screen->w - tux_life->w*4 + (tux_life->w*i), 0),
            LAYER_FOREGROUND1);
    }
  context.draw_text(white_text, _("LIVES"),
      Vector(screen->w - white_text->get_text_width(_("LIVES")) - white_text->get_text_width("   99"), 0),
      LEFT_ALLIGN, LAYER_FOREGROUND1);

  if (!tux->is_moving())
    {
      for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
        {
          if (i->pos == tux->get_tile_pos())
            {
              if(i->title == "")
                get_level_title(*i);

              context.draw_text(white_text, i->title, 
                  Vector(screen->w/2,
                         screen->h - white_text->get_height() - 30),
                  CENTER_ALLIGN, LAYER_FOREGROUND1);

              i->statistics.draw_worldmap_info(context);
              break;
            }
        }
      for(SpecialTiles::iterator i = special_tiles.begin(); i != special_tiles.end(); ++i)
        {
          if (i->pos == tux->get_tile_pos())
            {
               /* Display an in-map message in the map, if any as been selected */
              if(!i->map_message.empty() && !i->passive_message)
                context.draw_text(gold_text, i->map_message, 
                    Vector(screen->w/2,
                           screen->h - white_text->get_height() - 60),
                    CENTER_ALLIGN, LAYER_FOREGROUND1);
              break;
            }
        }
    }
  /* Display a passive message in the map, if needed */
  if(passive_message_timer.check())
    context.draw_text(gold_text, passive_message, 
            Vector(screen->w/2, screen->h - white_text->get_height() - 60),
            CENTER_ALLIGN, LAYER_FOREGROUND1);
}

void
WorldMap::display()
{
  Menu::set_current(0);

  quit = false;

  song = SoundManager::get()->load_music(datadir +  "/music/" + music);
  SoundManager::get()->play_music(song);

  FrameRate frame_rate(10);
  frame_rate.set_frame_limit(false);

  frame_rate.start();

  DrawingContext context;
  while(!quit)
    {
      float delta = frame_rate.get();

      delta *= 1.3f;

      if (delta > 10.0f)
        delta = .3f;
	
      frame_rate.update();

      Vector tux_pos = tux->get_pos();
      if (1)
        {
          offset.x = -tux_pos.x + screen->w/2;
          offset.y = -tux_pos.y + screen->h/2;

          if (offset.x > 0) offset.x = 0;
          if (offset.y > 0) offset.y = 0;

          if (offset.x < screen->w - width*32) offset.x = screen->w - width*32;
          if (offset.y < screen->h - height*32) offset.y = screen->h - height*32;
        } 

      draw(context, offset);
      get_input();
      update(delta);
      
      if(Menu::current())
        {
          Menu::current()->draw(context);
          mouse_cursor->draw(context);
        }

      context.do_drawing();

      SDL_Delay(20);
    }
}

void
WorldMap::savegame(const std::string& filename)
{
  if(filename == "")
    return;

  std::cout << "savegame: " << filename << std::endl;

  std::ofstream file(filename.c_str(), std::ios::out);
  lisp::Writer writer(file);

  int nb_solved_levels = 0, total_levels = 0;
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i) {
    ++total_levels;
    if (i->solved)
      ++nb_solved_levels;
  }
  char nb_solved_levels_str[80], total_levels_str[80];
  sprintf(nb_solved_levels_str, "%d", nb_solved_levels);
  sprintf(total_levels_str, "%d", total_levels);

  writer.write_comment("Worldmap save file");

  writer.start_list("supertux-savegame");

  writer.write_int("version", 1);
  writer.write_string("title",
      std::string(name + " - " + nb_solved_levels_str+"/"+total_levels_str));
  writer.write_string("map", map_filename);
  writer.write_int("lives", player_status.lives);
  writer.write_int("distros", player_status.lives);
  writer.write_int("max-score-multiplier", player_status.max_score_multiplier);

  writer.start_list("tux");

  writer.write_float("x", tux->get_tile_pos().x);
  writer.write_float("y", tux->get_tile_pos().y);
  writer.write_string("back", direction_to_string(tux->back_direction));
  writer.write_string("bonus", bonus_to_string(player_status.bonus));

  writer.end_list("tux");

  writer.start_list("levels");

  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      if (i->solved)
        {
        writer.start_list("level");

        writer.write_string("name", i->name);
        writer.write_bool("solved", true);
        i->statistics.write(writer);

        writer.end_list("level");
        }
    }  

  writer.end_list("levels");

  writer.end_list("supertux-savegame");
}

void
WorldMap::loadgame(const std::string& filename)
{
  std::cout << "loadgame: " << filename << std::endl;
  savegame_file = filename;

  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (parser.parse(filename));
  
    const lisp::Lisp* savegame = root->get_lisp("supertux-savegame");
    if(!savegame)
      throw std::runtime_error("File is not a supertux-savegame file.");

    /* Get the Map filename and then load it before setting level settings */
    std::string cur_map_filename = map_filename;
    savegame->get("map", map_filename);
    load_map(); 

    savegame->get("lives", player_status.lives);
    savegame->get("distros", player_status.distros);
    savegame->get("max-score-multiplier", player_status.max_score_multiplier);
    if (player_status.lives < 0)
      player_status.lives = START_LIVES;

    const lisp::Lisp* tux_lisp = savegame->get_lisp("tux");
    if(tux)
    {
      Vector p;
      std::string back_str = "none";
      std::string bonus_str = "none";

      tux_lisp->get("x", p.x);
      tux_lisp->get("y", p.y);
      tux_lisp->get("back", back_str);
      tux_lisp->get("bonus", bonus_str);
      
      player_status.bonus = string_to_bonus(bonus_str);
      tux->back_direction = string_to_direction(back_str);      
      tux->set_tile_pos(p);
    }

    const lisp::Lisp* levels_lisp = savegame->get_lisp("levels");
    if(levels_lisp) {
      lisp::ListIterator iter(levels_lisp);
      while(iter.next()) {
        if(iter.item() == "level") {
          std::string name;
          bool solved = false;

          const lisp::Lisp* level = iter.lisp();
          level->get("name", name);
          level->get("solved", solved);

          for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
          {
            if (name == i->name)
            {
              i->solved = solved;
              i->statistics.parse(*level);
              break;
            }
          }
        } else {
          std::cerr << "Unknown token '" << iter.item() 
            << "' in levels block in worldmap.\n";
        }
      }
    }
  } catch(std::exception& e) {
    std::cerr << "Problem loading game '" << filename << "': " << e.what() 
      << "\n";
    load_map();
    player_status.reset();
  }

  calculate_total_stats();
}

void
WorldMap::loadmap(const std::string& filename)
{
  savegame_file = "";
  map_filename = filename;
  load_map();
}

} // namespace WorldMapNS

/* Local Variables: */
/* mode:c++ */
/* End: */

