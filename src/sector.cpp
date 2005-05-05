//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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

#include <memory>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "sector.h"
#include "player_status.h"
#include "object/gameobjs.h"
#include "object/camera.h"
#include "object/background.h"
#include "object/particlesystem.h"
#include "object/tilemap.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "lisp/list_iterator.h"
#include "tile.h"
#include "audio/sound_manager.h"
#include "game_session.h"
#include "resources.h"
#include "statistics.h"
#include "collision_grid.h"
#include "collision_grid_iterator.h"
#include "object_factory.h"
#include "collision.h"
#include "math/rect.h"
#include "math/aatriangle.h"
#include "object/coin.h"
#include "object/block.h"
#include "object/invisible_block.h"
#include "object/bullet.h"
#include "badguy/jumpy.h"
#include "badguy/spike.h"
#include "trigger/sequence_trigger.h"
#include "player_status.h"
#include "scripting/script_interpreter.h"
#include "scripting/sound.h"
#include "scripting/scripted_object.h"

//#define USE_GRID

Sector* Sector::_current = 0;

Sector::Sector()
  : gravity(10), player(0), solids(0), camera(0),
    interpreter(0), currentmusic(LEVEL_MUSIC)
{
  song_title = "Mortimers_chipdisko.mod";
  player = new Player(&player_status);
  add_object(player);

  grid = new CollisionGrid(32000, 32000);
}

Sector::~Sector()
{
  update_game_objects();
  assert(gameobjects_new.size() == 0);

  delete grid;

  for(GameObjects::iterator i = gameobjects.begin(); i != gameobjects.end();
      ++i) {
    delete *i;
  }

  for(SpawnPoints::iterator i = spawnpoints.begin(); i != spawnpoints.end();
      ++i)
    delete *i;
    
  if(_current == this)
    _current = 0;
}

GameObject*
Sector::parse_object(const std::string& name, const lisp::Lisp& reader)
{
  if(name == "camera") {
    Camera* camera = new Camera(this);
    camera->parse(reader);
    return camera;
  } else if(name == "particles-snow") {
    SnowParticleSystem* partsys = new SnowParticleSystem();
    partsys->parse(reader);
    return partsys;
  } else if(name == "particles-rain") {
    RainParticleSystem* partsys = new RainParticleSystem();
    partsys->parse(reader);
    return partsys;
  } else if(name == "particles-clouds") {
    CloudParticleSystem* partsys = new CloudParticleSystem();
    partsys->parse(reader);
    return partsys;
  } else if(name == "money") { // for compatibility with old maps
    return new Jumpy(reader);
  } 

  try {
    return create_object(name, reader);
  } catch(std::exception& e) {
    std::cerr << e.what() << "\n";
  }
  
  return 0;
}

void
Sector::parse(const lisp::Lisp& sector)
{
  _current = this;
  
  lisp::ListIterator iter(&sector);
  while(iter.next()) {
    const std::string& token = iter.item();
    if(token == "name") {
      iter.value()->get(name);
    } else if(token == "gravity") {
      iter.value()->get(gravity);
    } else if(token == "music") {
      iter.value()->get(song_title);
      load_music();
    } else if(token == "spawnpoint") {
      const lisp::Lisp* spawnpoint_lisp = iter.lisp();
      
      SpawnPoint* sp = new SpawnPoint;
      spawnpoint_lisp->get("name", sp->name);
      spawnpoint_lisp->get("x", sp->pos.x);
      spawnpoint_lisp->get("y", sp->pos.y);
      spawnpoints.push_back(sp);
    } else if(token == "init-script") {
      iter.value()->get(init_script);
    } else {
      GameObject* object = parse_object(token, *(iter.lisp()));
      if(object) {
        add_object(object);
      }
    }
  }

  update_game_objects();
  fix_old_tiles();
  if(!camera) {
    std::cerr << "sector '" << name << "' does not contain a camera.\n";
    update_game_objects();
    add_object(new Camera(this));
  }
  if(!solids)
    throw std::runtime_error("sector does not contain a solid tile layer.");

  update_game_objects();
}

