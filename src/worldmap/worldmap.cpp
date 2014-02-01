//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include "worldmap/worldmap.hpp"

#include <config.h>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <vector>

#include "audio/sound_manager.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/parser.hpp"
#include "object/background.hpp"
#include "object/decal.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "scripting/squirrel_error.hpp"
#include "scripting/squirrel_util.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/menu/worldmap_menu.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "supertux/shrinkfade.hpp"
#include "supertux/spawn_point.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "worldmap/level.hpp"
#include "worldmap/special_tile.hpp"
#include "worldmap/sprite_change.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"

static const float CAMERA_PAN_SPEED = 5.0;

namespace worldmap {

WorldMap* WorldMap::current_ = NULL;

WorldMap::WorldMap(const std::string& filename, PlayerStatus* player_status, const std::string& force_spawnpoint) :
  tux(0),
  player_status(player_status),
  tileset(NULL), 
  free_tileset(false),
  worldmap_menu(),
  camera_offset(),
  name(),
  music(),
  init_script(),
  game_objects(),
  solid_tilemaps(),
  passive_message_timer(),
  passive_message(),
  map_filename(),
  levels_path(),
  special_tiles(),
  levels(),
  sprite_changes(),
  spawn_points(),
  teleporters(),
  total_stats(),
  worldmap_table(),
  scripts(),
  ambient_light( 1.0f, 1.0f, 1.0f, 1.0f ), 
  force_spawnpoint(force_spawnpoint),
  in_level(false), 
  pan_pos(),
  panning(false)
{
  tux = new Tux(this);
  add_object(tux);

  name = "<no title>";
  music = "music/salcon.ogg";

  total_stats.reset();

  worldmap_menu.reset(new WorldmapMenu());

  // create a new squirrel table for the worldmap
  using namespace scripting;

  sq_collectgarbage(global_vm);
  sq_newtable(global_vm);
  sq_pushroottable(global_vm);
  if(SQ_FAILED(sq_setdelegate(global_vm, -2)))
    throw scripting::SquirrelError(global_vm, "Couldn't set worldmap_table delegate");

  sq_resetobject(&worldmap_table);
  if(SQ_FAILED(sq_getstackobj(global_vm, -1, &worldmap_table)))
    throw scripting::SquirrelError(global_vm, "Couldn't get table from stack");

  sq_addref(global_vm, &worldmap_table);
  sq_pop(global_vm, 1);

  sound_manager->preload("sounds/warp.wav");
  
  // load worldmap objects
  load(filename);
}

WorldMap::~WorldMap()
{
  using namespace scripting;

  if(free_tileset)
    delete tileset;

  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ++i) {
    GameObject* object = *i;
    try_unexpose(object);
    object->unref();
  }

  for(SpawnPoints::iterator i = spawn_points.begin();
      i != spawn_points.end(); ++i) {
    delete *i;
  }

  for(ScriptList::iterator i = scripts.begin();
      i != scripts.end(); ++i) {
    HSQOBJECT& object = *i;
    sq_release(global_vm, &object);
  }
  sq_release(global_vm, &worldmap_table);

  sq_collectgarbage(global_vm);

  if(current_ == this)
    current_ = NULL;
}

void
WorldMap::add_object(GameObject* object)
{
  TileMap* tilemap = dynamic_cast<TileMap*> (object);
  if(tilemap != 0 && tilemap->is_solid()) {
    solid_tilemaps.push_back(tilemap);
  }

  object->ref();
  try_expose(object);
  game_objects.push_back(object);
}

void
WorldMap::try_expose(GameObject* object)
{
  ScriptInterface* object_ = dynamic_cast<ScriptInterface*> (object);
  if(object_ != NULL) {
    HSQUIRRELVM vm = scripting::global_vm;
    sq_pushobject(vm, worldmap_table);
    object_->expose(vm, -1);
    sq_pop(vm, 1);
  }
}

void
WorldMap::try_unexpose(GameObject* object)
{
  ScriptInterface* object_ = dynamic_cast<ScriptInterface*> (object);
  if(object_ != NULL) {
    HSQUIRRELVM vm = scripting::global_vm;
    SQInteger oldtop = sq_gettop(vm);
    sq_pushobject(vm, worldmap_table);
    try {
      object_->unexpose(vm, -1);
    } catch(std::exception& e) {
      log_warning << "Couldn't unregister object: " << e.what() << std::endl;
    }
    sq_settop(vm, oldtop);
  }
}

