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
#include "shrinkfade.hpp"
#include "video/surface.hpp"
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
#include "options_menu.hpp"
#include "scripting/squirrel_error.hpp"
#include "scripting/squirrel_util.hpp"
#include "worldmap/level.hpp"
#include "worldmap/special_tile.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/sprite_change.hpp"

namespace WorldMapNS {

enum WorldMapMenuIDs {
  MNID_RETURNWORLDMAP,
  MNID_QUITWORLDMAP
};

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
  else if (directory == "none")
    return D_NONE;
  else {
    log_warning << "unknown direction: \"" << directory << "\"" << std::endl;
    return D_NONE;
  }
}

//---------------------------------------------------------------------------

WorldMap::WorldMap(const std::string& filename, const std::string& force_spawnpoint)
  : tux(0), solids(0), force_spawnpoint(force_spawnpoint)
{
  tile_manager.reset(new TileManager("images/worldmap.strf"));

  tux = new Tux(this);
  add_object(tux);

  name = "<no title>";
  music = "music/salcon.ogg";

  total_stats.reset();

  worldmap_menu.reset(new Menu());
  worldmap_menu->add_label(_("Pause"));
  worldmap_menu->add_hl();
  worldmap_menu->add_entry(MNID_RETURNWORLDMAP, _("Continue"));
  worldmap_menu->add_submenu(_("Options"), get_options_menu());
  worldmap_menu->add_hl();
  worldmap_menu->add_entry(MNID_QUITWORLDMAP, _("Quit World"));

  load(filename);

  // create a new squirrel table for the worldmap
  using namespace Scripting;

  sq_collectgarbage(global_vm);
  sq_newtable(global_vm);
  sq_pushroottable(global_vm);
  if(SQ_FAILED(sq_setdelegate(global_vm, -2)))
    throw Scripting::SquirrelError(global_vm, "Couldn't set worldmap_table delegate");

  sq_resetobject(&worldmap_table);
  if(SQ_FAILED(sq_getstackobj(global_vm, -1, &worldmap_table)))
    throw Scripting::SquirrelError(global_vm, "Couldn't get table from stack");

  sq_addref(global_vm, &worldmap_table);
  sq_pop(global_vm, 1);
}

WorldMap::~WorldMap()
{
  using namespace Scripting;

  for(ScriptList::iterator i = scripts.begin();
      i != scripts.end(); ++i) {
    HSQOBJECT& object = *i;
    sq_release(global_vm, &object);
  }
  sq_release(global_vm, &worldmap_table);

  sq_collectgarbage(global_vm);

  if(current_ == this)
    current_ = NULL;

  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ++i) {
    GameObject* object = *i;
    object->unref();
  }

  for(SpawnPoints::iterator i = spawn_points.begin();
      i != spawn_points.end(); ++i) {
    delete *i;
  }
}

void
WorldMap::add_object(GameObject* object)
{
  TileMap* tilemap = dynamic_cast<TileMap*> (object);
  if(tilemap != 0 && tilemap->is_solid()) {
    solids = tilemap;
  }

  object->ref();
  game_objects.push_back(object);
}

void
WorldMap::move_to_spawnpoint(const std::string& spawnpoint)
{
  for(SpawnPoints::iterator i = spawn_points.begin(); i != spawn_points.end(); ++i) {
    SpawnPoint* sp = *i;
    if(sp->name == spawnpoint) {
      Vector p = sp->pos;
      tux->set_tile_pos(p);
      tux->set_direction(sp->auto_dir);
      return;
    }
  }
  log_warning << "Spawnpoint '" << spawnpoint << "' not found." << std::endl;
  if (spawnpoint != "main") {
    move_to_spawnpoint("main");
  }
}

void
WorldMap::change(const std::string& filename, const std::string& force_spawnpoint)
{
  main_loop->exit_screen();
  main_loop->push_screen(new WorldMap(filename, force_spawnpoint));
}

