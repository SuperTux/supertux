//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
#include <physfs.h>

#include "worldmap.hpp"

#include "gettext.hpp"
#include "log.hpp"
#include "mainloop.hpp"
#include "video/surface.hpp"
#include "video/screen.hpp"
#include "video/drawing_context.hpp"
#include "sprite/sprite_manager.hpp"
#include "audio/sound_manager.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/writer.hpp"
#include "game_session.hpp"
#include "sector.hpp"
#include "worldmap.hpp"
#include "resources.hpp"
#include "misc.hpp"
#include "log.hpp"
#include "world.hpp"
#include "player_status.hpp"
#include "textscroller.hpp"
#include "main.hpp"
#include "spawn_point.hpp"
#include "file_system.hpp"
#include "gui/menu.hpp"
#include "gui/mousecursor.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "object/background.hpp"
#include "object/tilemap.hpp"
#include "scripting/squirrel_error.hpp"
#include "scripting/wrapper_util.hpp"

Menu* worldmap_menu  = 0;

static const float TUXSPEED = 200;
static const float map_message_TIME = 2.8;

namespace WorldMapNS {

WorldMap* WorldMap::current_ = NULL;

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
  tux_sprite = sprite_manager->create("images/worldmap/common/tux.sprite");
  
  offset = 0;
  moving = false;
  direction = D_NONE;
  input_direction = D_NONE;
}

Tux::~Tux()
{
  delete tux_sprite;
}

