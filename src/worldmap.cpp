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
#include "globals.h"
#include "texture.h"
#include "screen.h"
#include "lispreader.h"
#include "gameloop.h"
#include "setup.h"
#include "worldmap.h"
#include "resources.h"

namespace WorldMapNS {

Direction reverse_dir(Direction direction)
{
  switch(direction)
    {
    case WEST:
      return EAST;
    case EAST:
      return WEST;
    case NORTH:
      return SOUTH;
    case SOUTH:
      return NORTH;
    case NONE:
      return NONE;
    }
  return NONE;
}

std::string
direction_to_string(Direction direction)
{
  switch(direction)
    {
    case WEST:
      return "west";
    case EAST:
      return "east";
    case NORTH:
      return "north";
    case SOUTH:
      return "south";
    default:
      return "none";
    }
}

Direction
string_to_direction(const std::string& directory)
{
  if (directory == "west")
    return WEST;
  else if (directory == "east")
    return EAST;
  else if (directory == "north")
    return NORTH;
  else if (directory == "south")
    return SOUTH;
  else
    return NONE;
}

TileManager* TileManager::instance_  = 0;

TileManager::TileManager()
{
  std::string stwt_filename = datadir +  "images/worldmap/antarctica.stwt";
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
  
              LispReader reader(lisp_cdr(element));
              reader.read_int("id",  &id);
              reader.read_bool("north", &tile->north);
              reader.read_bool("south", &tile->south);
              reader.read_bool("west",  &tile->west);
              reader.read_bool("east",  &tile->east);
              reader.read_bool("stop",  &tile->stop);
              reader.read_string("image",  &filename);

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
}

Tile*
TileManager::get(int i)
{
  assert(i >=0 && i < int(tiles.size()));
  return tiles[i];
}

Tux::Tux(WorldMap* worldmap_)
  : worldmap(worldmap_)
{
  sprite = new Surface(datadir +  "/images/worldmap/tux.png", USE_ALPHA);
  offset = 0;
  moving = false;
  tile_pos.x = 5;
  tile_pos.y = 5;
  direction = NONE;
  input_direction = NONE;
}

void
Tux::draw(const Point& offset)
{
  Point pos = get_pos();
  sprite->draw(pos.x + offset.x, 
               pos.y + offset.y);
}


Point
Tux::get_pos()
{
  float x = tile_pos.x * 32;
  float y = tile_pos.y * 32;

  switch(direction)
    {
    case WEST:
      x -= offset - 32;
      break;
    case EAST:
      x += offset - 32;
      break;
    case NORTH:
      y -= offset - 32;
      break;
    case SOUTH:
      y += offset - 32;
      break;
    case NONE:
      break;
    }
  
  return Point((int)x, (int)y); 
}

void
Tux::stop()
{
  offset = 0;
  direction = NONE;
  moving = false;
}

void
Tux::update(float delta)
{
  if (!moving)
    {
      if (input_direction != NONE)
        { 
          WorldMap::Level* level = worldmap->at_level();

          // We got a new direction, so lets start walking when possible
          Point next_tile;
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
              // Walk automatically to the next tile
              Point next_tile;
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

WorldMap::WorldMap()
{
  tux = new Tux(this);

  width  = 20;
  height = 15;

  level_sprite = new Surface(datadir +  "/images/worldmap/levelmarker.png", USE_ALPHA);
  leveldot_green = new Surface(datadir +  "/images/worldmap/leveldot_green.png", USE_ALPHA);
  leveldot_red = new Surface(datadir +  "/images/worldmap/leveldot_red.png", USE_ALPHA);

  input_direction = NONE;
  enter_level = false;

  name = "<no file>";
  music = "SALCON.MOD";

  load_map();
}

WorldMap::~WorldMap()
{
  delete tux;
}

void
WorldMap::load_map()
{
  std::string filename = datadir +  "levels/default/worldmap.stwm";
  
  lisp_object_t* root_obj = lisp_read_from_file(filename);
  if (!root_obj)
    st_abort("Couldn't load file", filename);
  
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

                      reader.read_string("name",  &level.name);
                      reader.read_int("x", &level.x);
                      reader.read_int("y", &level.y);

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
}

void WorldMap::get_level_title(Levels::pointer level)
{
/** get level's title */
level->title = "<no title>";

FILE * fi;
lisp_object_t* root_obj = 0;
fi = fopen((datadir +  "levels/" + level->name).c_str(), "r");
if (fi == NULL)
  {
  perror((datadir +  "levels/" + level->name).c_str());
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
  input_direction = NONE;
   
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
                    input_direction = WEST;
                  else if (event.jaxis.value > joystick_keymap.dead_zone)
                    input_direction = EAST;
                }
              else if (event.jaxis.axis == joystick_keymap.y_axis)
                {
                  if (event.jaxis.value > joystick_keymap.dead_zone)
                    input_direction = SOUTH;
                  else if (event.jaxis.value < -joystick_keymap.dead_zone)
                    input_direction = NORTH;
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
        input_direction = WEST;
      else if (keystate[SDLK_RIGHT])
        input_direction = EAST;
      else if (keystate[SDLK_UP])
        input_direction = NORTH;
      else if (keystate[SDLK_DOWN])
        input_direction = SOUTH;
    }
}

Point
WorldMap::get_next_tile(Point pos, Direction direction)
{
  switch(direction)
    {
    case WEST:
      pos.x -= 1;
      break;
    case EAST:
      pos.x += 1;
      break;
    case NORTH:
      pos.y -= 1;
      break;
    case SOUTH:
      pos.y += 1;
      break;
    case NONE:
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
  else
    { // Check if we the tile allows us to go to new_pos
      switch(direction)
        {
        case WEST:
          return (at(old_pos)->west && at(*new_pos)->east);

        case EAST:
          return (at(old_pos)->east && at(*new_pos)->west);

        case NORTH:
          return (at(old_pos)->north && at(*new_pos)->south);

        case SOUTH:
          return (at(old_pos)->south && at(*new_pos)->north);

        case NONE:
          assert(!"path_ok() can't work if direction is NONE");
        }
      return false;
    }
}

void
WorldMap::update()
{
  if (enter_level && !tux->is_moving())
    {
      Level* level = at_level();
      if (level)
        {
          if (level->x == tux->get_tile_pos().x && 
              level->y == tux->get_tile_pos().y)
            {
              std::cout << "Enter the current level: " << level->name << std::endl;;
              GameSession session(datadir +  "levels/" + level->name,
                                  1, ST_GL_LOAD_LEVEL_FILE);

              switch (session.run())
                {
                case GameSession::LEVEL_FINISHED:
                  level->solved = true;
                  break;
                case GameSession::LEVEL_ABORT:
                  // Reseting the player_status might be a worthy
                  // consideration, but I don't think we need it
                  // 'cause only the bad players will use it to
                  // 'cheat' a few items and that isn't necesarry a
                  // bad thing (ie. better they continue that way,
                  // then stop playing the game all together since it
                  // is to hard)
                  break;
                case GameSession::GAME_OVER:
                  quit = true;
                  break;
                case GameSession::NONE:
                  // Should never be reached 
                  break;
                }

              music_manager->play_music(song);
              Menu::set_current(0);
              if (!savegame_file.empty())
                savegame(savegame_file);
              return;
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
      tux->set_direction(input_direction);
      tux->update(0.33f);
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
            case MNID_SAVEGAME:
              if (!savegame_file.empty())
                savegame(savegame_file);
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

  return TileManager::instance()->get(tilemap[width * p.y + p.x]);
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
      if (i->solved)
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
  char str[80];
  sprintf(str, "%d", player_status.score);
  white_text->draw("SCORE", 0, 0);
  gold_text->draw(str, 96, 0);

  sprintf(str, "%d", player_status.distros);
  white_text->draw_align("COINS", 320-64, 0,  A_LEFT, A_TOP);
  gold_text->draw_align(str, 320+64, 0, A_RIGHT, A_TOP);

  white_text->draw("LIVES", 480, 0);
  if (player_status.lives >= 5)
    {
      sprintf(str, "%dx", player_status.lives);
      gold_text->draw(str, 585, 0);
      tux_life->draw(565+(18*3), 0);
    }
  else
    {
      for(int i= 0; i < player_status.lives; ++i)
        tux_life->draw(565+(18*i),0);
    }

  if (!tux->is_moving())
    {
      for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
        {
          if (i->x == tux->get_tile_pos().x && 
              i->y == tux->get_tile_pos().y)
            {
              white_text->draw_align(i->title.c_str(), screen->w/2, screen->h,  A_HMIDDLE, A_BOTTOM);
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

  song = music_manager->load_music(datadir +  "/music/" + music);
  music_manager->play_music(song);

  while(!quit) {
    Point tux_pos = tux->get_pos();
    if (1)
      {
        offset.x = -tux_pos.x + screen->w/2;
        offset.y = -tux_pos.y + screen->h/2;

        if (offset.x > 0) offset.x = 0;
        if (offset.y > 0) offset.y = 0;

        if (offset.x < screen->w - width*32) offset.x = screen->w - width*32;
        if (offset.y < screen->h - height*32) offset.y = screen->h - height*32;
      } 

    draw(offset);
    get_input();
    update();

  if(Menu::current())
    {
      Menu::current()->draw();
      mouse_cursor->draw();
    }
    flipscreen();

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
      << "  (tux     (x " << tux->get_tile_pos().x << ") (y " << tux->get_tile_pos().y << ")"
      << " (back \"" << direction_to_string(tux->back_direction) << "\"))\n"
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

  if (access(filename.c_str(), F_OK) == 0)
    {
      lisp_object_t* cur = lisp_read_from_file(filename);

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

          LispReader tux_reader(tux_cur);
          tux_reader.read_int("x", &p.x);
          tux_reader.read_int("y", &p.y);
          tux_reader.read_string("back", &back_str);
          
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
    }
}

} // namespace WorldMapNS

/* Local Variables: */
/* mode:c++ */
/* End: */