void
WorldMap::move_to_spawnpoint(const std::string& spawnpoint, bool pan)
{
  for(SpawnPoints::iterator i = spawn_points.begin(); i != spawn_points.end(); ++i) {
    SpawnPoint* sp = *i;
    if(sp->name == spawnpoint) {
      Vector p = sp->pos;
      tux->set_tile_pos(p);
      tux->set_direction(sp->auto_dir);
      if(pan) {
        panning = true;
        pan_pos = get_camera_pos_for_tux();
        clamp_camera_position(pan_pos);
      }
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
  g_screen_manager->exit_screen();
  g_screen_manager->push_screen(new WorldMap(filename, player_status, force_spawnpoint));
}

void
WorldMap::load(const std::string& filename)
{
  map_filename = filename;
  levels_path = FileSystem::dirname(map_filename);

  try {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(map_filename);

    const lisp::Lisp* level = root->get_lisp("supertux-level");
    if(level == NULL)
      throw std::runtime_error("file isn't a supertux-level file.");

    level->get("name", name);

    const lisp::Lisp* sector = level->get_lisp("sector");
    if(!sector)
      throw std::runtime_error("No sector specified in worldmap file.");

    const lisp::Lisp* tilesets_lisp = level->get_lisp("tilesets");
    if(tilesets_lisp != NULL) {
      tileset      = tile_manager->parse_tileset_definition(*tilesets_lisp);
      free_tileset = true;
    }
    std::string tileset_name;
    if(level->get("tileset", tileset_name)) {
      if(tileset != NULL) {
        log_warning << "multiple tilesets specified in level" << std::endl;
      } else {
        tileset = tile_manager->get_tileset(tileset_name);
      }
    }
    /* load default tileset */
    if(tileset == NULL) {
      tileset = tile_manager->get_tileset("images/worldmap.strf");
    }
    current_tileset = tileset;

    lisp::ListIterator iter(sector);
    while(iter.next()) {
      if(iter.item() == "tilemap") {
        add_object(new TileMap(*(iter.lisp())));
      } else if(iter.item() == "background") {
        add_object(new Background(*(iter.lisp())));
      } else if(iter.item() == "music") {
        iter.value()->get(music);
      } else if(iter.item() == "init-script") {
        iter.value()->get(init_script);
      } else if(iter.item() == "worldmap-spawnpoint") {
        SpawnPoint* sp = new SpawnPoint(*iter.lisp());
        spawn_points.push_back(sp);
      } else if(iter.item() == "level") {
        LevelTile* level = new LevelTile(levels_path, *iter.lisp());
        levels.push_back(level);
        add_object(level);
      } else if(iter.item() == "special-tile") {
        SpecialTile* special_tile = new SpecialTile(*iter.lisp());
        special_tiles.push_back(special_tile);
        add_object(special_tile);
      } else if(iter.item() == "sprite-change") {
        SpriteChange* sprite_change = new SpriteChange(*iter.lisp());
        sprite_changes.push_back(sprite_change);
        add_object(sprite_change);
      } else if(iter.item() == "teleporter") {
        Teleporter* teleporter = new Teleporter(*iter.lisp());
        teleporters.push_back(teleporter);
        add_object(teleporter);
      } else if(iter.item() == "decal") {
        Decal* decal = new Decal(*iter.lisp());
        add_object(decal);
      } else if(iter.item() == "ambient-light") {
        std::vector<float> vColor;
        sector->get( "ambient-light", vColor );
        if(vColor.size() < 3) {
          log_warning << "(ambient-light) requires a color as argument" << std::endl;
        } else {
          ambient_light = Color( vColor );
        }
      } else if(iter.item() == "name") {
        // skip
      } else {
        log_warning << "Unknown token '" << iter.item() << "' in worldmap" << std::endl;
      }
    }
    current_tileset = NULL;

    if(solid_tilemaps.size() == 0)
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
    const lisp::Lisp* root = parser.parse(levels_path + level.get_name());

    const lisp::Lisp* level_lisp = root->get_lisp("supertux-level");
    if(!level_lisp)
      return;

    level_lisp->get("name", level.title);
  } catch(std::exception& e) {
    log_warning << "Problem when reading leveltitle: " << e.what() << std::endl;
    return;
  }
}

void
WorldMap::get_level_target_time(LevelTile& level)
{
  if(last_position == tux->get_tile_pos()) {
    level.target_time = last_target_time;
    return;
  }
  
  try {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(levels_path + level.get_name());

    const lisp::Lisp* level_lisp = root->get_lisp("supertux-level");
    if(!level_lisp)
      return;

    level_lisp->get("target-time", level.target_time);

    last_position = level.pos;
    last_target_time = level.target_time;
  } catch(std::exception& e) {
    log_warning << "Problem when reading level target time: " << e.what() << std::endl;
    return;
  }
}

void WorldMap::calculate_total_stats()
{
  total_stats.zero();
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
  if(!MenuManager::current()) {
    MenuManager::set_current(worldmap_menu.get());
    tux->set_direction(D_NONE);  // stop tux movement when menu is called
  } else {
    MenuManager::set_current(NULL);
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

  if (!(new_pos->x >= 0 && new_pos->x < get_width()
        && new_pos->y >= 0 && new_pos->y < get_height()))
  { // New position is outsite the tilemap
    return false;
  }
  else
  { // Check if the tile allows us to go to new_pos
    int old_tile_data = tile_data_at(old_pos);
    int new_tile_data = tile_data_at(*new_pos);
    switch(direction)
    {
      case D_WEST:
        return (old_tile_data & Tile::WORLDMAP_WEST
                && new_tile_data & Tile::WORLDMAP_EAST);

      case D_EAST:
        return (old_tile_data & Tile::WORLDMAP_EAST
                && new_tile_data & Tile::WORLDMAP_WEST);

      case D_NORTH:
        return (old_tile_data & Tile::WORLDMAP_NORTH
                && new_tile_data & Tile::WORLDMAP_SOUTH);

      case D_SOUTH:
        return (old_tile_data & Tile::WORLDMAP_SOUTH
                && new_tile_data & Tile::WORLDMAP_NORTH);

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
  get_level_target_time(*level);
  if(level->statistics.completed(level->statistics, level->target_time)) {
    level->perfect = true;
    if(level->sprite->has_action("perfect"))
      level->sprite->set_action("perfect");
  }

  save_state();

  if (old_level_state != level->solved) {
    // Try to detect the next direction to which we should walk
    // FIXME: Mostly a hack
    Direction dir = D_NONE;

    int dirdata = available_directions_at(tux->get_tile_pos());
    // first, test for crossroads
    if (dirdata == Tile::WORLDMAP_CNSE ||
        dirdata == Tile::WORLDMAP_CNSW ||
        dirdata == Tile::WORLDMAP_CNEW ||
        dirdata == Tile::WORLDMAP_CSEW ||
        dirdata == Tile::WORLDMAP_CNSEW)
      dir = D_NONE;
    else if (dirdata & Tile::WORLDMAP_NORTH
             && tux->back_direction != D_NORTH)
      dir = D_NORTH;
    else if (dirdata & Tile::WORLDMAP_SOUTH
             && tux->back_direction != D_SOUTH)
      dir = D_SOUTH;
    else if (dirdata & Tile::WORLDMAP_EAST
             && tux->back_direction != D_EAST)
      dir = D_EAST;
    else if (dirdata & Tile::WORLDMAP_WEST
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
      log_warning << "Couldn't run level-extro-script: " << e.what() << std::endl;
    }
  }
}

Vector
WorldMap::get_camera_pos_for_tux() {
  Vector camera_offset;
  Vector tux_pos = tux->get_pos();
  camera_offset.x = tux_pos.x - SCREEN_WIDTH/2;
  camera_offset.y = tux_pos.y - SCREEN_HEIGHT/2;
  return camera_offset;
}

void
WorldMap::clamp_camera_position(Vector& c) {
  if (c.x < 0)
    c.x = 0;
  if (c.y < 0)
    c.y = 0;

  if (c.x > (int)get_width()*32 - SCREEN_WIDTH)
    c.x = (int)get_width()*32 - SCREEN_WIDTH;
  if (c.y > (int)get_height()*32 - SCREEN_HEIGHT)
    c.y = (int)get_height()*32 - SCREEN_HEIGHT;

  if (int(get_width()*32) < SCREEN_WIDTH)
    c.x = get_width()*16.0 - SCREEN_WIDTH/2.0;
  if (int(get_height()*32) < SCREEN_HEIGHT)
    c.y = get_height()*16.0 - SCREEN_HEIGHT/2.0;
}

void
WorldMap::update(float delta)
{
  if(!in_level) {
    Menu* menu = MenuManager::current();
    if(menu != NULL) {
      if(menu == worldmap_menu.get()) {
        switch (worldmap_menu->check())
        {
          case MNID_RETURNWORLDMAP: // Return to game
            MenuManager::set_current(0);
            break;
          case MNID_QUITWORLDMAP: // Quit Worldmap
            g_screen_manager->exit_screen();
            break;
        }
      }

      return;
    }

    // update GameObjects
    for(size_t i = 0; i < game_objects.size(); ++i) {
      GameObject* object = game_objects[i];
      if(!panning || object != tux) {
        object->update(delta);
      }
    }

    // remove old GameObjects
    for(GameObjects::iterator i = game_objects.begin();
        i != game_objects.end(); ) {
      GameObject* object = *i;
      if(!object->is_valid()) {
        try_unexpose(object);
        object->unref();
        i = game_objects.erase(i);
      } else {
        ++i;
      }
    }

    /* update solid_tilemaps list */
    //FIXME: this could be more efficient
    solid_tilemaps.clear();
    for(std::vector<GameObject*>::iterator i = game_objects.begin();
        i != game_objects.end(); ++i)
    {
      TileMap* tm = dynamic_cast<TileMap*>(*i);
      if (!tm) continue;
      if (tm->is_solid()) solid_tilemaps.push_back(tm);
    }

    Vector requested_pos;

    // position "camera"
    if(!panning) {
      camera_offset = get_camera_pos_for_tux();
    } else {
      Vector delta = pan_pos - camera_offset;
      float mag = delta.norm();
      if(mag > CAMERA_PAN_SPEED) {
        delta *= CAMERA_PAN_SPEED/mag;
      }
      camera_offset += delta;
      if(camera_offset == pan_pos) {
        panning = false;
      }
    }

    requested_pos = camera_offset;
    clamp_camera_position(camera_offset);

    if(panning) {
      if(requested_pos.x != camera_offset.x) {
        pan_pos.x = camera_offset.x;
      }
      if(requested_pos.y != camera_offset.y) {
        pan_pos.y = camera_offset.y;
      }
    }

    // handle input
    Controller *controller = g_jk_controller->get_main_controller();
    bool enter_level = false;
    if(controller->pressed(Controller::ACTION)
       || controller->pressed(Controller::JUMP)
       || controller->pressed(Controller::MENU_SELECT)) {
      /* some people define UP and JUMP on the same key... */
      if(!controller->pressed(Controller::UP))
        enter_level = true;
    }
    if(controller->pressed(Controller::PAUSE_MENU))
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
        move_to_spawnpoint(teleporter->spawnpoint, true);
      }
    }

    // check for auto-play levels
    LevelTile* level = at_level();
    if (level && (level->auto_play) && (!level->solved) && (!tux->is_moving())) {
      enter_level = true;
    }

    if (enter_level && !tux->is_moving())
    {
      /* Check level action */
      LevelTile* level = at_level();
      if (!level) {
        //Respawn if player on a tile with no level and nowhere to go.
        int tile_data = tile_data_at(tux->get_tile_pos());
        if(!( tile_data & ( Tile::WORLDMAP_NORTH |  Tile::WORLDMAP_SOUTH | Tile::WORLDMAP_WEST | Tile::WORLDMAP_EAST ))){
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
                                    level->pos.y*32 +  8 - camera_offset.y);
          std::string levelfile = levels_path + level->get_name();

          // update state and savegame
          save_state();

          g_screen_manager->push_screen(new GameSession(levelfile, player_status, &level->statistics),
                                   new ShrinkFade(shrinkpos, 1.0f));
          in_level = true;
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
}

int
WorldMap::tile_data_at(Vector p)
{
  int dirs = 0;

  for(std::list<TileMap*>::const_iterator i = solid_tilemaps.begin(); i != solid_tilemaps.end(); i++) {
    TileMap* tilemap = *i;
    const Tile* tile = tilemap->get_tile((int)p.x, (int)p.y);
    int dirdata = tile->getData();
    dirs |= dirdata;
  }

  return dirs;
}

int
WorldMap::available_directions_at(Vector p)
{
  return tile_data_at(p) & Tile::WORLDMAP_DIR_MASK;
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
  if (int(get_width()*32) < SCREEN_WIDTH || int(get_height()*32) < SCREEN_HEIGHT)
    context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
                             Color(0.0f, 0.0f, 0.0f, 1.0f), LAYER_BACKGROUND0);

  context.set_ambient_color( ambient_light );
  context.push_transform();
  context.set_translation(camera_offset);

  for(GameObjects::iterator i = game_objects.begin();
      i != game_objects.end(); ++i) {
    GameObject* object = *i;
    if(!panning || object != tux) {
      object->draw(context);
    }
  }

  /*
  // FIXME: make this a runtime switch similar to draw_collrects/show_collrects?
  // draw visual indication of possible walk directions
  static int flipme = 0; 
  if (flipme++ & 0x04)
  for (int x = 0; x < get_width(); x++) {
  for (int y = 0; y < get_height(); y++) {
  int data = tile_data_at(Vector(x,y));
  int px = x * 32;
  int py = y * 32;
  const int W = 4;
  if (data & Tile::WORLDMAP_NORTH)    context.draw_filled_rect(Rect(px + 16-W, py       , px + 16+W, py + 16-W), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_SOUTH)    context.draw_filled_rect(Rect(px + 16-W, py + 16+W, px + 16+W, py + 32  ), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_EAST)     context.draw_filled_rect(Rect(px + 16+W, py + 16-W, px + 32  , py + 16+W), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_WEST)     context.draw_filled_rect(Rect(px       , py + 16-W, px + 16-W, py + 16+W), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_DIR_MASK) context.draw_filled_rect(Rect(px + 16-W, py + 16-W, px + 16+W, py + 16+W), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  if (data & Tile::WORLDMAP_STOP)     context.draw_filled_rect(Rect(px + 4   , py + 4   , px + 28  , py + 28  ), Color(0.2f, 0.2f, 0.2f, 0.7f), LAYER_FOREGROUND1 + 1000);
  }
  }
  */

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

        context.draw_text(Resources::normal_font, level->title,
                          Vector(SCREEN_WIDTH/2,
                                 SCREEN_HEIGHT - Resources::normal_font->get_height() - 10),
                          ALIGN_CENTER, LAYER_HUD, WorldMap::level_title_color);

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

        get_level_target_time(*level);
        level->statistics.draw_worldmap_info(context, level->target_time);
        break;
      }
    }

    for(SpecialTiles::iterator i = special_tiles.begin();
        i != special_tiles.end(); ++i) {
      SpecialTile* special_tile = *i;

      if (special_tile->pos == tux->get_tile_pos()) {
        /* Display an in-map message in the map, if any as been selected */
        if(!special_tile->map_message.empty() && !special_tile->passive_message)
          context.draw_text(Resources::normal_font, special_tile->map_message,
                            Vector(SCREEN_WIDTH/2,
                                   SCREEN_HEIGHT - Resources::normal_font->get_height() - 60),
                            ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::message_color);
        break;
      }
    }

    // display teleporter messages
    Teleporter* teleporter = at_teleporter(tux->get_tile_pos());
    if (teleporter && (teleporter->message != "")) {
      Vector pos = Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT - Resources::normal_font->get_height() - 30);
      context.draw_text(Resources::normal_font, teleporter->message, pos, ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::teleporter_message_color);
    }

  }

  /* Display a passive message in the map, if needed */
  if(passive_message_timer.started())
    context.draw_text(Resources::normal_font, passive_message,
                      Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT - Resources::normal_font->get_height() - 60),
                      ALIGN_CENTER, LAYER_FOREGROUND1, WorldMap::message_color);

  context.pop_transform();
}