void
Tux::draw(DrawingContext& context)
{
  switch (player_status->bonus) {
    case GROWUP_BONUS:
      tux_sprite->set_action(moving ? "large-walking" : "large-stop");
      break;
    case FIRE_BONUS:
      tux_sprite->set_action(moving ? "fire-walking" : "fire-stop");
      break;
    case NO_BONUS:
      tux_sprite->set_action(moving ? "small-walking" : "small-stop");
      break;
    default:
      log_debug << "Bonus type not handled in worldmap." << std::endl;
      tux_sprite->set_action("large-stop");
      break;
  }

  tux_sprite->draw(context, get_pos(), LAYER_OBJECTS);
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
Tux::tryStartWalking() 
{
  if (moving) return;
  if (input_direction == D_NONE) return;

  WorldMap::Level* level = worldmap->at_level();

  // We got a new direction, so lets start walking when possible
  Vector next_tile;
  if ((!level || level->solved) && worldmap->path_ok(input_direction, tile_pos, &next_tile))
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

bool 
Tux::canWalk(const Tile* tile, Direction dir)
{
  return ((tile->getData() & Tile::WORLDMAP_NORTH && dir == D_NORTH) ||
	  (tile->getData() & Tile::WORLDMAP_SOUTH && dir == D_SOUTH) ||
	  (tile->getData() & Tile::WORLDMAP_EAST && dir == D_EAST) ||
	  (tile->getData() & Tile::WORLDMAP_WEST && dir == D_WEST));
}

void 
Tux::tryContinueWalking(float elapsed_time)
{
  if (!moving) return;

  // Let tux walk
  offset += TUXSPEED * elapsed_time;

  // Do nothing if we have not yet reached the next tile
  if (offset <= 32) return;

  offset -= 32;

  // if this is a special_tile with passive_message, display it
  WorldMap::SpecialTile* special_tile = worldmap->at_special_tile();
  if(special_tile && special_tile->passive_message)
  {  
    // direction and the apply_action_ are opposites, since they "see"
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

  // stop if we reached a level, a WORLDMAP_STOP tile or a special tile without a passive_message
  if ((worldmap->at_level()) || (worldmap->at(tile_pos)->getData() & Tile::WORLDMAP_STOP) || (special_tile && !special_tile->passive_message))
  {
    if(special_tile && !special_tile->map_message.empty() && !special_tile->passive_message) worldmap->passive_message_timer.start(0);
    stop();
    return;
  }

  // if user wants to change direction, try changing, else guess the direction in which to walk next
  const Tile* tile = worldmap->at(tile_pos);
  if (direction != input_direction)
  { 
    if(canWalk(tile, input_direction))
    {  
      direction = input_direction;
      back_direction = reverse_dir(direction);
    }
  }
  else
  {
    Direction dir = D_NONE;
    if (tile->getData() & Tile::WORLDMAP_NORTH && back_direction != D_NORTH) dir = D_NORTH;
    else if (tile->getData() & Tile::WORLDMAP_SOUTH && back_direction != D_SOUTH) dir = D_SOUTH;
    else if (tile->getData() & Tile::WORLDMAP_EAST && back_direction != D_EAST) dir = D_EAST;
    else if (tile->getData() & Tile::WORLDMAP_WEST && back_direction != D_WEST) dir = D_WEST;

    if (dir == D_NONE) 
    {
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
  if(direction != D_NONE)
  {
    Vector next_tile;
    if (worldmap->path_ok(direction, tile_pos, &next_tile))
    {
      tile_pos = next_tile;
    }
    else
    {
      log_warning << "Tilemap data is buggy" << std::endl;
      stop();
    }
  }
}

void
Tux::updateInputDirection()
{
  if(main_controller->hold(Controller::UP)) input_direction = D_NORTH;
  else if(main_controller->hold(Controller::DOWN)) input_direction = D_SOUTH;
  else if(main_controller->hold(Controller::LEFT)) input_direction = D_WEST;
  else if(main_controller->hold(Controller::RIGHT)) input_direction = D_EAST;
}


void
Tux::update(float elapsed_time)
{
  updateInputDirection(); 
  if (moving) tryContinueWalking(elapsed_time); else tryStartWalking();
}

//---------------------------------------------------------------------------

WorldMap::WorldMap()
  : tux(0), solids(0)
{
  tile_manager = new TileManager("images/worldmap.strf");
  
  tux = new Tux(this);
  add_object(tux);
    
  messagedot = new Surface("images/worldmap/common/messagedot.png");
  teleporterdot = sprite_manager->create("images/worldmap/common/teleporter.sprite");

  name = "<no title>";
  music = "music/salcon.ogg";
  intro_displayed = false;

  total_stats.reset();
}

WorldMap::~WorldMap()
{
  if(current_ == this)
    current_ = NULL;

  clear_objects();
  for(SpawnPoints::iterator i = spawn_points.begin();
      i != spawn_points.end(); ++i) {
    delete *i;
  }
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i) {
    Level& level = *i;
    delete level.sprite;
  }
  for(SpecialTiles::iterator i = special_tiles.begin(); i != special_tiles.end(); ++i) {
    delete i->sprite;
  }
  
  delete tile_manager;

  delete messagedot;
  delete teleporterdot;
}

void
WorldMap::add_object(GameObject* object)
{
  TileMap* tilemap = dynamic_cast<TileMap*> (object);
  if(tilemap != 0 && tilemap->is_solid()) {
    solids = tilemap;
  }

  game_objects.push_back(object);
}

void
WorldMap::clear_objects()
{
  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ++i)
    delete *i;
  game_objects.clear();
  solids = 0;
  tux = new Tux(this);
  add_object(tux);
}

// Don't forget to set map_filename before calling this
void
WorldMap::load_map()
{
  levels_path = FileSystem::dirname(map_filename);

  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (parser.parse(map_filename));

    const lisp::Lisp* lisp = root->get_lisp("supertux-level");
    if(!lisp)
      throw std::runtime_error("file isn't a supertux-level file.");

    lisp->get("name", name);
    
    const lisp::Lisp* sector = lisp->get_lisp("sector");
    if(!sector)
      throw std::runtime_error("No sector sepcified in worldmap file.");
    
    clear_objects();
    lisp::ListIterator iter(sector);
    while(iter.next()) {
      if(iter.item() == "tilemap") {
        add_object(new TileMap(*(iter.lisp()), tile_manager));
      } else if(iter.item() == "background") {
        add_object(new Background(*(iter.lisp())));
      } else if(iter.item() == "music") {
        iter.value()->get(music);
      } else if(iter.item() == "intro-script") {
        iter.value()->get(intro_script);
      } else if(iter.item() == "worldmap-spawnpoint") {
        SpawnPoint* sp = new SpawnPoint(iter.lisp());
        spawn_points.push_back(sp);
      } else if(iter.item() == "level") {
        parse_level_tile(iter.lisp());
      } else if(iter.item() == "special-tile") {
        parse_special_tile(iter.lisp());
      } else if(iter.item() == "name") {
        // skip
      } else {
        log_warning << "Unknown token '" << iter.item() << "' in worldmap" << std::endl;
      }
    }
    if(solids == 0)
      throw std::runtime_error("No solid tilemap specified");

    // search for main spawnpoint
    for(SpawnPoints::iterator i = spawn_points.begin();
        i != spawn_points.end(); ++i) {
      SpawnPoint* sp = *i;
      if(sp->name == "main") {
        Vector p = sp->pos;
        tux->set_tile_pos(p);
        break;
      }
    }

  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when parsing worldmap '" << map_filename << "': " <<
      e.what();
    throw std::runtime_error(msg.str());
  }
}

void
WorldMap::parse_special_tile(const lisp::Lisp* lisp)
{
  SpecialTile special_tile;
  
  lisp->get("x", special_tile.pos.x);
  lisp->get("y", special_tile.pos.y);

  std::string sprite;
  if (lisp->get("sprite", sprite)) {
    special_tile.sprite = sprite_manager->create(sprite);
  } else {
    special_tile.sprite = 0;
  }

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
}

void
WorldMap::parse_level_tile(const lisp::Lisp* level_lisp)
{
  Level level;

  level.solved = false;
                  
  level.north = true;
  level.east  = true;
  level.south = true;
  level.west  = true;

  std::string sprite = "images/worldmap/common/leveldot.sprite";
  level_lisp->get("sprite", sprite);
  level.sprite = sprite_manager->create(sprite);

  level_lisp->get("extro-script", level.extro_script);
  level_lisp->get("next-worldmap", level.next_worldmap);

  level.quit_worldmap = false;
  level_lisp->get("quit-worldmap", level.quit_worldmap);

  level_lisp->get("name", level.name);
  
  if (!PHYSFS_exists((levels_path + level.name).c_str()))
  {
  	// Do we want to bail out instead...? We might get messages from modders
  	// who can't make their levels run because they're too dumb to watch
  	// their terminals...
    log_warning << "level file '" << level.name << "' does not exist and will not be added to the worldmap" << std::endl;
    return;
  }

  level_lisp->get("x", level.pos.x);
  level_lisp->get("y", level.pos.y);

  level.auto_path = true;
  level_lisp->get("auto-path", level.auto_path);

  level.vertical_flip = false;
  level_lisp->get("vertical-flip", level.vertical_flip);

  levels.push_back(level);
}

void
WorldMap::get_level_title(Level& level)
{
  /** get special_tile's title */
  level.title = "<no title>";

  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (parser.parse(levels_path + level.name));

    const lisp::Lisp* level_lisp = root->get_lisp("supertux-level");
    if(!level_lisp)
      return;
    
    level_lisp->get("name", level.title);
  } catch(std::exception& e) {
    log_warning << "Problem when reading leveltitle: " << e.what() << std::endl;
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
  if(!Menu::current()) {
    Menu::set_current(worldmap_menu);
    tux->set_direction(D_NONE);  // stop tux movement when menu is called
  } else {
    Menu::set_current(0);
  }
}

Vector
WorldMap::get_next_tile(Vector pos, Direction direction)
{
  switch(direction) {
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

  if (!(new_pos->x >= 0 && new_pos->x < solids->get_width()
        && new_pos->y >= 0 && new_pos->y < solids->get_height()))
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
WorldMap::finished_level(const std::string& filename)
{
  // TODO calculate level from filename?
  (void) filename;
  Level* level = at_level();

  bool old_level_state = level->solved;
  level->solved = true;
  level->sprite->set_action("solved");

  // deal with statistics
  level->statistics.merge(global_stats);
  calculate_total_stats();

  save_state();
  if(World::current() != NULL)
    World::current()->save_state();

  if (old_level_state != level->solved && level->auto_path) {
    // Try to detect the next direction to which we should walk
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

    if (dir != D_NONE) {
      tux->set_direction(dir);
    }
  }

  if (level->extro_script != "") {
    /* TODO
    try {
      std::auto_ptr<ScriptInterpreter> interpreter
        (new ScriptInterpreter(levels_path));
      std::istringstream in(level->extro_script);
      interpreter->run_script(in, "level-extro-script");
      add_object(interpreter.release());
    } catch(std::exception& e) {
      log_fatal << "Couldn't run level-extro-script:" << e.what() << std::endl;
    }
    */
  }
  
  if (!level->next_worldmap.empty()) {
    // Load given worldmap
    loadmap(level->next_worldmap);
  }
  
  if (level->quit_worldmap)
    main_loop->exit_screen();
}

void
WorldMap::update(float delta)
{
  Menu* menu = Menu::current();
  if(menu) {
    menu->update();

    if(menu == worldmap_menu) {
      switch (worldmap_menu->check())
      {
        case MNID_RETURNWORLDMAP: // Return to game  
          Menu::set_current(0);
          break;
        case MNID_QUITWORLDMAP: // Quit Worldmap
          main_loop->exit_screen();
          break;
      }
    }

    return;
  }

  // update GameObjects
  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ++i) {
    GameObject* object = *i;
    object->update(delta);
  }

  // remove old GameObjects
  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ) {
    GameObject* object = *i;
    if(!object->is_valid()) {
      delete object;
      i = game_objects.erase(i);
    } else {
      ++i;
    }
  }

  // position "camera"
  Vector tux_pos = tux->get_pos();
  camera_offset.x = tux_pos.x - SCREEN_WIDTH/2;
  camera_offset.y = tux_pos.y - SCREEN_HEIGHT/2;

  if (camera_offset.x < 0)
    camera_offset.x = 0;
  if (camera_offset.y < 0)
    camera_offset.y = 0;

  if (camera_offset.x > solids->get_width()*32 - SCREEN_WIDTH)
    camera_offset.x = solids->get_width()*32 - SCREEN_WIDTH;
  if (camera_offset.y > solids->get_height()*32 - SCREEN_HEIGHT)
    camera_offset.y = solids->get_height()*32 - SCREEN_HEIGHT;

  // handle input
  bool enter_level = false;
  if(main_controller->pressed(Controller::ACTION)
      || main_controller->pressed(Controller::JUMP)
      || main_controller->pressed(Controller::MENU_SELECT))
    enter_level = true;
  if(main_controller->pressed(Controller::PAUSE_MENU))
    on_escape_press();
  
  if (enter_level && !tux->is_moving())
    {
      /* Check special tile action */
      SpecialTile* special_tile = at_special_tile();
      if(special_tile)
        {
        if (special_tile->teleport_dest != Vector(-1,-1))
          {
          // TODO: an animation, camera scrolling or a fading would be a nice touch
          sound_manager->play("sounds/warp.wav");
          tux->back_direction = D_NONE;
          tux->set_tile_pos(special_tile->teleport_dest);
          SDL_Delay(1000);
          }
        }

      /* Check level action */
      Level* level = at_level();
      if (!level) {
        log_warning << "No level to enter at: " << tux->get_tile_pos().x << ", " << tux->get_tile_pos().y << std::endl;
        return;
      }

      if (level->pos == tux->get_tile_pos()) {
        // do a shriking fade to the level
        shrink_fade(Vector((level->pos.x*32 + 16 + offset.x),
                           (level->pos.y*32 + 16 + offset.y)), 500);

        try {
          GameSession *session =
            new GameSession(levels_path + level->name,
                ST_GL_LOAD_LEVEL_FILE, &level->statistics);
          main_loop->push_screen(session);
        } catch(std::exception& e) {
          log_fatal << "Couldn't load level: " << e.what() << std::endl;
        }
      }
    }
  else
    {
//      tux->set_direction(input_direction);
    }
}

const Tile*
WorldMap::at(Vector p)
{
  return solids->get_tile((int) p.x, (int) p.y);
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
WorldMap::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(camera_offset);
  
  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ++i) {
    GameObject* object = *i;
    object->draw(context);
  }
  
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      const Level& level = *i;
      level.sprite->draw(context, level.pos*32 + Vector(16, 16), LAYER_TILES+1);
    }

  for(SpecialTiles::iterator i = special_tiles.begin(); i != special_tiles.end(); ++i)
    {
      if(i->invisible)
        continue;

      if (i->sprite)
	i->sprite->draw(context, i->pos*32 + Vector(16, 16), LAYER_TILES+1);

      else if (i->teleport_dest != Vector(-1, -1))
	teleporterdot->draw(context, i->pos*32 + Vector(16, 16), LAYER_TILES+1);

      else if (!i->map_message.empty() && !i->passive_message)
        context.draw_surface(messagedot,
                Vector(i->pos.x*32, i->pos.y*32), LAYER_TILES+1);
    }

  draw_status(context);
  context.pop_transform();
}