void
Sector::parse_old_format(const lisp::Lisp& reader)
{
  _current = this;
  
  name = "main";
  reader.get("gravity", gravity);

  std::string backgroundimage;
  reader.get("background", backgroundimage);
  float bgspeed = .5;
  reader.get("bkgd_speed", bgspeed);
  bgspeed /= 100;

  Color bkgd_top, bkgd_bottom;
  int r = 0, g = 0, b = 128;
  reader.get("bkgd_red_top", r);
  reader.get("bkgd_green_top",  g);
  reader.get("bkgd_blue_top",  b);
  bkgd_top.red = r;
  bkgd_top.green = g;
  bkgd_top.blue = b;
  
  reader.get("bkgd_red_bottom",  r);
  reader.get("bkgd_green_bottom", g);
  reader.get("bkgd_blue_bottom", b);
  bkgd_bottom.red = r;
  bkgd_bottom.green = g;
  bkgd_bottom.blue = b;
  
  if(backgroundimage != "") {
    Background* background = new Background;
    background->set_image(backgroundimage, bgspeed);
    add_object(background);
  } else {
    Background* background = new Background;
    background->set_gradient(bkgd_top, bkgd_bottom);
    add_object(background);
  }

  std::string particlesystem;
  reader.get("particle_system", particlesystem);
  if(particlesystem == "clouds")
    add_object(new CloudParticleSystem());
  else if(particlesystem == "snow")
    add_object(new SnowParticleSystem());
  else if(particlesystem == "rain")
    add_object(new RainParticleSystem());

  Vector startpos(100, 170);
  reader.get("start_pos_x", startpos.x);
  reader.get("start_pos_y", startpos.y);

  SpawnPoint* spawn = new SpawnPoint;
  spawn->pos = startpos;
  spawn->name = "main";
  spawnpoints.push_back(spawn);

  song_title = "Mortimers_chipdisko.mod";
  reader.get("music", song_title);
  load_music();

  int width, height = 15;
  reader.get("width", width);
  reader.get("height", height);
  
  std::vector<unsigned int> tiles;
  if(reader.get_vector("interactive-tm", tiles)
      || reader.get_vector("tilemap", tiles)) {
    TileMap* tilemap = new TileMap();
    tilemap->set(width, height, tiles, LAYER_TILES, true);
    add_object(tilemap);
  }

  if(reader.get_vector("background-tm", tiles)) {
    TileMap* tilemap = new TileMap();
    tilemap->set(width, height, tiles, LAYER_BACKGROUNDTILES, false);
    add_object(tilemap);
  }

  if(reader.get_vector("foreground-tm", tiles)) {
    TileMap* tilemap = new TileMap();
    tilemap->set(width, height, tiles, LAYER_FOREGROUNDTILES, false);
    add_object(tilemap);
  }

  // read reset-points (now spawn-points)
  const lisp::Lisp* resetpoints = reader.get_lisp("reset-points");
  if(resetpoints) {
    lisp::ListIterator iter(resetpoints);
    while(iter.next()) {
      if(iter.item() == "point") {
        Vector sp_pos;
        if(reader.get("x", sp_pos.x) && reader.get("y", sp_pos.y))
          {
          SpawnPoint* sp = new SpawnPoint;
          sp->name = "main";
          sp->pos = sp_pos;
          spawnpoints.push_back(sp);
          }
      } else {
        std::cerr << "Unknown token '" << iter.item() << "' in reset-points.\n";
      }
    }
  }

  // read objects
  const lisp::Lisp* objects = reader.get_lisp("objects");
  if(objects) {
    lisp::ListIterator iter(objects);
    while(iter.next()) {
      GameObject* object = parse_object(iter.item(), *(iter.lisp()));
      if(object) {
        add_object(object);
      } else {
        std::cerr << "Unknown object '" << iter.item() << "' in level.\n";
      }
    }
  }

  // add a camera
  Camera* camera = new Camera(this);
  add_object(camera);

  update_game_objects();
  fix_old_tiles();
  update_game_objects();
  if(solids == 0)
    throw std::runtime_error("sector does not contain a solid tile layer.");  
}