void
WorldMap::setup()
{
  sound_manager->play_music(music);
  MenuManager::set_current(NULL);

  current_ = this;
  load_state();

  // if force_spawnpoint was set, move Tux there, then clear force_spawnpoint
  if (force_spawnpoint != "") {
    move_to_spawnpoint(force_spawnpoint);
    force_spawnpoint = "";
  }

  tux->setup();

  // register worldmap_table as worldmap in scripting
  using namespace scripting;

  sq_pushroottable(global_vm);
  sq_pushstring(global_vm, "worldmap", -1);
  sq_pushobject(global_vm, worldmap_table);
  if(SQ_FAILED(sq_createslot(global_vm, -3)))
    throw SquirrelError(global_vm, "Couldn't set worldmap in roottable");
  sq_pop(global_vm, 1);

  //Run default.nut just before init script
  try {
    IFileStreambuf ins(levels_path + "default.nut");
    std::istream in(&ins);
    run_script(in, "WorldMap::default.nut");
  } catch(std::exception& ) {
    // doesn't exist or erroneous; do nothing
  }

  if(init_script != "") {
    std::istringstream in(init_script);
    run_script(in, "WorldMap::init");
  }
}

void
WorldMap::leave()
{
  using namespace scripting;

  // save state of world and player
  save_state();

  // remove worldmap_table from roottable
  sq_pushroottable(global_vm);
  sq_pushstring(global_vm, "worldmap", -1);
  if(SQ_FAILED(sq_deleteslot(global_vm, -2, SQFalse)))
    throw SquirrelError(global_vm, "Couldn't unset worldmap in roottable");
  sq_pop(global_vm, 1);
}