void
WorldMap::load(const std::string& filename)
{
  map_filename = filename;
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

    lisp::ListIterator iter(sector);
    while(iter.next()) {
      if(iter.item() == "tilemap") {
        add_object(new TileMap(*(iter.lisp()), tile_manager.get()));
      } else if(iter.item() == "background") {
        add_object(new Background(*(iter.lisp())));
      } else if(iter.item() == "music") {
        iter.value()->get(music);
      } else if(iter.item() == "init-script") {
        iter.value()->get(init_script);
      } else if(iter.item() == "worldmap-spawnpoint") {
        SpawnPoint* sp = new SpawnPoint(iter.lisp());
        spawn_points.push_back(sp);
      } else if(iter.item() == "level") {
        LevelTile* level = new LevelTile(levels_path, iter.lisp());
        levels.push_back(level);
        add_object(level);
      } else if(iter.item() == "special-tile") {
        SpecialTile* special_tile = new SpecialTile(iter.lisp());
        special_tiles.push_back(special_tile);
        add_object(special_tile);
      } else if(iter.item() == "sprite-change") {
        SpriteChange* sprite_change = new SpriteChange(iter.lisp());
        sprite_changes.push_back(sprite_change);
        add_object(sprite_change);
      } else if(iter.item() == "teleporter") {
        Teleporter* teleporter = new Teleporter(iter.lisp());
        teleporters.push_back(teleporter);
        add_object(teleporter);
      } else if(iter.item() == "name") {
        // skip
      } else {
        log_warning << "Unknown token '" << iter.item() << "' in worldmap" << std::endl;
      }
    }
    if(solids == 0)
      throw std::runtime_error("No solid tilemap specified");

    move_to_spawnpoint("main");

  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when parsing worldmap '" << map_filename << "': " <<
      e.what();
    throw std::runtime_error(msg.str());
  }
}

void
WorldMap::get_level_title(LevelTile& level)
{
  /** get special_tile's title */
  level.title = "<no title>";

  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (parser.parse(levels_path + level.get_name()));

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
  for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
    LevelTile* level = *i;
    if (level->solved) {
      total_stats += level->statistics;
    }
  }
}

void
WorldMap::on_escape_press()
{
  // Show or hide the menu
  if(!Menu::current()) {
    Menu::set_current(worldmap_menu.get());
    tux->set_direction(D_NONE);  // stop tux movement when menu is called
  } else {
    Menu::set_current(NULL);
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
WorldMap::path_ok(Direction direction, const Vector& old_pos, Vector* new_pos)
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
          assert(!"path_ok() can't walk if direction is NONE");
        }
      return false;
    }
}

