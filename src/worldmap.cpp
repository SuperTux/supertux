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

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <unistd.h>

#include "app/globals.h"
#include "video/surface.h"
#include "video/screen.h"
#include "video/drawing_context.h"
#include "utils/lispreader.h"
#include "gameloop.h"
#include "app/setup.h"
#include "sector.h"
#include "worldmap.h"
#include "audio/sound_manager.h"
#include "resources.h"
#include "app/gettext.h"
#include "misc.h"

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

TileManager::TileManager()
{
  std::string stwt_filename = datadir +  "/images/worldmap/antarctica.stwt";
  lisp_object_t* root_obj = lisp_read_from_file(stwt_filename);
 
  if (!root_obj)
    st_abort("Couldn't load file", stwt_filename);

  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-worldmap-tiles") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "tile") == 0)
            {
              int id = 0;
              std::string filename = "<invalid>";

              Tile* tile = new Tile;             
              tile->north = true;
              tile->east  = true;
              tile->south = true;
              tile->west  = true;
              tile->stop  = true;
              tile->auto_walk = false;
  
              LispReader reader(lisp_cdr(element));
              reader.read_int("id", id);
              reader.read_bool("north", tile->north);
              reader.read_bool("south", tile->south);
              reader.read_bool("west",  tile->west);
              reader.read_bool("east",  tile->east);
              reader.read_bool("stop",  tile->stop);
              reader.read_bool("auto-walk",  tile->auto_walk);
              reader.read_string("image", filename);

              tile->sprite = new Surface(
                           datadir +  "/images/worldmap/" + filename, 
                           true);

              if (id >= int(tiles.size()))
                tiles.resize(id+1);

              tiles[id] = tile;
            }
          else
            {
              puts("Unhandled symbol");
            }

          cur = lisp_cdr(cur);
        }
    }
  else
    {
      assert(0);
    }

  lisp_free(root_obj);
}

TileManager::~TileManager()
{
  for(std::vector<Tile*>::iterator i = tiles.begin(); i != tiles.end(); ++i)
    delete *i;
}