void
Sector::fix_old_tiles()
{
  // hack for now...
  for(size_t x=0; x < solids->get_width(); ++x) {
    for(size_t y=0; y < solids->get_height(); ++y) {
      const Tile* tile = solids->get_tile(x, y);
      Vector pos(x*32, y*32);
      
      if(tile->getID() == 112) {
        add_object(new InvisibleBlock(pos));
        solids->change(x, y, 0);
      } else if(tile->getID() == 295) {
        add_object(new Spike(pos, Spike::NORTH));
        solids->change(x, y, 0);
      } else if(tile->getID() == 296) {
        add_object(new Spike(pos, Spike::EAST));
        solids->change(x, y, 0);
      } else if(tile->getID() == 297) {
        add_object(new Spike(pos, Spike::SOUTH));
        solids->change(x, y, 0);
      } else if(tile->getID() == 298) {
        add_object(new Spike(pos, Spike::WEST));
        solids->change(x, y, 0);
      } else if(tile->getAttributes() & Tile::COIN) {
        add_object(new Coin(pos));
        solids->change(x, y, 0);
      } else if(tile->getAttributes() & Tile::FULLBOX) {
        add_object(new BonusBlock(pos, tile->getData()));
        solids->change(x, y, 0);
      } else if(tile->getAttributes() & Tile::BRICK) {
        add_object(new Brick(pos, tile->getData()));
        solids->change(x, y, 0);
      } else if(tile->getAttributes() & Tile::GOAL) {
        std::string sequence = tile->getData() == 0 ? "endsequence" : "stoptux";
        add_object(new SequenceTrigger(pos, sequence));
        solids->change(x, y, 0);
      }
    }                                                   
  }
}

void
Sector::write(lisp::Writer& writer)
{
  writer.write_string("name", name);
  writer.write_float("gravity", gravity);
  writer.write_string("music", song_title);

  // write spawnpoints
  for(SpawnPoints::iterator i = spawnpoints.begin(); i != spawnpoints.end();
      ++i) {
    SpawnPoint* spawn = *i;
    writer.start_list("spawn-points");
    writer.write_string("name", spawn->name);
    writer.write_float("x", spawn->pos.x);
    writer.write_float("y", spawn->pos.y);
    writer.end_list("spawn-points");
  }

  // write objects
  for(GameObjects::iterator i = gameobjects.begin();
      i != gameobjects.end(); ++i) {
    Serializable* serializable = dynamic_cast<Serializable*> (*i);
    if(serializable)
      serializable->write(writer);
  }
}

void
Sector::add_object(GameObject* object)
{
  // make sure the object isn't already in the list
#ifdef DEBUG
  for(GameObjects::iterator i = gameobjects.begin(); i != gameobjects.end();
      ++i) {
    if(*i == object) {
      assert("object already added to sector" == 0);
    }
  }
  for(GameObjects::iterator i = gameobjects_new.begin();
      i != gameobjects_new.end(); ++i) {
    if(*i == object) {
      assert("object already added to sector" == 0);
    }
  }
#endif

  gameobjects_new.push_back(object);
}

