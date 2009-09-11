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
#include <assert.h>
#include <unistd.h>
#include "globals.h"
#include "texture.h"
#include "screen.h"
#include "lispreader.h"
#include "gameloop.h"
#include "setup.h"
#include "worldmap.h"
#include "resources.h"

extern int SCREEN_W;
extern int SCREEN_H;

extern int window_width;
extern int window_height;

#define DISPLAY_MAP_MESSAGE_TIME 2800

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
              reader.read_int("id",  &id);
              reader.read_bool("north", &tile->north);
              reader.read_bool("south", &tile->south);
              reader.read_bool("west",  &tile->west);
              reader.read_bool("east",  &tile->east);
              reader.read_bool("stop",  &tile->stop);
              reader.read_bool("auto-walk",  &tile->auto_walk);
              reader.read_string("image",  &filename);

              std::string temp;
              reader.read_string("one-way",  &temp);
              tile->one_way = BOTH_WAYS;
              if(!temp.empty())
                {
                if(temp == "north-south")
                  tile->one_way = NORTH_SOUTH_WAY;
                else if(temp == "south-north")
                  tile->one_way = SOUTH_NORTH_WAY;
                else if(temp == "east-west")
                  tile->one_way = EAST_WEST_WAY;
                else if(temp == "west-east")
                  tile->one_way = WEST_EAST_WAY;
                }

              tile->sprite = new Surface(
                           datadir +  "/images/worldmap/" + filename, 
                           USE_ALPHA);

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
  largetux_sprite = new Surface(datadir +  "/images/worldmap/tux.png", USE_ALPHA);
  firetux_sprite = new Surface(datadir +  "/images/worldmap/firetux.png", USE_ALPHA);
  smalltux_sprite = new Surface(datadir +  "/images/worldmap/smalltux.png", USE_ALPHA);

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
Tux::draw(const Point& offset)
{
  Point pos = get_pos();
  switch (player_status.bonus)
    {
    case PlayerStatus::GROWUP_BONUS:
      largetux_sprite->draw(pos.x + offset.x, 
                            pos.y + offset.y - 10);
      break;
    case PlayerStatus::FLOWER_BONUS:
      firetux_sprite->draw(pos.x + offset.x, 
                           pos.y + offset.y - 10);
      break;
    case PlayerStatus::NO_BONUS:
      smalltux_sprite->draw(pos.x + offset.x, 
                            pos.y + offset.y - 10);
      break;
    }
}


Point
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
  
  return Point((int)x, (int)y); 
}

void
Tux::stop()
{
  offset = 0;
  direction = D_NONE;
  moving = false;
}