void
WorldMap::draw_status(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));
 
  player_status->draw(context);

  if (!tux->is_moving())
    {
      for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
        {
          if (i->pos == tux->get_tile_pos())
            {
              if(i->title == "")
                get_level_title(*i);

              context.draw_text(white_text, i->title, 
                  Vector(SCREEN_WIDTH/2,
                         SCREEN_HEIGHT - white_text->get_height() - 30),
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
                    Vector(SCREEN_WIDTH/2,
                           SCREEN_HEIGHT - white_text->get_height() - 60),
                    CENTER_ALLIGN, LAYER_FOREGROUND1);
              break;
            }
        }
    }
  /* Display a passive message in the map, if needed */
  if(passive_message_timer.started())
    context.draw_text(gold_text, passive_message, 
            Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT - white_text->get_height() - 60),
            CENTER_ALLIGN, LAYER_FOREGROUND1);

  context.pop_transform();
}

void
WorldMap::setup()
{
  sound_manager->play_music(music);
  Menu::set_current(NULL);

  current_ = this;
  load_state();
}

static void store_float(HSQUIRRELVM vm, const char* name, float val)
{
  sq_pushstring(vm, name, -1);
  sq_pushfloat(vm, val);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw Scripting::SquirrelError(vm, "Couldn't add float value to table");
}