void
Sector::activate(const std::string& spawnpoint)
{
  SpawnPoint* sp = 0;
  for(SpawnPoints::iterator i = spawnpoints.begin(); i != spawnpoints.end();
      ++i) {
    if((*i)->name == spawnpoint) {
      sp = *i;
      break;
    }
  }                                                                           
  if(!sp) {
    std::cerr << "Spawnpoint '" << spawnpoint << "' not found.\n";
    if(spawnpoint != "main") {
      activate("main");
    } else {
      activate(Vector(0, 0));
    }
  } else {
    activate(sp->pos);
  }

  // Run init script
  if(init_script != "") {
    try {
      // TODO we should keep the interpreter across sessions (or some variables)
      // so that you can store information across levels/sectors...
      delete interpreter;
      interpreter = 0;
      interpreter = new ScriptInterpreter();

      // expose ScriptedObjects to the script
      for(GameObjects::iterator i = gameobjects.begin();
          i != gameobjects.end(); ++i) {
        GameObject* object = *i;
        Scripting::ScriptedObject* scripted_object
          = dynamic_cast<Scripting::ScriptedObject*> (object);
        if(!scripted_object)
          continue;

        std::cout << "Exposing " << scripted_object->get_name() << "\n";
        interpreter->expose_object(scripted_object,
                                   scripted_object->get_name(),
                                   "ScriptedObject");
      }
      Scripting::Sound* sound = new Scripting::Sound();
      interpreter->expose_object(sound, "Sound", "Sound");

      std::string sourcename = std::string("Sector(") + name + ") - init";
      std::istringstream in(init_script);
      printf("Load script.\n");
      interpreter->load_script(in, sourcename);
      printf("run script.\n");
      interpreter->run_script();
    } catch(std::exception& e) {
      std::cerr << "Couldn't execute init script: " << e.what() << "\n";
    }
  }
}

void
Sector::activate(const Vector& player_pos)
{
  _current = this;

  player->move(player_pos);
  camera->reset(player->get_pos());
}

Rect
Sector::get_active_region()
{
  return Rect(
    camera->get_translation() - Vector(1600, 1200),
    camera->get_translation() + Vector(1600, 1200));
}

void
Sector::action(float elapsed_time)
{
  if(interpreter)
    interpreter->update();
  
  player->check_bounds(camera);

#if 0
  CollisionGridIterator iter(*grid, get_active_region());
  while(MovingObject* object = iter.next()) {
    if(!object->is_valid())
      continue;

    object->action(elapsed_time);
  }
#else
  /* update objects */
  for(GameObjects::iterator i = gameobjects.begin();
          i != gameobjects.end(); ++i) {
    GameObject* object = *i;
    if(!object->is_valid())
      continue;
    
    object->action(elapsed_time);
  }
#endif
  
  /* Handle all possible collisions. */
  collision_handler();                                                                              
  update_game_objects();
}

void
Sector::update_game_objects()
{
  /** cleanup marked objects */
  for(std::vector<GameObject*>::iterator i = gameobjects.begin();
      i != gameobjects.end(); /* nothing */) {
    GameObject* object = *i;
    
    if(object->is_valid()) {
      ++i;
      continue;
    }
    
    Bullet* bullet = dynamic_cast<Bullet*> (object);
    if(bullet) {
      bullets.erase(
          std::remove(bullets.begin(), bullets.end(), bullet),
          bullets.end());
    }
    MovingObject* movingobject = dynamic_cast<MovingObject*> (object);
    if(movingobject) {
      grid->remove_object(movingobject);
    }
    delete *i;
    i = gameobjects.erase(i);
  }

  /* add newly created objects */
  for(std::vector<GameObject*>::iterator i = gameobjects_new.begin();
      i != gameobjects_new.end(); ++i)
  {
    GameObject* object = *i;
    
    Bullet* bullet = dynamic_cast<Bullet*> (object);
    if(bullet)
      bullets.push_back(bullet);

    MovingObject* movingobject = dynamic_cast<MovingObject*> (object);
    if(movingobject)
      grid->add_object(movingobject);
    
    TileMap* tilemap = dynamic_cast<TileMap*> (object);
    if(tilemap && tilemap->is_solid()) {
      if(solids == 0) {
        solids = tilemap;
      } else {
        std::cerr << "Another solid tilemaps added. Ignoring.";
      }
    }

    Camera* camera = dynamic_cast<Camera*> (object);
    if(camera) {
      if(this->camera != 0) {
        std::cerr << "Warning: Multiple cameras added. Ignoring.";
        continue;
      }
      this->camera = camera;
    }

    gameobjects.push_back(object);
  }
  gameobjects_new.clear();
}

void
Sector::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(camera->get_translation());

#if 0
  CollisionGridIterator iter(*grid, get_active_region());
  while(MovingObject* object = iter.next()) {
    if(!object->is_valid())
      continue;

    object->draw(context);
  }