void
WorldMap::save_state()
{
  using namespace scripting;

  HSQUIRRELVM vm = global_vm;
  int oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    sq_pushstring(vm, "state", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw scripting::SquirrelError(vm, "Couldn't get state table");

    // get or create worlds table
    sq_pushstring(vm, "worlds", -1);
    if(SQ_FAILED(sq_get(vm, -2))) {
      sq_pushstring(vm, "worlds", -1);
      sq_newtable(vm);
      if(SQ_FAILED(sq_createslot(vm, -3)))
        throw scripting::SquirrelError(vm, "Couldn't create state.worlds");

      sq_pushstring(vm, "worlds", -1);
      if(SQ_FAILED(sq_get(vm, -2)))
        throw scripting::SquirrelError(vm, "Couldn't create.get state.worlds");
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

      sq_pushstring(vm, level->get_name().c_str(), -1);
      sq_newtable(vm);

      store_bool(vm, "solved", level->solved);
      store_bool(vm, "perfect", level->perfect);
      level->statistics.serialize_to_squirrel(vm);

      sq_createslot(vm, -3);
    }

    sq_createslot(vm, -3);

    // overall statistics...
    total_stats.serialize_to_squirrel(vm);

    // push world into worlds table
    sq_createslot(vm, -3);
  } catch(std::exception& ) {
    sq_settop(vm, oldtop);
  }

  sq_settop(vm, oldtop);

  if(World::current() != NULL)
    World::current()->save_state();
}