/*
static void store_int(HSQUIRRELVM vm, const char* name, int val)
{
  sq_pushstring(vm, name, -1);
  sq_pushinteger(vm, val);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw Scripting::SquirrelError(vm, "Couldn't add float value to table");
}
*/

static void store_string(HSQUIRRELVM vm, const char* name, const std::string& val)
{
  sq_pushstring(vm, name, -1);
  sq_pushstring(vm, val.c_str(), val.length());
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw Scripting::SquirrelError(vm, "Couldn't add float value to table");
}

static void store_bool(HSQUIRRELVM vm, const char* name, bool val)
{
  sq_pushstring(vm, name, -1);
  sq_pushbool(vm, val ? SQTrue : SQFalse);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw Scripting::SquirrelError(vm, "Couldn't add float value to table");
}

static float read_float(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if(SQ_FAILED(sq_get(vm, -2))) {
    std::ostringstream msg;
    msg << "Couldn't get float value for '" << name << "' from table";
    throw Scripting::SquirrelError(vm, msg.str());
  }
  
  float result;
  if(SQ_FAILED(sq_getfloat(vm, -1, &result))) {
    std::ostringstream msg;
    msg << "Couldn't get float value for '" << name << "' from table";
    throw Scripting::SquirrelError(vm, msg.str());
  }
  sq_pop(vm, 1);

  return result;
}