void
WorldMap::finished_level(Level* gamelevel)
{
  // TODO use Level* parameter here?
  LevelTile* level = at_level();

  bool old_level_state = level->solved;
  level->solved = true;
  level->sprite->set_action("solved");

  // deal with statistics
  level->statistics.merge(gamelevel->stats);
  calculate_total_stats();

  save_state();
  if(World::current() != NULL)
    World::current()->save_state();

  if (old_level_state != level->solved && level->auto_path) {
    // Try to detect the next direction to which we should walk
    // FIXME: Mostly a hack
    Direction dir = D_NONE;

    const Tile* tile = at(tux->get_tile_pos());

    // first, test for crossroads
    if (tile->getData() & Tile::WORLDMAP_CNSE || tile->getData() && Tile::WORLDMAP_CNSW
     || tile->getData() & Tile::WORLDMAP_CNEW || tile->getData() && Tile::WORLDMAP_CSEW
     || tile->getData() & Tile::WORLDMAP_CNSEW)
      dir = D_NONE;
    else if (tile->getData() & Tile::WORLDMAP_NORTH
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
    try {
      std::istringstream in(level->extro_script);
      run_script(in, "worldmap:extro_script");
    } catch(std::exception& e) {
      log_fatal << "Couldn't run level-extro-script: " << e.what() << std::endl;
    }
  }
}

void
WorldMap::update(float delta)
{
  Menu* menu = Menu::current();
  if(menu != NULL) {
    menu->update();

    if(menu == worldmap_menu.get()) {
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
  for(size_t i = 0; i < game_objects.size(); ++i) {
    GameObject* object = game_objects[i];
    object->update(delta);
  }

  // remove old GameObjects
  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ) {
    GameObject* object = *i;
    if(!object->is_valid()) {
      object->unref();
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

  // check for teleporters
  Teleporter* teleporter = at_teleporter(tux->get_tile_pos());
  if (teleporter && (teleporter->automatic || (enter_level && (!tux->is_moving())))) {
    enter_level = false;
    if (teleporter->worldmap != "") {
      change(teleporter->worldmap, teleporter->spawnpoint);
    } else {
      // TODO: an animation, camera scrolling or a fading would be a nice touch
      sound_manager->play("sounds/warp.wav");
      tux->back_direction = D_NONE;
      move_to_spawnpoint(teleporter->spawnpoint);
    }
  }

  // check for auto-play levels
  LevelTile* level = at_level();
  if (level && (level->auto_play) && (!level->solved) && (!tux->is_moving())) {
    enter_level = true;
    level->solved = true;
  }

  if (enter_level && !tux->is_moving())
    {
      /* Check level action */
      LevelTile* level = at_level();
      if (!level) {
        //Respawn if player on a tile with no level and nowhere to go.
        const Tile* tile = at(tux->get_tile_pos());
        if(!( tile->getData() & ( Tile::WORLDMAP_NORTH |  Tile::WORLDMAP_SOUTH | Tile::WORLDMAP_WEST | Tile::WORLDMAP_EAST ))){
          log_warning << "Player at illegal position " << tux->get_tile_pos().x << ", " << tux->get_tile_pos().y << " respawning." << std::endl;
          move_to_spawnpoint("main");
          return;
        }
        log_warning << "No level to enter at: " << tux->get_tile_pos().x << ", " << tux->get_tile_pos().y << std::endl;
        return;
      }

      if (level->pos == tux->get_tile_pos()) {
        try {
          Vector shrinkpos = Vector(level->pos.x*32 + 16 - camera_offset.x,
                                    level->pos.y*32 + 16 - camera_offset.y);
          std::string levelfile = levels_path + level->get_name();
          main_loop->push_screen(new GameSession(levelfile, &level->statistics),
                                 new ShrinkFade(shrinkpos, 0.5));
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

LevelTile*
WorldMap::at_level()
{
  for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
    LevelTile* level = *i;
    if (level->pos == tux->get_tile_pos())
      return level;
  }

  return NULL;
}

SpecialTile*
WorldMap::at_special_tile()
{
  for(SpecialTiles::iterator i = special_tiles.begin();
      i != special_tiles.end(); ++i) {
    SpecialTile* special_tile = *i;
    if (special_tile->pos == tux->get_tile_pos())
      return special_tile;
  }

  return NULL;
}

SpriteChange*
WorldMap::at_sprite_change(const Vector& pos)
{
  for(SpriteChanges::iterator i = sprite_changes.begin();
      i != sprite_changes.end(); ++i) {
    SpriteChange* sprite_change = *i;
    if(sprite_change->pos == pos)
      return sprite_change;
  }

  return NULL;
}

Teleporter*
WorldMap::at_teleporter(const Vector& pos)
{
  for(std::vector<Teleporter*>::iterator i = teleporters.begin(); i != teleporters.end(); ++i) {
    Teleporter* teleporter = *i;
    if(teleporter->pos == pos) return teleporter;
  }

  return NULL;
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

  draw_status(context);
  context.pop_transform();
}

void
WorldMap::draw_status(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  player_status->draw(context);

  if (!tux->is_moving()) {
    for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
      LevelTile* level = *i;

      if (level->pos == tux->get_tile_pos()) {
        if(level->title == "")
          get_level_title(*level);

        context.draw_text(white_text, level->title,
            Vector(SCREEN_WIDTH/2,
              SCREEN_HEIGHT - white_text->get_height() - 30),
            CENTER_ALLIGN, LAYER_FOREGROUND1);

        // if level is solved, draw level picture behind stats
        /*
        if (level->solved) {
          if (const Surface* picture = level->get_picture()) {
            Vector pos = Vector(SCREEN_WIDTH - picture->get_width(), SCREEN_HEIGHT - picture->get_height());
            context.push_transform();
            context.set_alpha(0.5);
            context.draw_surface(picture, pos, LAYER_FOREGROUND1-1);
            context.pop_transform();
          }
        }
        */

        level->statistics.draw_worldmap_info(context);
        break;
      }
    }

    for(SpecialTiles::iterator i = special_tiles.begin();
        i != special_tiles.end(); ++i) {
      SpecialTile* special_tile = *i;

      if (special_tile->pos == tux->get_tile_pos()) {
        /* Display an in-map message in the map, if any as been selected */
        if(!special_tile->map_message.empty() && !special_tile->passive_message)
          context.draw_text(gold_text, special_tile->map_message,
              Vector(SCREEN_WIDTH/2,
                SCREEN_HEIGHT - white_text->get_height() - 60),
              CENTER_ALLIGN, LAYER_FOREGROUND1);
        break;
      }
    }

    // display teleporter messages
    Teleporter* teleporter = at_teleporter(tux->get_tile_pos());
    if (teleporter && (teleporter->message != "")) {
      Vector pos = Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT - white_text->get_height() - 30);
      context.draw_text(white_text, teleporter->message, pos, CENTER_ALLIGN, LAYER_FOREGROUND1);
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

  // if force_spawnpoint was set, move Tux there, then clear force_spawnpoint
  if (force_spawnpoint != "") {
    move_to_spawnpoint(force_spawnpoint);
    force_spawnpoint = "";
  }

  tux->setup();

  // register worldmap_table as worldmap in scripting
  using namespace Scripting;

  sq_pushroottable(global_vm);
  sq_pushstring(global_vm, "worldmap", -1);
  sq_pushobject(global_vm, worldmap_table);
  if(SQ_FAILED(sq_createslot(global_vm, -3)))
    throw SquirrelError(global_vm, "Couldn't set worldmap in roottable");
  sq_pop(global_vm, 1);

  if(init_script != "") {
    std::istringstream in(init_script);
    run_script(in, "WorldMap::init");
  }
}

void
WorldMap::leave()
{
  // remove worldmap_table from roottable
  using namespace Scripting;

  sq_pushroottable(global_vm);
  sq_pushstring(global_vm, "worldmap", -1);
  if(SQ_FAILED(sq_deleteslot(global_vm, -2, SQFalse)))
    throw SquirrelError(global_vm, "Couldn't unset worldmap in roottable");
  sq_pop(global_vm, 1);
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
  using namespace Scripting;

  HSQUIRRELVM vm = global_vm;
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

    for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
      LevelTile* level = *i;

      if (level->solved) {
        sq_pushstring(vm, level->get_name().c_str(), -1);
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
  using namespace Scripting;

  HSQUIRRELVM vm = global_vm;
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

    for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
      LevelTile* level = *i;
      sq_pushstring(vm, level->get_name().c_str(), -1);
      if(SQ_SUCCEEDED(sq_get(vm, -2))) {
        level->solved = read_bool(vm, "solved");
        level->sprite->set_action(level->solved ? "solved" : "default");
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
  for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
    LevelTile* level = *i;

    if(level->solved)
      count++;
  }

  return count;
}

HSQUIRRELVM
WorldMap::run_script(std::istream& in, const std::string& sourcename)
{
  using namespace Scripting;

  // garbage collect thread list
  for(ScriptList::iterator i = scripts.begin();
      i != scripts.end(); ) {
    HSQOBJECT& object = *i;
    HSQUIRRELVM vm = object_to_vm(object);

    if(sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
      sq_release(global_vm, &object);
      i = scripts.erase(i);
      continue;
    }

    ++i;
  }

  HSQOBJECT object = create_thread(global_vm);
  scripts.push_back(object);

  HSQUIRRELVM vm = object_to_vm(object);

  // set worldmap_table as roottable for the thread
  sq_pushobject(vm, worldmap_table);
  sq_setroottable(vm);

  compile_and_run(vm, in, sourcename);

  return vm;
}

} // namespace WorldMapNS