void
Tux::update(float delta)
{
  if (!moving)
    {
      if (input_direction != D_NONE)
        { 
          WorldMapNS::WorldMap::Level* level = worldmap->at_level();

          // We got a new direction, so lets start walking when possible
          Point next_tile;
          if ((!level || level->solved || level->name.empty())
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

          WorldMap::Level* level = worldmap->at_level();
          if(level && level->name.empty() && !level->display_map_message.empty() &&
             level->passive_message)
            {  // direction and the apply_action_ are opposites, since they "see"
               // directions in a different way
            if((direction == D_NORTH && level->apply_action_south) ||
               (direction == D_SOUTH && level->apply_action_north) ||
               (direction == D_WEST && level->apply_action_east) ||
               (direction == D_EAST && level->apply_action_west))
              {
              worldmap->passive_message = level->display_map_message;
              worldmap->passive_message_timer.start(DISPLAY_MAP_MESSAGE_TIME);
              }
            }

          Tile* cur_tile = worldmap->at(tile_pos);
          if (cur_tile->stop || (level && (!level->name.empty() || level->teleport_dest_x != -1)))
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
              Point next_tile;
              if (worldmap->path_ok(direction, tile_pos, &next_tile))
                {
                  tile_pos = next_tile;
                }
              else
                {
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

  width  = (int)(20);
  height = (int)(15);
  
  start_x = int(4);
  start_y = int(5);

  passive_message_timer.init(true);

  level_sprite = new Surface(datadir +  "/images/worldmap/levelmarker.png", USE_ALPHA);
  leveldot_green = new Surface(datadir +  "/images/worldmap/leveldot_green.png", USE_ALPHA);
  leveldot_red = new Surface(datadir +  "/images/worldmap/leveldot_red.png", USE_ALPHA);
  leveldot_teleporter = new Surface(datadir +  "/images/worldmap/teleporter.png", USE_ALPHA);
  
  map_file = datadir + "/levels/worldmaps/world1.stwm";
  
  input_direction = D_NONE;
  enter_level = false;

  name = "<no file>";
  music = "salcon.mod";
}

WorldMap::~WorldMap()
{
  delete tux;
  delete tile_manager;

  delete level_sprite;
  delete leveldot_green;
  delete leveldot_red;
  delete leveldot_teleporter;
}

void
WorldMap::set_map_file(std::string mapfile)
{
  map_file = datadir + "/levels/worldmaps/" + mapfile;
}

void
WorldMap::load_map()
{
  lisp_object_t* root_obj = lisp_read_from_file(map_file);
  if (!root_obj)
    st_abort("Couldn't load file", map_file);
  
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-worldmap") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "tilemap") == 0)
            {
              LispReader reader(lisp_cdr(element));
              reader.read_int("width",  &width);
              reader.read_int("height", &height);
              reader.read_int_vector("data", &tilemap);
            }
          else if (strcmp(lisp_symbol(lisp_car(element)), "properties") == 0)
            {
              LispReader reader(lisp_cdr(element));
              reader.read_string("name",  &name);
              reader.read_string("music", &music);
   	      reader.read_int("start_pos_x", &start_x);
	      reader.read_int("start_pos_y", &start_y);
            }
          else if (strcmp(lisp_symbol(lisp_car(element)), "levels") == 0)
            {
              lisp_object_t* cur = lisp_cdr(element);
              
              while(!lisp_nil_p(cur))
                {
                  lisp_object_t* element = lisp_car(cur);
                  
                  if (strcmp(lisp_symbol(lisp_car(element)), "level") == 0)
                    {
                      Level level;
                      LispReader reader(lisp_cdr(element));
                      level.solved = false;
                      
                      level.north = true;
                      level.east  = true;
                      level.south = true;
                      level.west  = true;

                      reader.read_string("extro-filename",  &level.extro_filename);
                      reader.read_string("name",  &level.name);
                      reader.read_int("x", &level.x);
                      reader.read_int("y", &level.y);
                      reader.read_string("map-message", &level.display_map_message);
                      level.auto_path = true;
                      reader.read_bool("auto-path", &level.auto_path);
                      level.passive_message = true;
                      reader.read_bool("passive-message", &level.passive_message);
							 
							 level.invisible_teleporter = false;
							 level.teleport_dest_x = level.teleport_dest_y = -1;
							 reader.read_int("dest_x", &level.teleport_dest_x);
							 reader.read_int("dest_y", &level.teleport_dest_y);
							 reader.read_string("teleport-message", &level.teleport_message);
							 reader.read_bool("invisible-teleporter", &level.invisible_teleporter);
                      
							 level.apply_action_north = level.apply_action_south =
                            level.apply_action_east = level.apply_action_west = true;
                      reader.read_bool("apply-action-up", &level.apply_action_north);
                      reader.read_bool("apply-action-down", &level.apply_action_south);
                      reader.read_bool("apply-action-left", &level.apply_action_west);
                      reader.read_bool("apply-action-right", &level.apply_action_east);

                      if(!level.name.empty())
                        get_level_title(&level);   // get level's title

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

void WorldMap::get_level_title(Levels::pointer level)
{
  /** get level's title */
  level->title = "<no title>";

  FILE * fi;
  lisp_object_t* root_obj = 0;
  fi = fopen((datadir +  "/levels/" + level->name).c_str(), "r");
  if (fi == NULL)
  {
    perror((datadir +  "/levels/" + level->name).c_str());
    return;
  }

  lisp_stream_t stream;
  lisp_stream_init_file (&stream, fi);
  root_obj = lisp_read (&stream);

  if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
  {
    printf("World: Parse Error in file %s", level->name.c_str());
  }

  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-level") == 0)
  {
    LispReader reader(lisp_cdr(root_obj));
    reader.read_string("name",  &level->title);
  }

  lisp_free(root_obj);

  fclose(fi);
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
            case 258:
              switch(event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                case 265:
                  on_escape_press();
                  break;
                case SDLK_LCTRL:
                case SDLK_RETURN:
                case 259:
                  enter_level = true;
                  break;
                default:
                  break;
                }
              break;

#ifdef TSCONTROL
			case SDL_MOUSEBUTTONDOWN:
			  if (event.motion.y < screen->h/4) {
				input_direction = D_NORTH;
			  }
			  else if (event.motion.y > 3*screen->h/4) {
			    input_direction = D_SOUTH;
			  }
			  else if (event.motion.x < screen->w/4) {
			    input_direction = D_WEST;
			  }
			  else if (event.motion.x > 3*screen->w/4) {
			    input_direction = D_EAST;
			  }
			  else {
			    enter_level = true;
			  }
			break;
#endif

#ifndef GP2X      
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

	    case SDL_JOYHATMOTION:
	      if (event.jhat.value == SDL_HAT_UP)
                    input_direction = D_NORTH;
	      if (event.jhat.value == SDL_HAT_DOWN)
                    input_direction = D_SOUTH;
	      if (event.jhat.value == SDL_HAT_LEFT)
                    input_direction = D_WEST;
	      if (event.jhat.value == SDL_HAT_RIGHT)
                    input_direction = D_EAST;
	      break;

#endif
            case SDL_JOYBUTTONDOWN:
#ifndef GP2X
              if (event.jbutton.button == joystick_keymap.b_button)
                enter_level = true;
              else if (event.jbutton.button == joystick_keymap.start_button)
                on_escape_press();
              break;
#else
              if (event.jbutton.button == joystick_keymap.a_button)
                enter_level = true;
              else if (event.jbutton.button == joystick_keymap.start_button)
                on_escape_press();
              else if (event.jbutton.button == joystick_keymap.up_button)
                input_direction = D_NORTH;
              else if (event.jbutton.button == joystick_keymap.down_button)
                input_direction = D_SOUTH;
              else if (event.jbutton.button == joystick_keymap.right_button)
                input_direction = D_EAST;
              else if (event.jbutton.button == joystick_keymap.left_button)
                input_direction = D_WEST;
              break;
#endif

            default:
              break;
            }
        }
    }

  if (!Menu::current())
    {
      Uint8 *keystate = SDL_GetKeyState(NULL);
  
      if (keystate[SDLK_LEFT] || keystate[260])
        input_direction = D_WEST;
      else if (keystate[SDLK_RIGHT] || keystate[262])
        input_direction = D_EAST;
      else if (keystate[SDLK_UP] || keystate[264])
        input_direction = D_NORTH;
      else if (keystate[SDLK_DOWN] || keystate[258])
        input_direction = D_SOUTH;
    }
}

Point
WorldMap::get_next_tile(Point pos, Direction direction)
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
WorldMap::path_ok(Direction direction, Point old_pos, Point* new_pos)
{
  *new_pos = get_next_tile(old_pos, direction);

  if (!(new_pos->x >= 0 && new_pos->x < width
        && new_pos->y >= 0 && new_pos->y < height))
    { // New position is outsite the tilemap
      return false;
    }
  else if(at(*new_pos)->one_way != BOTH_WAYS)
    {
      if((at(*new_pos)->one_way == NORTH_SOUTH_WAY && direction != D_SOUTH) ||
         (at(*new_pos)->one_way == SOUTH_NORTH_WAY && direction != D_NORTH) ||
         (at(*new_pos)->one_way == EAST_WEST_WAY && direction != D_WEST) ||
         (at(*new_pos)->one_way == WEST_EAST_WAY && direction != D_EAST))
        return false;
      return true;
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
      Level* level = at_level();
      if (level && !level->name.empty())
        {
          if (level->x == tux->get_tile_pos().x && 
              level->y == tux->get_tile_pos().y)
            {
              std::cout << "Enter the current level: " << level->name << std::endl;;
              GameSession session(datadir +  "/levels/" + level->name,
                                  1, ST_GL_LOAD_LEVEL_FILE);

              switch (session.run())
                {
                case GameSession::ES_LEVEL_FINISHED:
                  {
                    bool old_level_state = level->solved;
                    level->solved = true;

                    if (session.get_world()->get_tux()->got_coffee)
                      player_status.bonus = PlayerStatus::FLOWER_BONUS;
                    else if (session.get_world()->get_tux()->size == BIG)
                      player_status.bonus = PlayerStatus::GROWUP_BONUS;
                    else
                      player_status.bonus = PlayerStatus::NO_BONUS;

                    if (old_level_state != level->solved && level->auto_path)
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

                    if (!level->extro_filename.empty())
                      { 
#ifndef NOSOUND
                        MusicRef theme =
                          music_manager->load_music(datadir + "/music/theme.mod");
#ifdef GP2X
                        MusicRef credits = music_manager->load_music(datadir + "/music/credits.xm");
#else
						MusicRef credits = music_manager->load_music(datadir + "/music/credits.ogg");
#endif
                        music_manager->play_music(theme);
#endif
                        // Display final credits and go back to the main menu
                        display_text_file(level->extro_filename,
                                          "/images/background/extro.jpg", SCROLL_SPEED_MESSAGE);
#ifndef NOSOUND
			music_manager->play_music(credits,0);
#endif
			display_text_file("CREDITS",
                                          "/images/background/oiltux.jpg", SCROLL_SPEED_CREDITS);
#ifndef NOSOUND
                        music_manager->play_music(theme);
#endif
                        quit = true;
                      }
                  }

                  break;
                case GameSession::ES_LEVEL_ABORT:
                  // Reseting the player_status might be a worthy
                  // consideration, but I don't think we need it
                  // 'cause only the bad players will use it to
                  // 'cheat' a few items and that isn't necesarry a
                  // bad thing (ie. better they continue that way,
                  // then stop playing the game all together since it
                  // is to hard)
                  break;
                case GameSession::ES_GAME_OVER:
                  quit = true;
                  player_status.reset();
                  break;
                case GameSession::ES_NONE:
                  // Should never be reached 
                  break;
                }

#ifndef NOSOUND
              music_manager->play_music(song);
#endif
              Menu::set_current(0);
              if (!savegame_file.empty())
                savegame(savegame_file);
              return;
            }
        }
      else if (level && level->teleport_dest_x != -1 && level->teleport_dest_y != -1) {
			if (level->x == tux->get_tile_pos().x && 
              level->y == tux->get_tile_pos().y)
				{
#ifndef NOSOUND
#ifndef GP2X
					play_sound(sounds[SND_TELEPORT], SOUND_CENTER_SPEAKER);
#else
					play_chunk(SND_TELEPORT);
					updateSound();
#endif
#endif
					tux->back_direction = D_NONE;
					tux->set_tile_pos(Point(level->teleport_dest_x, level->teleport_dest_y));
					SDL_Delay(1000);
				}
		}
		else
        {
          std::cout << "Nothing to enter at: "
                    << tux->get_tile_pos().x << ", " << tux->get_tile_pos().y << std::endl;
        }
    }
  else
    {
      tux->update(delta);
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
WorldMap::at(Point p)
{
  assert(p.x >= 0 
         && p.x < width
         && p.y >= 0
         && p.y < height);

  return tile_manager->get(tilemap[width * p.y + p.x]);
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
WorldMap::draw(const Point& offset)
{
  for(int y = 0; y < height; ++y)
    for(int x = 0; x < width; ++x)
      {
        Tile* tile = at(Point(x, y));
        tile->sprite->draw(x*32 + offset.x,
                           y*32 + offset.y);
      }
  
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      if(i->name.empty()) {
      	if ((i->teleport_dest_x != -1) && !i->invisible_teleporter) {
				leveldot_teleporter->draw(i->x*32 + offset.x, 
                             i->y*32 + offset.y);
			}
			else continue;
		}

      else if (i->solved)
        leveldot_green->draw(i->x*32 + offset.x, 
                             i->y*32 + offset.y);
      else
        leveldot_red->draw(i->x*32 + offset.x, 
                           i->y*32 + offset.y);        
    }

  tux->draw(offset);
  draw_status();
}

void
WorldMap::draw_status()
{
  int xdiv;
  
#ifdef RES320X240
  xdiv=2;
#else
  xdiv=1;
#endif

  char str[80];
  sprintf(str, "%d", player_status.score);
  white_text->draw("SCORE", 0, 0);
  gold_text->draw(str, (int)(96)/xdiv, 0);

  sprintf(str, "%d", player_status.distros);
  white_text->draw_align("COINS", (int)(320-64)/xdiv, 0,  A_LEFT, A_TOP);
  gold_text->draw_align(str, (int)(320+64)/xdiv, 0, A_RIGHT, A_TOP);

  white_text->draw("LIVES", (int)(480)/xdiv, 0);
  if (player_status.lives >= 5)
    {
      sprintf(str, "%dx", player_status.lives);
#ifdef RES320X240
      gold_text->draw_align(str, (int)(617)/xdiv-5, 0, A_RIGHT, A_TOP);
      tux_life->draw((int)((565-12+(18*3))), 0);
#else
      gold_text->draw_align(str, (int)(617), 0, A_RIGHT, A_TOP);
      tux_life->draw((int)((565+(18*3))), 0);
#endif
    }
  else
    {
      for(int i= 0; i < player_status.lives; ++i)
        tux_life->draw((565+(18/xdiv*i)),0);
    }

  if (!tux->is_moving())
    {
      for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
        {
          if (i->x == tux->get_tile_pos().x && 
              i->y == tux->get_tile_pos().y)
            {
              if(!i->name.empty())
                {
#ifndef RES320X240
              white_text->draw_align(i->title.c_str(), screen->w/2, screen->h,  A_HMIDDLE, A_BOTTOM);
#else
              white_text->draw_align(i->title.c_str(), SCREEN_W/2, SCREEN_H,  A_HMIDDLE, A_BOTTOM);
#endif
                }
				  else if (i->teleport_dest_x != -1) {
				  	if(!i->teleport_message.empty())
               	 gold_text->draw_align(i->teleport_message.c_str(), SCREEN_W/2, SCREEN_H,  A_HMIDDLE, A_BOTTOM);
				  }

              /* Display a message in the map, if any as been selected */
              if(!i->display_map_message.empty() && !i->passive_message)
                gold_text->draw_align(i->display_map_message.c_str(),
                                      SCREEN_W/2, SCREEN_H - 30,A_HMIDDLE, A_BOTTOM);
              break;
            }
        }
    }

  /* Display a passive message in the map, if needed */
  if(passive_message_timer.check())
    gold_text->draw_align(passive_message.c_str(),
                          SCREEN_W/2, SCREEN_H - 30,A_HMIDDLE, A_BOTTOM);
}

void
WorldMap::display()
{
  Menu::set_current(0);

  quit = false;

#ifndef NOSOUND
  song = music_manager->load_music(datadir +  "/music/" + music);
  music_manager->play_music(song);
#endif

  unsigned int last_update_time;
  unsigned int update_time;

  last_update_time = update_time = st_get_ticks();

  while(!quit)
    {
      float delta = ((float)(update_time-last_update_time))/100.0;

      delta *= 1.3f;

      if (delta > 10.0f)
        delta = .3f;
      
      last_update_time = update_time;
      update_time      = st_get_ticks();

      Point tux_pos = tux->get_pos();
      if (1)
        {
#ifndef GP2X
          offset.x = -tux_pos.x + screen->w/2;
          offset.y = -tux_pos.y + screen->h/2;

          if (offset.x > 0) offset.x = 0;
          if (offset.y > 0) offset.y = 0;

          if (offset.x < SCREEN_W - width*32) offset.x = SCREEN_W - width*32;
          if (offset.y < SCREEN_H - height*32) offset.y = SCREEN_H - height*32;
#else
          offset.x = -tux_pos.x + SCREEN_W/2;
          offset.y = -tux_pos.y + SCREEN_H/2;

          if (offset.x > 0) offset.x = 0;
          if (offset.y > 0) offset.y = 0;

          if (offset.x < 640 - width*32) offset.x = 640 - width*32;
          if (offset.y < 480 - height*32) offset.y = 480 - height*32;
#endif
        } 

      draw(offset);
      get_input();
      update(delta);

#ifndef TSCONTROL
      if(Menu::current())
        {
          Menu::current()->draw();
          mouse_cursor->draw();
        }
#else
      if(Menu::current())
        {
          Menu::current()->draw();
        }
        if (show_mouse) mouse_cursor->draw();
#endif
      flipscreen();

#ifndef NOSOUND
#ifdef GP2X
      updateSound();
#endif
#endif
      SDL_Delay(20);
    }
}

void
WorldMap::savegame(const std::string& filename)
{
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
      << "  (title  \"Icyisland - " << nb_solved_levels << "/" << levels.size() << "\")\n"
      << "  (lives   " << player_status.lives << ")\n"
      << "  (score   " << player_status.score << ")\n"
      << "  (distros " << player_status.distros << ")\n"
      << "  (tux (x " << tux->get_tile_pos().x << ") (y " << tux->get_tile_pos().y << ")\n"
      << "       (back \"" << direction_to_string(tux->back_direction) << "\")\n"
      << "       (bonus \"" << bonus_to_string(player_status.bonus) <<  "\"))\n"
      << "  (levels\n";
  
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      if (i->solved && !i->name.empty())
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

  if (access(filename.c_str(), F_OK) != 0)
    return;
  
  lisp_object_t* savegame = lisp_read_from_file(filename);
  if (!savegame)
    {
      std::cout << "WorldMap:loadgame: File not found: " << filename << std::endl;
      return;
    }

  lisp_object_t* cur = savegame;

  if (strcmp(lisp_symbol(lisp_car(cur)), "supertux-savegame") != 0)
    return;

  cur = lisp_cdr(cur);
  LispReader reader(cur);

  reader.read_int("lives",  &player_status.lives);
  reader.read_int("score",  &player_status.score);
  reader.read_int("distros", &player_status.distros);

  if (player_status.lives < 0)
    player_status.lives = START_LIVES;

  lisp_object_t* tux_cur = 0;
  if (reader.read_lisp("tux", &tux_cur))
    {
      Point p;
      std::string back_str = "none";
      std::string bonus_str = "none";

      LispReader tux_reader(tux_cur);
      tux_reader.read_int("x", &p.x);
      tux_reader.read_int("y", &p.y);
      tux_reader.read_string("back", &back_str);
      tux_reader.read_string("bonus", &bonus_str);
      
      player_status.bonus = string_to_bonus(bonus_str);
      tux->back_direction = string_to_direction(back_str);      
      tux->set_tile_pos(p);
    }

  lisp_object_t* level_cur = 0;
  if (reader.read_lisp("levels", &level_cur))
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
              level_reader.read_string("name",   &name);
              level_reader.read_bool("solved", &solved);

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
  set_map_file(filename);
  load_map();
}

} // namespace WorldMapNS

/* Local Variables: */
/* mode:c++ */
/* End: */