static std::string read_string(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if(SQ_FAILED(sq_get(vm, -2))) {
    std::ostringstream msg;
    msg << "Couldn't get string value for '" << name << "' from table";
    throw Scripting::SquirrelError(vm, msg.str());
  }
  
  const char* result;
  if(SQ_FAILED(sq_getstring(vm, -1, &result))) {
    std::ostringstream msg;
    msg << "Couldn't get string value for '" << name << "' from table";
    throw Scripting::SquirrelError(vm, msg.str());
  }
  sq_pop(vm, 1);

  return std::string(result);
}

static bool read_bool(HSQUIRRELVM vm, const char* name)
{
  sq_pushstring(vm, name, -1);
  if(SQ_FAILED(sq_get(vm, -2))) {
    std::ostringstream msg;
    msg << "Couldn't get bool value for '" << name << "' from table";
    throw Scripting::SquirrelError(vm, msg.str());
  } 
  
  SQBool result;
  if(SQ_FAILED(sq_getbool(vm, -1, &result))) {
    std::ostringstream msg;
    msg << "Couldn't get bool value for '" << name << "' from table";
    throw Scripting::SquirrelError(vm, msg.str());
  }
  sq_pop(vm, 1);

  return result == SQTrue;
}

void
WorldMap::save_state()
{
  HSQUIRRELVM vm = ScriptManager::instance->get_vm();
  int oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    sq_pushstring(vm, "state", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw Scripting::SquirrelError(vm, "Couldn't get state table");

    // get or create worlds table
    sq_pushstring(vm, "worlds", -1);
    if(SQ_FAILED(sq_get(vm, -2))) {
      sq_pushstring(vm, "worlds", -1);
      sq_newtable(vm);
      if(SQ_FAILED(sq_createslot(vm, -3)))
        throw Scripting::SquirrelError(vm, "Couldn't create state.worlds");

      sq_pushstring(vm, "worlds", -1);
      if(SQ_FAILED(sq_get(vm, -2)))
        throw Scripting::SquirrelError(vm, "Couldn't create.get state.worlds");
    }
    
    sq_pushstring(vm, map_filename.c_str(), map_filename.length());
    if(SQ_FAILED(sq_deleteslot(vm, -2, SQFalse)))
      sq_pop(vm, 1);

    // construct new table for this worldmap
    sq_pushstring(vm, map_filename.c_str(), map_filename.length());
    sq_newtable(vm);

    // store tux
    sq_pushstring(vm, "tux", -1);
    sq_newtable(vm);
    
    store_float(vm, "x", tux->get_tile_pos().x);
    store_float(vm, "y", tux->get_tile_pos().y);
    store_string(vm, "back", direction_to_string(tux->back_direction));

    sq_createslot(vm, -3);
    
    // levels...
    sq_pushstring(vm, "levels", -1);
    sq_newtable(vm);

    for(Levels::iterator i = levels.begin(); i != levels.end(); ++i) {
      if (i->solved) {
        sq_pushstring(vm, i->name.c_str(), -1);
        sq_newtable(vm);

        store_bool(vm, "solved", true);            
        // TODO write statistics
        // i->statistics.write(writer);

        sq_createslot(vm, -3);
      }
    }
    
    sq_createslot(vm, -3);

    // push world into worlds table
    sq_createslot(vm, -3);
  } catch(std::exception& e) {
    sq_settop(vm, oldtop);
  }

  sq_settop(vm, oldtop);
}