Tile*
TileManager::get(int i)
{
  assert(i >=0 && i < int(tiles.size()));
  return tiles[i];
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
  moving = false;
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
              std::cout << "Back triggered" << std::endl;
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

          if (worldmap->at(tile_pos)->stop || worldmap->at_level())
            {
              stop();
            }
          else
            {
              if (worldmap->at(tile_pos)->auto_walk)
                { // Turn to a new direction
                  Tile* tile = worldmap->at(tile_pos);
                  Direction dir = D_NONE;
                  
                  if (tile->north && back_direction != D_NORTH)
                    dir = D_NORTH;
                  else if (tile->south && back_direction != D_SOUTH)
                    dir = D_SOUTH;
                  else if (tile->east && back_direction != D_EAST)
                    dir = D_EAST;
                  else if (tile->west && back_direction != D_WEST)
                    dir = D_WEST;

                  if (dir != D_NONE)
                    {
                      direction = dir;
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

//---------------------------------------------------------------------------
Tile::Tile()
{
}

Tile::~Tile()
{
  delete sprite;
}

//---------------------------------------------------------------------------

WorldMap::WorldMap()
{
  tile_manager = new TileManager();
  //tux = new Tux(this);
  
  width  = 20;
  height = 15;
  
  start_x = 4;
  start_y = 5;
  
  level_sprite = new Surface(datadir +  "/images/worldmap/levelmarker.png", true);
  leveldot_green = new Surface(datadir +  "/images/worldmap/leveldot_green.png", true);
  leveldot_red = new Surface(datadir +  "/images/worldmap/leveldot_red.png", true);

  input_direction = D_NONE;
  enter_level = false;

  name = "<no title>";
  music = "SALCON.MOD";
}

WorldMap::~WorldMap()
{
  delete tux;
  delete tile_manager;

  delete level_sprite;
  delete leveldot_green;
  delete leveldot_red;
}

void
WorldMap::load_map()
{
  lisp_object_t* root_obj = lisp_read_from_file(datadir + "/levels/worldmap/" + map_filename);
  if (!root_obj)
    st_abort("Couldn't load file", datadir + "/levels/worldmap/" + map_filename);

  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-worldmap") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "tilemap") == 0)
            {
              LispReader reader(lisp_cdr(element));
              reader.read_int("width",  width);
              reader.read_int("height", height);
              reader.read_int_vector("data", tilemap);
            }
          else if (strcmp(lisp_symbol(lisp_car(element)), "properties") == 0)
            {
              LispReader reader(lisp_cdr(element));
              reader.read_string("name", name, true);
              reader.read_string("music", music);
	      reader.read_int("start_pos_x", start_x);
	      reader.read_int("start_pos_y", start_y);
            }
          else if (strcmp(lisp_symbol(lisp_car(element)), "levels") == 0)
            {
              lisp_object_t* cur = lisp_cdr(element);
              
              while(!lisp_nil_p(cur))
                {
                  lisp_object_t* element = lisp_car(cur);

                  if (strcmp(lisp_symbol(lisp_car(element)), "special-tile") == 0)
                    {
                      Level level;
                      LispReader reader(lisp_cdr(element));
                      level.solved = false;
                      
                      level.north = true;
                      level.east  = true;
                      level.south = true;
                      level.west  = true;

                      reader.read_string("extro-filename", level.extro_filename);
                      reader.read_string("map-message", level.display_map_message);
                      reader.read_string("next-world", level.next_worldmap);
                      reader.read_string("level", level.name, true);
                      reader.read_int("x", level.x);
                      reader.read_int("y", level.y);
                      level.swap_x = level.swap_y = -1;
                      reader.read_int("swap-x", level.swap_x);
                      reader.read_int("swap-y", level.swap_y);
                      level.vertical_flip = false;
                      reader.read_bool("flip-level", level.vertical_flip);
                      level.quit_worldmap = false;
                      reader.read_bool("exit-game", level.quit_worldmap);

                      levels.push_back(level);
                    }

                  /* Kept for backward compability */
                  else if (strcmp(lisp_symbol(lisp_car(element)), "level") == 0)
                    {
                      Level level;
                      LispReader reader(lisp_cdr(element));
                      level.solved = false;
                      
                      level.north = true;
                      level.east  = true;
                      level.south = true;
                      level.west  = true;

                      reader.read_string("extro-filename", level.extro_filename);
                      if(!level.extro_filename.empty())
                        level.quit_worldmap = true;
                      reader.read_string("name", level.name, true);
                      reader.read_int("x", level.x);
                      reader.read_int("y", level.y);
                      level.vertical_flip = false;
                      level.swap_x = level.swap_y = -1;

                      levels.push_back(level);
                    }
                  
                  cur = lisp_cdr(cur);      
                }
            }
          else
            {
              
            }
          
          cur = lisp_cdr(cur);
        }
    }

    lisp_free(root_obj);
    tux = new Tux(this);
}

void WorldMap::get_level_title(Level& level)
{
  /** get level's title */
  level.title = "<no title>";

  LispReader* reader = LispReader::load(datadir + "/levels/" + level.name, "supertux-level");
  if(!reader)
    {
    std::cerr << "Error: Could not open level file. Ignoring...\n";
    return;
    }

  reader->read_string("name", level.title, true);
  delete reader;
}

void
WorldMap::on_escape_press()
{
  // Show or hide the menu
  if(!Menu::current())
    Menu::set_current(worldmap_menu); 
  else
    Menu::set_current(0); 
}

void
WorldMap::get_input()
{
  enter_level = false;
  input_direction = D_NONE;
   
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
              st_abort("Received window close", "");
              break;
          
            case SDL_KEYDOWN:
              switch(event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                  on_escape_press();
                  break;
                case SDLK_LCTRL:
                case SDLK_RETURN:
                  enter_level = true;
                  break;
                default:
                  break;
                }
              break;
          
            case SDL_JOYAXISMOTION:
              if (event.jaxis.axis == joystick_keymap.x_axis)
                {
                  if (event.jaxis.value < -joystick_keymap.dead_zone)
                    input_direction = D_WEST;
                  else if (event.jaxis.value > joystick_keymap.dead_zone)
                    input_direction = D_EAST;
                }
              else if (event.jaxis.axis == joystick_keymap.y_axis)
                {
                  if (event.jaxis.value > joystick_keymap.dead_zone)
                    input_direction = D_SOUTH;
                  else if (event.jaxis.value < -joystick_keymap.dead_zone)
                    input_direction = D_NORTH;
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

  if (!Menu::current())
    {
      Uint8 *keystate = SDL_GetKeyState(NULL);
  
      if (keystate[SDLK_LEFT])
        input_direction = D_WEST;
      else if (keystate[SDLK_RIGHT])
        input_direction = D_EAST;
      else if (keystate[SDLK_UP])
        input_direction = D_NORTH;
      else if (keystate[SDLK_DOWN])
        input_direction = D_SOUTH;
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
    { // Check if we the tile allows us to go to new_pos
      switch(direction)
        {
        case D_WEST:
          return (at(old_pos)->west && at(*new_pos)->east);

        case D_EAST:
          return (at(old_pos)->east && at(*new_pos)->west);

        case D_NORTH:
          return (at(old_pos)->north && at(*new_pos)->south);

        case D_SOUTH:
          return (at(old_pos)->south && at(*new_pos)->north);

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
      bool level_finished = true;
      Level* level = at_level();
      if (!level)
        {
        std::cout << "Nothing to enter at: "
          << tux->get_tile_pos().x << ", " << tux->get_tile_pos().y << std::endl;
        return;
        }


      if(!level->name.empty())
        {
          if (level->x == tux->get_tile_pos().x && 
              level->y == tux->get_tile_pos().y)
            {
              PlayerStatus old_player_status = player_status;

              std::cout << "Enter the current level: " << level->name << std::endl;
              // do a shriking fade to the level
              shrink_fade(Vector((level->x*32 + 16 + offset.x),(level->y*32 + 16
                      + offset.y)), 500);
              GameSession session(datadir +  "/levels/" + level->name,
                                  ST_GL_LOAD_LEVEL_FILE, level->vertical_flip);

              switch (session.run())
                {
                case GameSession::ES_LEVEL_FINISHED:
                  {
                    level_finished = true;
                    bool old_level_state = level->solved;
                    level->solved = true;

                    if (session.get_current_sector()->player->got_power !=
                          session.get_current_sector()->player->NONE_POWER)
                      player_status.bonus = PlayerStatus::FLOWER_BONUS;
                    else if (session.get_current_sector()->player->size == BIG)
                      player_status.bonus = PlayerStatus::GROWUP_BONUS;
                    else
                      player_status.bonus = PlayerStatus::NO_BONUS;

                    if (old_level_state != level->solved)
                      { // Try to detect the next direction to which we should walk
                        // FIXME: Mostly a hack
                        Direction dir = D_NONE;
                    
                        Tile* tile = at(tux->get_tile_pos());

                        if (tile->north && tux->back_direction != D_NORTH)
                          dir = D_NORTH;
                        else if (tile->south && tux->back_direction != D_SOUTH)
                          dir = D_SOUTH;
                        else if (tile->east && tux->back_direction != D_EAST)
                          dir = D_EAST;
                        else if (tile->west && tux->back_direction != D_WEST)
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
                  player_status.score = old_player_status.score;
                  player_status.distros = old_player_status.distros;
                  player_status.lives = std::min(old_player_status.lives, player_status.lives);
                  player_status.bonus = player_status.NO_BONUS;

                  break;
                case GameSession::ES_GAME_OVER:
                  {
                  level_finished = false;
                  /* draw an end screen */
                  /* in the future, this should make a dialog a la SuperMario, asking
                  if the player wants to restart the world map with no score and from
                  level 1 */
                  char str[80];

                  DrawingContext context;
                  context.draw_gradient(Color (200,240,220), Color(200,200,220),
                      LAYER_BACKGROUND0);

                  context.draw_text_center(blue_text, _("GAMEOVER"), 
                      Vector(0, 200), LAYER_FOREGROUND1);

                  sprintf(str, _("SCORE: %d"), player_status.score);
                  context.draw_text_center(gold_text, str,
                      Vector(0, 230), LAYER_FOREGROUND1);

                  sprintf(str, _("COINS: %d"), player_status.distros);
                  context.draw_text_center(gold_text, str,
                      Vector(0, screen->w - 32), LAYER_FOREGROUND1);

                  context.do_drawing();
  
                  SDL_Event event;
                  wait_for_event(event,2000,5000,true);

                  quit = true;
                  player_status.reset();
                  break;
                  }
                case GameSession::ES_NONE:
                  assert(false);
                  // Should never be reached 
                  break;
                }

              sound_manager->play_music(song);
              Menu::set_current(0);
              if (!savegame_file.empty())
                savegame(savegame_file);
            }
        }
      /* The porpose of the next checking is that if the player lost
         the level (in case there is one), don't show anything */
      if(level_finished)
        {
        if (!level->extro_filename.empty())
          {
          // Display a text file
          display_text_file(level->extro_filename, SCROLL_SPEED_MESSAGE);
          }
        if (level->swap_x != -1 && level->swap_y != -1)
          {
          // TODO: add an effect, like a camera scrolling, or at least, a fading
          tux->set_tile_pos(Vector(level->swap_x, level->swap_y));
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
      tux->set_direction(input_direction);
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

Tile*
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
      if (i->x == tux->get_tile_pos().x && 
          i->y == tux->get_tile_pos().y)
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
        Tile* tile = at(Vector(x, y));
        context.draw_surface(tile->sprite,
            Vector(x*32 + offset.x, y*32 + offset.y), LAYER_TILES);
      }
  
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      if(i->name.empty())
        continue;

      if (i->solved)
        context.draw_surface(leveldot_green,
            Vector(i->x*32 + offset.x, i->y*32 + offset.y), LAYER_TILES+1);
      else
        context.draw_surface(leveldot_red,
            Vector(i->x*32 + offset.x, i->y*32 + offset.y), LAYER_TILES+1);
    }

  tux->draw(context, offset);
  draw_status(context);
}

void
WorldMap::draw_status(DrawingContext& context)
{
  char str[80];
  sprintf(str, " %d", player_status.score);

  context.draw_text(white_text, _("SCORE"), Vector(0, 0), LAYER_FOREGROUND1);
  context.draw_text(gold_text, str, Vector(96, 0), LAYER_FOREGROUND1);

  sprintf(str, "%d", player_status.distros);
  context.draw_text(white_text, _("COINS"), Vector(screen->w/2 - 16*5, 0),
      LAYER_FOREGROUND1);
  context.draw_text(gold_text, str, Vector(screen->w/2 + (16*5)/2, 0),
        LAYER_FOREGROUND1);

  if (player_status.lives >= 5)
    {
      sprintf(str, "%dx", player_status.lives);
      context.draw_text(gold_text, str, 
          Vector(screen->w - gold_text->get_text_width(str) - tux_life->w, 0),
          LAYER_FOREGROUND1);
      context.draw_surface(tux_life, Vector(screen->w -
            gold_text->get_text_width("9"), 0), LAYER_FOREGROUND1);
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
      LAYER_FOREGROUND1);

  if (!tux->is_moving())
    {
      for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
        {
          if (i->x == tux->get_tile_pos().x && 
              i->y == tux->get_tile_pos().y)
            {
              if(!i->name.empty())
                {
                if(i->title == "")
                  get_level_title(*i);

                context.draw_text_center(white_text, i->title, 
                    Vector(0, screen->h - white_text->get_height() - 30),
                    LAYER_FOREGROUND1);
                }

              /* Display a message in the map, if any as been selected */
              if(!i->display_map_message.empty())
                context.draw_text_center(gold_text, i->display_map_message, 
                    Vector(0, screen->h - white_text->get_height() - 60),
                    LAYER_FOREGROUND1);
              break;
            }
        }
    }
}

void
WorldMap::display()
{
  Menu::set_current(0);

  quit = false;

  song = sound_manager->load_music(datadir +  "/music/" + music);
  sound_manager->play_music(song);
  
  unsigned int last_update_time;
  unsigned int update_time;

  last_update_time = update_time = st_get_ticks();

  DrawingContext context;
  while(!quit)
    {
      float delta = ((float)(update_time-last_update_time))/100.0;

      delta *= 1.3f;

      if (delta > 10.0f)
        delta = .3f;
      
      last_update_time = update_time;
      update_time      = st_get_ticks();

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
  if(filename != "")
    return;

  std::cout << "savegame: " << filename << std::endl;
  std::ofstream out(filename.c_str());

  int nb_solved_levels = 0;
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      if (i->solved)
        ++nb_solved_levels;
    }

  out << "(supertux-savegame\n"
      << "  (version 1)\n"
      << "  (title  \"" << name << " - " << nb_solved_levels << "/" << levels.size() << "\")\n"
      << "  (map    \"" << map_filename << "\")\n"
      << "  (lives   " << player_status.lives << ")\n"
      << "  (score   " << player_status.score << ")\n"
      << "  (distros " << player_status.distros << ")\n"
      << "  (tux (x " << tux->get_tile_pos().x << ") (y " << tux->get_tile_pos().y << ")\n"
      << "       (back \"" << direction_to_string(tux->back_direction) << "\")\n"
      << "       (bonus \"" << bonus_to_string(player_status.bonus) <<  "\"))\n"
      << "  (levels\n";
  
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      if (i->solved)
        {
          out << "     (level (name \"" << i->name << "\")\n"
              << "            (solved #t))\n";
        }
    }  

  out << "   )\n"
      << " )\n\n;; EOF ;;" << std::endl;
}

void
WorldMap::loadgame(const std::string& filename)
{
  std::cout << "loadgame: " << filename << std::endl;
  savegame_file = filename;
  map_filename = "icyisland.stwm";

  if (access(filename.c_str(), F_OK) != 0)
    {
    load_map();
    return;
    }
  
  lisp_object_t* savegame = lisp_read_from_file(filename);
  if (!savegame)
    {
      std::cout << "WorldMap:loadgame: File not found: " << filename << std::endl;
      load_map();
      return;
    }

  lisp_object_t* cur = savegame;

  if (strcmp(lisp_symbol(lisp_car(cur)), "supertux-savegame") != 0)
    {
    load_map();
    return;
    }

  cur = lisp_cdr(cur);
  LispReader reader(cur);

  /* Get the Map filename and then load it before setting level settings */
  reader.read_string("map", map_filename);
  load_map(); 

  reader.read_int("lives", player_status.lives);
  reader.read_int("score", player_status.score);
  reader.read_int("distros", player_status.distros);

  if (player_status.lives < 0)
    player_status.lives = START_LIVES;

  lisp_object_t* tux_cur = 0;
  if (reader.read_lisp("tux", tux_cur))
    {
      Vector p;
      std::string back_str = "none";
      std::string bonus_str = "none";

      LispReader tux_reader(tux_cur);
      tux_reader.read_float("x", p.x);
      tux_reader.read_float("y", p.y);
      tux_reader.read_string("back", back_str);
      tux_reader.read_string("bonus", bonus_str);
      
      player_status.bonus = string_to_bonus(bonus_str);
      tux->back_direction = string_to_direction(back_str);      
      tux->set_tile_pos(p);
    }

  lisp_object_t* level_cur = 0;
  if (reader.read_lisp("levels", level_cur))
    {
      while(level_cur)
        {
          lisp_object_t* sym  = lisp_car(lisp_car(level_cur));
          lisp_object_t* data = lisp_cdr(lisp_car(level_cur));

          if (strcmp(lisp_symbol(sym), "level") == 0)
            {
              std::string name;
              bool solved = false;

              LispReader level_reader(data);
              level_reader.read_string("name", name, true);
              level_reader.read_bool("solved", solved);

              for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
                {
                  if (name == i->name)
                    i->solved = solved;
                }
            }

          level_cur = lisp_cdr(level_cur);
        }
    }
 
  lisp_free(savegame);
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