#else
  for(GameObjects::iterator i = gameobjects.begin();
      i != gameobjects.end(); ++i) {
    GameObject* object = *i; 
    if(!object->is_valid())
      continue;
    
    object->draw(context);
  }
#endif

  context.pop_transform();
}

static const float DELTA = .001;

void
Sector::collision_tilemap(MovingObject* object, int depth)
{
  if(depth >= 4) {
#ifdef DEBUG
    std::cout << "Max collision depth reached.\n";
#endif
    object->movement = Vector(0, 0);
    return;
  }

  // calculate rectangle where the object will move
  float x1, x2;
  if(object->get_movement().x >= 0) {
    x1 = object->get_pos().x;
    x2 = object->get_bbox().p2.x + object->get_movement().x;
  } else {
    x1 = object->get_pos().x + object->get_movement().x;
    x2 = object->get_bbox().p2.x;
  }
  float y1, y2;
  if(object->get_movement().y >= 0) {
    y1 = object->get_pos().y;
    y2 = object->get_bbox().p2.y + object->get_movement().y;
  } else {
    y1 = object->get_pos().y + object->get_movement().y;
    y2 = object->get_bbox().p2.y;
  }

  // test with all tiles in this rectangle
  int starttilex = int(x1-1) / 32;
  int starttiley = int(y1-1) / 32;
  int max_x = int(x2+1);
  int max_y = int(y2+1);

  CollisionHit temphit, hit;
  Rect dest = object->get_bbox();
  dest.move(object->movement);
  hit.time = -1; // represents an invalid value
  for(int x = starttilex; x*32 < max_x; ++x) {
    for(int y = starttiley; y*32 < max_y; ++y) {
      const Tile* tile = solids->get_tile(x, y);
      if(!tile)
        continue;
      // skip non-solid tiles
      if(!(tile->getAttributes() & Tile::SOLID))
        continue;
      // only handle unisolid when the player is falling down and when he was
      // above the tile before
      if(tile->getAttributes() & Tile::UNISOLID) {
        if(object->movement.y < 0 || object->get_bbox().p2.y > y*32)
          continue;
      }

      if(tile->getAttributes() & Tile::SLOPE) { // slope tile
        AATriangle triangle;
        Vector p1(x*32, y*32);
        Vector p2((x+1)*32, (y+1)*32);
        triangle = AATriangle(p1, p2, tile->getData());

        if(Collision::rectangle_aatriangle(temphit, dest, object->movement,
              triangle)) {
          if(temphit.time > hit.time)
            hit = temphit;
        }
      } else { // normal rectangular tile
        Rect rect(x*32, y*32, (x+1)*32, (y+1)*32);
        if(Collision::rectangle_rectangle(temphit, dest,
              object->movement, rect)) {
          if(temphit.time > hit.time)
            hit = temphit;
        }
      }
    }
  }

  // did we collide at all?
  if(hit.time < 0)
    return;
 
  // call collision function
  HitResponse response = object->collision(*solids, hit);
  if(response == ABORT_MOVE) {
    object->movement = Vector(0, 0);
    return;
  }
  if(response == FORCE_MOVE) {
      return;
  }
  // move out of collision and try again
  object->movement += hit.normal * (hit.depth + DELTA);
  collision_tilemap(object, depth+1);
}

void
Sector::collision_object(MovingObject* object1, MovingObject* object2)
{
  CollisionHit hit;
  Rect dest1 = object1->get_bbox();
  dest1.move(object1->get_movement());
  Rect dest2 = object2->get_bbox();
  dest2.move(object2->get_movement());

  Vector movement = object1->get_movement() - object2->get_movement();
  if(Collision::rectangle_rectangle(hit, dest1, movement, dest2)) {
    HitResponse response1 = object1->collision(*object2, hit);
    hit.normal *= -1;
    HitResponse response2 = object2->collision(*object1, hit);

    if(response1 != CONTINUE) {
      if(response1 == ABORT_MOVE)
        object1->movement = Vector(0, 0);
      if(response2 == CONTINUE)
        object2->movement += hit.normal * (hit.depth + DELTA);
    } else if(response2 != CONTINUE) {
      if(response2 == ABORT_MOVE)
        object2->movement = Vector(0, 0);
      if(response1 == CONTINUE)
        object1->movement += -hit.normal * (hit.depth + DELTA);
    } else {
      object1->movement += -hit.normal * (hit.depth/2 + DELTA);
      object2->movement += hit.normal * (hit.depth/2 + DELTA);
    }
  }
}