void
WorldMap::load_state()
{
  HSQUIRRELVM vm = ScriptManager::instance->get_vm();
  int oldtop = sq_gettop(vm);
 
  try {
    // get state table
    sq_pushroottable(vm);
    sq_pushstring(vm, "state", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw Scripting::SquirrelError(vm, "Couldn't get state table");

    // get worlds table
    sq_pushstring(vm, "worlds", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw Scripting::SquirrelError(vm, "Couldn't get state.worlds");

    // get table for our world
    sq_pushstring(vm, map_filename.c_str(), map_filename.length());
    if(SQ_FAILED(sq_get(vm, -2)))
      throw Scripting::SquirrelError(vm, "Couldn't get state.world.mapfilename");

    // load tux
    sq_pushstring(vm, "tux", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw Scripting::SquirrelError(vm, "Couldn't get tux");

    Vector p;
    p.x = read_float(vm, "x");
    p.y = read_float(vm, "y");
    std::string back_str = read_string(vm, "back");
    tux->back_direction = string_to_direction(back_str);
    tux->set_tile_pos(p);

    sq_pop(vm, 1);

    // load levels
    sq_pushstring(vm, "levels", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw Scripting::SquirrelError(vm, "Couldn't get levels");

    for(Levels::iterator i = levels.begin(); i != levels.end(); ++i) {
      sq_pushstring(vm, i->name.c_str(), -1);
      if(SQ_SUCCEEDED(sq_get(vm, -2))) {
        i->solved = read_bool(vm, "solved");
        i->sprite->set_action(i->solved ? "solved" : "default");
        // i->statistics.parse(*level);
        sq_pop(vm, 1);
      }
    }
    sq_pop(vm, 1);

  } catch(std::exception& e) {
    log_debug << "Not loading worldmap state: " << e.what() << std::endl;
  }
  sq_settop(vm, oldtop);
}

size_t
WorldMap::level_count()
{
  return levels.size();
}

size_t
WorldMap::solved_level_count()
{
  size_t count = 0;
  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i) {
    if(i->solved)
      count++;
  }

  return count;
}
    
void
WorldMap::loadmap(const std::string& filename)
{
  savegame_file = "";
  map_filename = filename;
  load_map();
}

} // namespace WorldMapNS