void
WorldMap::load_state()
{
  using namespace scripting;

  HSQUIRRELVM vm = global_vm;
  int oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    sq_pushstring(vm, "state", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw scripting::SquirrelError(vm, "Couldn't get state table");

    // get worlds table
    sq_pushstring(vm, "worlds", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw scripting::SquirrelError(vm, "Couldn't get state.worlds");

    // get table for our world
    sq_pushstring(vm, map_filename.c_str(), map_filename.length());
    if(SQ_FAILED(sq_get(vm, -2)))
      throw scripting::SquirrelError(vm, "Couldn't get state.worlds.mapfilename");

    // load tux
    sq_pushstring(vm, "tux", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
      throw scripting::SquirrelError(vm, "Couldn't get tux");

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
      throw scripting::SquirrelError(vm, "Couldn't get levels");

    for(LevelTiles::iterator i = levels.begin(); i != levels.end(); ++i) {
      LevelTile* level = *i;
      sq_pushstring(vm, level->get_name().c_str(), -1);
      if(SQ_SUCCEEDED(sq_get(vm, -2))) {
        level->solved = read_bool(vm, "solved");
        level->perfect = read_bool(vm, "perfect");
        if(!level->solved)
          level->sprite->set_action("default");
        else
          level->sprite->set_action((level->sprite->has_action("perfect") && level->perfect) ? "perfect" : "solved");
        level->statistics.unserialize_from_squirrel(vm);
        sq_pop(vm, 1);
      }
    }

    // leave state table
    sq_pop(vm, 1);

    // load overall statistics
    total_stats.unserialize_from_squirrel(vm);

  } catch(std::exception& e) {
    log_debug << "Not loading worldmap state: " << e.what() << std::endl;
  }
  sq_settop(vm, oldtop);

  in_level = false;
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
  using namespace scripting;

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

float
WorldMap::get_width() const
{
  float width = 0;
  for(std::list<TileMap*>::const_iterator i = solid_tilemaps.begin(); i != solid_tilemaps.end(); i++) {
    TileMap* solids = *i;
    if (solids->get_width() > width) width = solids->get_width();
  }
  return width;
}

float
WorldMap::get_height() const
{
  float height = 0;
  for(std::list<TileMap*>::const_iterator i = solid_tilemaps.begin(); i != solid_tilemaps.end(); i++) {
    TileMap* solids = *i;
    if (solids->get_height() > height) height = solids->get_height();
  }
  return height;
}

} // namespace worldmap

/* EOF */