void
Sector::collision_handler()
{
#ifdef USE_GRID
  grid->check_collisions();
#else
  for(std::vector<GameObject*>::iterator i = gameobjects.begin();
      i != gameobjects.end(); ++i) {
    GameObject* gameobject = *i;
    if(!gameobject->is_valid())
      continue;
    MovingObject* movingobject = dynamic_cast<MovingObject*> (gameobject);
    if(!movingobject)
      continue;
    if(movingobject->get_flags() & GameObject::FLAG_NO_COLLDET) {
      movingobject->bbox.move(movingobject->movement);
      movingobject->movement = Vector(0, 0);
      continue;
    }

    // collision with tilemap
    if(! (movingobject->movement == Vector(0, 0)))
      collision_tilemap(movingobject, 0);

    // collision with other objects
    for(std::vector<GameObject*>::iterator i2 = i+1;
        i2 != gameobjects.end(); ++i2) {
      GameObject* other_object = *i2;
      if(!other_object->is_valid() 
          || other_object->get_flags() & GameObject::FLAG_NO_COLLDET)
        continue;
      MovingObject* movingobject2 = dynamic_cast<MovingObject*> (other_object);
      if(!movingobject2)
        continue;

      collision_object(movingobject, movingobject2);
    }

    movingobject->bbox.move(movingobject->get_movement());
    movingobject->movement = Vector(0, 0);
  }
#endif
}

bool
Sector::add_bullet(const Vector& pos, float xm, Direction dir)
{
  // TODO remove this function and move these checks elsewhere...
  static const size_t MAX_FIRE_BULLETS = 2;
  static const size_t MAX_ICE_BULLETS = 1;

  Bullet* new_bullet = 0;
  if(player_status.bonus == FIRE_BONUS) {
    if(bullets.size() > MAX_FIRE_BULLETS-1)
      return false;
    new_bullet = new Bullet(pos, xm, dir, FIRE_BULLET);
  } else if(player_status.bonus == ICE_BONUS) {
    if(bullets.size() > MAX_ICE_BULLETS-1)
      return false;
    new_bullet = new Bullet(pos, xm, dir, ICE_BULLET);
  } else {
    return false;
  }
  add_object(new_bullet);

  sound_manager->play_sound("shoot");

  return true;
}

bool
Sector::add_smoke_cloud(const Vector& pos)
{
  add_object(new SmokeCloud(pos));
  return true;
}

void
Sector::add_floating_text(const Vector& pos, const std::string& text)
{
  add_object(new FloatingText(pos, text));
}

void
Sector::load_music()
{
  level_song = sound_manager->load_music(
    get_resource_filename("/music/" + song_title));
}

void
Sector::play_music(MusicType type)
{
  currentmusic = type;
  switch(currentmusic) {
    case LEVEL_MUSIC:
      sound_manager->play_music(level_song);
      break;
    case HERRING_MUSIC:
      sound_manager->play_music(herring_song);
      break;
    default:
      sound_manager->halt_music();
      break;
  }
}

MusicType
Sector::get_music_type()
{
  return currentmusic;
}

int
Sector::get_total_badguys()
{
  int total_badguys = 0;
  for(GameObjects::iterator i = gameobjects.begin();
      i != gameobjects.end(); ++i) {
    BadGuy* badguy = dynamic_cast<BadGuy*> (*i);
    if(badguy)
      total_badguys++;
  }

  return total_badguys;
}

bool
Sector::inside(const Rect& rect) const
{
  if(rect.p1.x > solids->get_width() * 32 
      || rect.p1.y > solids->get_height() * 32
      || rect.p2.x < 0 || rect.p2.y < 0)
    return false;

  return true;
}
