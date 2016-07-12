//  SuperTux -  A Jump'n Run
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

#include "supertux/sector.hpp"

#include <algorithm>
#include <math.h>
#include <vector>

#include "scripting/scripting.hpp"
#include "scripting/squirrel_util.hpp"
#include "scripting/sector.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/jumpy.hpp"
#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/display_effect.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/smoke_cloud.hpp"
#include "object/text_object.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "supertux/collision.hpp"
#include "supertux/constants.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/player_status.hpp"
#include "supertux/savegame.hpp"
#include "supertux/spawn_point.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "trigger/sequence_trigger.hpp"
#include "util/file_system.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

Sector* Sector::_current = 0;

bool Sector::show_collrects = false;
bool Sector::draw_solids_only = false;

Sector::Sector(Level* parent) :
  level(parent),
  name(),
  bullets(),
  init_script(),
  gameobjects_new(),
  currentmusic(LEVEL_MUSIC),
  sector_table(),
  scripts(),
  ambient_light( 1.0f, 1.0f, 1.0f, 1.0f ),
  foremost_layer(),
  gameobjects(),
  moving_objects(),
  spawnpoints(),
  portables(),
  music(),
  gravity(10.0),
  player(0),
  solid_tilemaps(),
  camera(0),
  effect(0)
{
  PlayerStatus* player_status;
  if (Editor::is_active()) {
    player_status = Editor::current()->m_savegame->get_player_status();
  } else {
    player_status = GameSession::current()->get_savegame().get_player_status();
  }
  if (!player_status) {
    log_warning << "Player status is not initialized." << std::endl;
  }
  add_object(std::make_shared<Player>(player_status, "Tux"));
  add_object(std::make_shared<DisplayEffect>("Effect"));
  add_object(std::make_shared<TextObject>("Text"));

  SoundManager::current()->preload("sounds/shoot.wav");

  // create a new squirrel table for the sector
  using namespace scripting;

  sq_collectgarbage(global_vm);

  sq_newtable(global_vm);
  sq_pushroottable(global_vm);
  if(SQ_FAILED(sq_setdelegate(global_vm, -2)))
    throw scripting::SquirrelError(global_vm, "Couldn't set sector_table delegate");

  sq_resetobject(&sector_table);
  if(SQ_FAILED(sq_getstackobj(global_vm, -1, &sector_table)))
    throw scripting::SquirrelError(global_vm, "Couldn't get sector table");
  sq_addref(global_vm, &sector_table);
  sq_pop(global_vm, 1);
}

Sector::~Sector()
{
  using namespace scripting;
  try
  {
    deactivate();
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }


  for(auto& script : scripts) {
    sq_release(global_vm, &script);
  }
  sq_release(global_vm, &sector_table);
  sq_collectgarbage(global_vm);

  update_game_objects();
  assert(gameobjects_new.size() == 0);

  for(auto& object: gameobjects) {
    before_object_remove(object);
  }
}

Level*
Sector::get_level() const
{
  return level;
}

HSQUIRRELVM
Sector::run_script(std::istream& in, const std::string& sourcename)
{
  using namespace scripting;

  // garbage collect thread list
  for(auto i = scripts.begin(); i != scripts.end(); ) {
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

  // set sector_table as roottable for the thread
  sq_pushobject(vm, sector_table);
  sq_setroottable(vm);

  try {
    compile_and_run(vm, in, "Sector " + name + " - " + sourcename);
  } catch(std::exception& e) {
    log_warning << "Error running script: " << e.what() << std::endl;
  }

  return vm;
}

void
Sector::add_object(GameObjectPtr object)
{
  // make sure the object isn't already in the list
#ifndef NDEBUG
  for(auto& game_object : gameobjects) {
    assert(game_object != object);
  }
  for(auto& gameobject : gameobjects_new) {
    assert(gameobject != object);
  }
#endif

  gameobjects_new.push_back(object);
}

void
Sector::activate(const std::string& spawnpoint)
{
  std::shared_ptr<SpawnPoint> sp;
  for(auto& spawn_point : spawnpoints) {
    if(spawn_point->name == spawnpoint) {
      sp = spawn_point;
      break;
    }
  }
  if(!sp) {
    log_warning << "Spawnpoint '" << spawnpoint << "' not found." << std::endl;
    if(spawnpoint != "main") {
      activate("main");
    } else {
      activate(Vector(0, 0));
    }
  } else {
    activate(sp->pos);
  }
}

void
Sector::activate(const Vector& player_pos)
{
  if(_current != this) {
    if(_current != NULL)
      _current->deactivate();
    _current = this;

    // register sectortable as sector in scripting
    HSQUIRRELVM vm = scripting::global_vm;
    sq_pushroottable(vm);
    sq_pushstring(vm, "sector", -1);
    sq_pushobject(vm, sector_table);
    if(SQ_FAILED(sq_createslot(vm, -3)))
      throw scripting::SquirrelError(vm, "Couldn't set sector in roottable");
    sq_pop(vm, 1);

    for(auto& object : gameobjects) {
      try_expose(object);
    }
  }
  try_expose_me();


  // two-player hack: move other players to main player's position
  // Maybe specify 2 spawnpoints in the level?
  for(auto& object : gameobjects) {
    auto p = dynamic_cast<Player*>(object.get());
    if (!p) continue;

    // spawn smalltux below spawnpoint
    if (!p->is_big()) {
      p->move(player_pos + Vector(0,32));
    } else {
      p->move(player_pos);
    }

    // spawning tux in the ground would kill him
    if(!is_free_of_tiles(p->get_bbox())) {
      std::string current_level = "[" + Sector::current()->get_level()->filename + "] ";
      log_warning << current_level << "Tried spawning Tux in solid matter. Compensating." << std::endl;
      Vector npos = p->get_bbox().p1;
      npos.y-=32;
      p->move(npos);
    }
  }

  //FIXME: This is a really dirty workaround for this strange camera jump
  player->move(player->get_pos()+Vector(-32, 0));
  camera->reset(player->get_pos());
  camera->update(1);
  player->move(player->get_pos()+(Vector(32, 0)));
  camera->update(1);

  update_game_objects();

  //Run default.nut just before init script
  //Check to see if it's in a levelset (info file)
  std::string basedir = FileSystem::dirname(get_level()->filename);
  if(PHYSFS_exists((basedir + "/info").c_str())) {
    try {
      IFileStreambuf ins(basedir + "/default.nut");
      std::istream in(&ins);
      run_script(in, "default.nut");
    } catch(std::exception& ) {
      // doesn't exist or erroneous; do nothing
    }
  }

  // Run init script
  if(!init_script.empty() && !Editor::is_active()) {
    std::istringstream in(init_script);
    run_script(in, "init-script");
  }
}

void
Sector::deactivate()
{
  if(_current != this)
    return;

  // remove sector entry from global vm
  HSQUIRRELVM vm = scripting::global_vm;
  sq_pushroottable(vm);
  sq_pushstring(vm, "sector", -1);
  if(SQ_FAILED(sq_deleteslot(vm, -2, SQFalse)))
    throw scripting::SquirrelError(vm, "Couldn't unset sector in roottable");
  sq_pop(vm, 1);

  for(auto object: gameobjects) {
    try_unexpose(object);
  }

  try_unexpose_me();
  _current = NULL;
}

Rectf
Sector::get_active_region() const
{
  return Rectf(
    camera->get_translation() - Vector(1600, 1200),
    camera->get_translation() + Vector(1600, 1200) + Vector(SCREEN_WIDTH,SCREEN_HEIGHT));
}

int
Sector::calculate_foremost_layer() const
{
  int layer = LAYER_BACKGROUND0;
  for(auto i = gameobjects.begin(); i != gameobjects.end(); ++i)
  {
    TileMap* tm = dynamic_cast<TileMap*>(i->get());
    if (!tm) continue;
    if(tm->get_layer() > layer)
    {
      if( (tm->get_alpha() < 1.0) )
      {
        layer = tm->get_layer() - 1;
      }
      else
      {
        layer = tm->get_layer() + 1;
      }
    }
  }
  log_debug << "Calculated baduy falling layer was: " << layer << std::endl;
  return layer;
}

int
Sector::get_foremost_layer() const
{
  return foremost_layer;
}

void
Sector::update(float elapsed_time)
{
  player->check_bounds();

  /* update objects */
  for(auto& object : gameobjects) {
    if(!object->is_valid())
      continue;

    object->update(elapsed_time);
  }

  /* Handle all possible collisions. */
  handle_collisions();
  update_game_objects();
}

void
Sector::update_game_objects()
{
  /** cleanup marked objects */
  for(auto i = gameobjects.begin();
      i != gameobjects.end(); /* nothing */) {
    GameObjectPtr& object = *i;

    if(object->is_valid()) {
      ++i;
      continue;
    }

    before_object_remove(object);

    i = gameobjects.erase(i);
  }

  /* add newly created objects */
  for(auto& object : gameobjects_new)
  {
    before_object_add(object);

    gameobjects.push_back(object);
  }
  gameobjects_new.clear();

  /* update solid_tilemaps list */
  //FIXME: this could be more efficient
  solid_tilemaps.clear();
  for(auto i = gameobjects.begin(); i != gameobjects.end(); ++i)
  {
    TileMap* tm = dynamic_cast<TileMap*>(i->get());
    if (!tm) continue;
    if (tm->is_solid()) solid_tilemaps.push_back(tm);
  }

}

bool
Sector::before_object_add(GameObjectPtr object)
{
  auto bullet = dynamic_cast<Bullet*>(object.get());
  if (bullet)
  {
    bullets.push_back(bullet);
  }

  auto movingobject = dynamic_cast<MovingObject*>(object.get());
  if (movingobject)
  {
    moving_objects.push_back(movingobject);
  }

  auto portable = dynamic_cast<Portable*>(object.get());
  if(portable)
  {
    portables.push_back(portable);
  }

  auto tilemap = dynamic_cast<TileMap*>(object.get());
  if(tilemap && tilemap->is_solid()) {
    solid_tilemaps.push_back(tilemap);
  }

  auto camera_ = dynamic_cast<Camera*>(object.get());
  if(camera_) {
    if(this->camera != 0) {
      log_warning << "Multiple cameras added. Ignoring" << std::endl;
      return false;
    }
    this->camera = camera_;
  }

  auto player_ = dynamic_cast<Player*>(object.get());
  if(player_) {
    if(this->player != 0) {
      log_warning << "Multiple players added. Ignoring" << std::endl;
      return false;
    }
    this->player = player_;
  }

  auto effect_ = dynamic_cast<DisplayEffect*>(object.get());
  if(effect_) {
    if(this->effect != 0) {
      log_warning << "Multiple DisplayEffects added. Ignoring" << std::endl;
      return false;
    }
    this->effect = effect_;
  }

  if(_current == this) {
    try_expose(object);
  }

  return true;
}

void
Sector::try_expose(GameObjectPtr object)
{
  ScriptInterface* object_ = dynamic_cast<ScriptInterface*>(object.get());
  if(object_ != NULL) {
    HSQUIRRELVM vm = scripting::global_vm;
    sq_pushobject(vm, sector_table);
    object_->expose(vm, -1);
    sq_pop(vm, 1);
  }
}

void
Sector::try_expose_me()
{
  HSQUIRRELVM vm = scripting::global_vm;
  sq_pushobject(vm, sector_table);
  auto obj = new scripting::Sector(this);
  expose_object(vm, -1, obj, "settings", true);
  sq_pop(vm, 1);
}

void
Sector::before_object_remove(GameObjectPtr object)
{
  Portable* portable = dynamic_cast<Portable*>(object.get());
  if (portable) {
    portables.erase(std::find(portables.begin(), portables.end(), portable));
  }
  Bullet* bullet = dynamic_cast<Bullet*>(object.get());
  if (bullet) {
    bullets.erase(std::find(bullets.begin(), bullets.end(), bullet));
  }
  MovingObject* moving_object = dynamic_cast<MovingObject*>(object.get());
  if (moving_object) {
    moving_objects.erase(
      std::find(moving_objects.begin(), moving_objects.end(), moving_object));
  }

  if(_current == this)
    try_unexpose(object);
}

void
Sector::try_unexpose(GameObjectPtr object)
{
  ScriptInterface* object_ = dynamic_cast<ScriptInterface*>(object.get());
  if(object_ != NULL) {
    HSQUIRRELVM vm = scripting::global_vm;
    SQInteger oldtop = sq_gettop(vm);
    sq_pushobject(vm, sector_table);
    try {
      object_->unexpose(vm, -1);
    } catch(std::exception& e) {
      log_warning << "Couldn't unregister object: " << e.what() << std::endl;
    }
    sq_settop(vm, oldtop);
  }
}

void
Sector::try_unexpose_me()
{
  HSQUIRRELVM vm = scripting::global_vm;
  SQInteger oldtop = sq_gettop(vm);
  sq_pushobject(vm, sector_table);
  try {
    scripting::unexpose_object(vm, -1, "settings");
  } catch(std::exception& e) {
    log_warning << "Couldn't unregister object: " << e.what() << std::endl;
  }
  sq_settop(vm, oldtop);
}
void
Sector::draw(DrawingContext& context)
{
  context.set_ambient_color( ambient_light );
  context.push_transform();
  context.set_translation(camera->get_translation());

  for(auto& object : gameobjects) {
    if(!object->is_valid())
      continue;

    if (draw_solids_only)
    {
      TileMap* tm = dynamic_cast<TileMap*>(object.get());
      if (tm && !tm->is_solid())
        continue;
    }

    object->draw(context);
  }

  if(show_collrects) {
    Color color(1.0f, 0.0f, 0.0f, 0.75f);
    for(auto& object : moving_objects) {
      const Rectf& rect = object->get_bbox();

      context.draw_filled_rect(rect, color, LAYER_FOREGROUND1 + 10);
    }
  }

  context.pop_transform();
}

/*-------------------------------------------------------------------------
 * Collision Detection
 *-------------------------------------------------------------------------*/

/** r1 is supposed to be moving, r2 a solid object */
void check_collisions(collision::Constraints* constraints,
                      const Vector& obj_movement, const Rectf& obj_rect, const Rectf& other_rect,
                      GameObject* object = NULL, MovingObject* other = NULL, const Vector& other_movement = Vector(0,0))
{
  if(!collision::intersects(obj_rect, other_rect))
    return;

  MovingObject *moving_object = dynamic_cast<MovingObject*> (object);
  CollisionHit dummy;
  if(other != NULL && object != NULL && !other->collides(*object, dummy))
    return;
  if(moving_object != NULL && other != NULL && !moving_object->collides(*other, dummy))
    return;

  // calculate intersection
  float itop    = obj_rect.get_bottom() - other_rect.get_top();
  float ibottom = other_rect.get_bottom() - obj_rect.get_top();
  float ileft   = obj_rect.get_right() - other_rect.get_left();
  float iright  = other_rect.get_right() - obj_rect.get_left();

  if(fabsf(obj_movement.y) > fabsf(obj_movement.x)) {
    if(ileft < SHIFT_DELTA) {
      constraints->constrain_right(other_rect.get_left(), other_movement.x);
      return;
    } else if(iright < SHIFT_DELTA) {
      constraints->constrain_left(other_rect.get_right(), other_movement.x);
      return;
    }
  } else {
    // shiftout bottom/top
    if(itop < SHIFT_DELTA) {
      constraints->constrain_bottom(other_rect.get_top(), other_movement.y);
      return;
    } else if(ibottom < SHIFT_DELTA) {
      constraints->constrain_top(other_rect.get_bottom(), other_movement.y);
      return;
    }
  }

  constraints->ground_movement += other_movement;
  if(other != NULL && object != NULL) {
    HitResponse response = other->collision(*object, dummy);
    if(response == ABORT_MOVE)
      return;

    if(other->get_movement() != Vector(0, 0)) {
      // TODO what todo when we collide with 2 moving objects?!?
      constraints->ground_movement += other->get_movement();
    }
  }

  float vert_penetration = std::min(itop, ibottom);
  float horiz_penetration = std::min(ileft, iright);
  if(vert_penetration < horiz_penetration) {
    if(itop < ibottom) {
      constraints->constrain_bottom(other_rect.get_top(), other_movement.y);
      constraints->hit.bottom = true;
    } else {
      constraints->constrain_top(other_rect.get_bottom(), other_movement.y);
      constraints->hit.top = true;
    }
  } else {
    if(ileft < iright) {
      constraints->constrain_right(other_rect.get_left(), other_movement.x);
      constraints->hit.right = true;
    } else {
      constraints->constrain_left(other_rect.get_right(), other_movement.x);
      constraints->hit.left = true;
    }
  }
}

void
Sector::collision_tilemap(collision::Constraints* constraints,
                          const Vector& movement, const Rectf& dest,
                          MovingObject& object) const
{
  // calculate rectangle where the object will move
  float x1 = dest.get_left();
  float x2 = dest.get_right();
  float y1 = dest.get_top();
  float y2 = dest.get_bottom();

  for(auto& solids : solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2));

    for(int x = test_tiles.left; x < test_tiles.right; ++x) {
      for(int y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const Tile* tile = solids->get_tile(x, y);
        if(!tile)
          continue;
        // skip non-solid tiles
        if(!tile->is_solid ())
          continue;
        Rectf tile_bbox = solids->get_tile_bbox(x, y);

        /* If the tile is a unisolid tile, the "is_solid()" function above
         * didn't do a thorough check. Calculate the position and (relative)
         * movement of the object and determine whether or not the tile is
         * solid with regard to those parameters. */
        if(tile->is_unisolid ()) {
          Vector relative_movement = movement
            - solids->get_movement(/* actual = */ true);

          if (!tile->is_solid (tile_bbox, object.get_bbox(), relative_movement))
            continue;
        } /* if (tile->is_unisolid ()) */

        if(tile->is_slope ()) { // slope tile
          AATriangle triangle;
          int slope_data = tile->getData();
          if (solids->get_drawing_effect() & VERTICAL_FLIP)
            slope_data = AATriangle::vertical_flip(slope_data);
          triangle = AATriangle(tile_bbox, slope_data);

          collision::rectangle_aatriangle(constraints, dest, triangle,
              solids->get_movement(/* actual = */ false));
        } else { // normal rectangular tile
          check_collisions(constraints, movement, dest, tile_bbox, NULL, NULL,
              solids->get_movement(/* actual = */ false));
        }
      }
    }
  }
}

uint32_t
Sector::collision_tile_attributes(const Rectf& dest, const Vector& mov) const
{
  float x1 = dest.p1.x;
  float y1 = dest.p1.y;
  float x2 = dest.p2.x;
  float y2 = dest.p2.y;

  uint32_t result = 0;
  for(auto& solids: solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2));
    // For ice (only), add a little fudge to recognize tiles Tux is standing on.
    Rect test_tiles_ice = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2 + SHIFT_DELTA));

    for(int x = test_tiles.left; x < test_tiles.right; ++x) {
      int y;
      for(y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const Tile* tile = solids->get_tile(x, y);
        if(!tile)
          continue;
        if ( tile->is_collisionful( solids->get_tile_bbox(x, y), dest, mov) ) {
          result |= tile->getAttributes();
        }
      }
      for(; y < test_tiles_ice.bottom; ++y) {
        const Tile* tile = solids->get_tile(x, y);
        if(!tile)
          continue;
        if ( tile->is_collisionful( solids->get_tile_bbox(x, y), dest, mov) ) {
          result |= (tile->getAttributes() & Tile::ICE);
        }
      }
    }
  }

  return result;
}

/** fills in CollisionHit and Normal vector of 2 intersecting rectangle */
static void get_hit_normal(const Rectf& r1, const Rectf& r2, CollisionHit& hit,
                           Vector& normal)
{
  float itop = r1.get_bottom() - r2.get_top();
  float ibottom = r2.get_bottom() - r1.get_top();
  float ileft = r1.get_right() - r2.get_left();
  float iright = r2.get_right() - r1.get_left();

  float vert_penetration = std::min(itop, ibottom);
  float horiz_penetration = std::min(ileft, iright);
  if(vert_penetration < horiz_penetration) {
    if(itop < ibottom) {
      hit.bottom = true;
      normal.y = vert_penetration;
    } else {
      hit.top = true;
      normal.y = -vert_penetration;
    }
  } else {
    if(ileft < iright) {
      hit.right = true;
      normal.x = horiz_penetration;
    } else {
      hit.left = true;
      normal.x = -horiz_penetration;
    }
  }
}

void
Sector::collision_object(MovingObject* object1, MovingObject* object2) const
{
  using namespace collision;

  const Rectf& r1 = object1->dest;
  const Rectf& r2 = object2->dest;

  CollisionHit hit;
  if(intersects(object1->dest, object2->dest)) {
    Vector normal;
    get_hit_normal(r1, r2, hit, normal);

    if(!object1->collides(*object2, hit))
      return;
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);
    if(!object2->collides(*object1, hit))
      return;
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);

    HitResponse response1 = object1->collision(*object2, hit);
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);
    HitResponse response2 = object2->collision(*object1, hit);
    if(response1 == CONTINUE && response2 == CONTINUE) {
      normal *= (0.5 + DELTA);
      object1->dest.move(-normal);
      object2->dest.move(normal);
    } else if (response1 == CONTINUE && response2 == FORCE_MOVE) {
      normal *= (1 + DELTA);
      object1->dest.move(-normal);
    } else if (response1 == FORCE_MOVE && response2 == CONTINUE) {
      normal *= (1 + DELTA);
      object2->dest.move(normal);
    }
  }
}

void
Sector::collision_static(collision::Constraints* constraints,
                         const Vector& movement, const Rectf& dest,
                         MovingObject& object)
{
  collision_tilemap(constraints, movement, dest, object);

  // collision with other (static) objects
  for(auto& moving_object : moving_objects) {
    if(moving_object->get_group() != COLGROUP_STATIC
       && moving_object->get_group() != COLGROUP_MOVING_STATIC)
      continue;
    if(!moving_object->is_valid())
      continue;

    if(moving_object != &object)
      check_collisions(constraints, movement, dest, moving_object->bbox,
                       &object, moving_object);
  }
}

void
Sector::collision_static_constrains(MovingObject& object)
{
  using namespace collision;
  float infinity = (std::numeric_limits<float>::has_infinity ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::max());

  Constraints constraints;
  Vector movement = object.get_movement();
  Vector pressure = Vector(0,0);
  Rectf& dest = object.dest;

  for(int i = 0; i < 2; ++i) {
    collision_static(&constraints, Vector(0, movement.y), dest, object);
    if(!constraints.has_constraints())
      break;

    // apply calculated horizontal constraints
    if(constraints.get_position_bottom() < infinity) {
      float height = constraints.get_height ();
      if(height < object.get_bbox().get_height()) {
        // we're crushed, but ignore this for now, we'll get this again
        // later if we're really crushed or things will solve itself when
        // looking at the vertical constraints
        pressure.y += object.get_bbox().get_height() - height;
      } else {
        dest.p2.y = constraints.get_position_bottom() - DELTA;
        dest.p1.y = dest.p2.y - object.get_bbox().get_height();
      }
    } else if(constraints.get_position_top() > -infinity) {
      dest.p1.y = constraints.get_position_top() + DELTA;
      dest.p2.y = dest.p1.y + object.get_bbox().get_height();
    }
  }
  if(constraints.has_constraints()) {
    if(constraints.hit.bottom) {
      dest.move(constraints.ground_movement);
    }
    if(constraints.hit.top || constraints.hit.bottom) {
      constraints.hit.left = false;
      constraints.hit.right = false;
      object.collision_solid(constraints.hit);
    }
  }

  constraints = Constraints();
  for(int i = 0; i < 2; ++i) {
    collision_static(&constraints, movement, dest, object);
    if(!constraints.has_constraints())
      break;

    // apply calculated vertical constraints
    float width = constraints.get_width ();
    if(width < infinity) {
      if(width + SHIFT_DELTA < object.get_bbox().get_width()) {
        // we're crushed, but ignore this for now, we'll get this again
        // later if we're really crushed or things will solve itself when
        // looking at the horizontal constraints
        pressure.x += object.get_bbox().get_width() - width;
      } else {
        float xmid = constraints.get_x_midpoint ();
        dest.p1.x = xmid - object.get_bbox().get_width()/2;
        dest.p2.x = xmid + object.get_bbox().get_width()/2;
      }
    } else if(constraints.get_position_right() < infinity) {
      dest.p2.x = constraints.get_position_right() - DELTA;
      dest.p1.x = dest.p2.x - object.get_bbox().get_width();
    } else if(constraints.get_position_left() > -infinity) {
      dest.p1.x = constraints.get_position_left() + DELTA;
      dest.p2.x = dest.p1.x + object.get_bbox().get_width();
    }
  }

  if(constraints.has_constraints()) {
    if( constraints.hit.left || constraints.hit.right
        || constraints.hit.top || constraints.hit.bottom
        || constraints.hit.crush )
      object.collision_solid(constraints.hit);
  }

  // an extra pass to make sure we're not crushed vertically
  if (pressure.y > 0) {
    constraints = Constraints();
    collision_static(&constraints, movement, dest, object);
    if(constraints.get_position_bottom() < infinity) {
      float height = constraints.get_height ();
      if(height + SHIFT_DELTA < object.get_bbox().get_height()) {
        CollisionHit h;
        h.top = true;
        h.bottom = true;
        h.crush = pressure.y > 16;
        object.collision_solid(h);
      }
    }
  }

  // an extra pass to make sure we're not crushed horizontally
  if (pressure.x > 0) {
    constraints = Constraints();
    collision_static(&constraints, movement, dest, object);
    if(constraints.get_position_right() < infinity) {
      float width = constraints.get_width ();
      if(width + SHIFT_DELTA < object.get_bbox().get_width()) {
        CollisionHit h;
        h.top = true;
        h.bottom = true;
        h.left = true;
        h.right = true;
        h.crush = pressure.x > 16;
        object.collision_solid(h);
      }
    }
  }
}

namespace {
const float MAX_SPEED = 16.0f;
}

void
Sector::handle_collisions()
{

  if (Editor::is_active()) {
    return;
    //Oběcts in editor shouldn't collide.
  }

  using namespace collision;

  // calculate destination positions of the objects
  for(auto moving_object : moving_objects) {
    Vector mov = moving_object->get_movement();

    // make sure movement is never faster than MAX_SPEED. Norm is pretty fat, so two addl. checks are done before.
    if (((mov.x > MAX_SPEED * M_SQRT1_2) || (mov.y > MAX_SPEED * M_SQRT1_2)) && (mov.norm() > MAX_SPEED)) {
      moving_object->movement = mov.unit() * MAX_SPEED;
      //log_debug << "Temporarily reduced object's speed of " << mov.norm() << " to " << moving_object->movement.norm() << "." << std::endl;
    }

    moving_object->dest = moving_object->get_bbox();
    moving_object->dest.move(moving_object->get_movement());
  }

  // part1: COLGROUP_MOVING vs COLGROUP_STATIC and tilemap
  for(auto& moving_object : moving_objects) {
    if((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC
        && moving_object->get_group() != COLGROUP_MOVING_ONLY_STATIC)
       || !moving_object->is_valid())
      continue;

    collision_static_constrains(*moving_object);
  }

  // part2: COLGROUP_MOVING vs tile attributes
  for(auto& moving_object : moving_objects) {
    if((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC
        && moving_object->get_group() != COLGROUP_MOVING_ONLY_STATIC)
       || !moving_object->is_valid())
      continue;

    uint32_t tile_attributes = collision_tile_attributes(moving_object->dest, moving_object->get_movement());
    if(tile_attributes >= Tile::FIRST_INTERESTING_FLAG) {
      moving_object->collision_tile(tile_attributes);
    }
  }

  // part2.5: COLGROUP_MOVING vs COLGROUP_TOUCHABLE
  for(auto& moving_object : moving_objects) {
    if((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC)
       || !moving_object->is_valid())
      continue;

    for(auto& moving_object_2 : moving_objects) {
      if(moving_object_2->get_group() != COLGROUP_TOUCHABLE
         || !moving_object_2->is_valid())
        continue;

      if(intersects(moving_object->dest, moving_object_2->dest)) {
        Vector normal;
        CollisionHit hit;
        get_hit_normal(moving_object->dest, moving_object_2->dest,
                       hit, normal);
        if(!moving_object->collides(*moving_object_2, hit))
          continue;
        if(!moving_object_2->collides(*moving_object, hit))
          continue;

        moving_object->collision(*moving_object_2, hit);
        moving_object_2->collision(*moving_object, hit);
      }
    }
  }

  // part3: COLGROUP_MOVING vs COLGROUP_MOVING
  for(auto i = moving_objects.begin(); i != moving_objects.end(); ++i) {
    MovingObject* moving_object = *i;

    if((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC)
       || !moving_object->is_valid())
      continue;

    for(auto i2 = i+1; i2 != moving_objects.end(); ++i2) {
      MovingObject* moving_object_2 = *i2;
      if((moving_object_2->get_group() != COLGROUP_MOVING
          && moving_object_2->get_group() != COLGROUP_MOVING_STATIC)
         || !moving_object_2->is_valid())
        continue;

      collision_object(moving_object, moving_object_2);
    }
  }

  // apply object movement
  for(auto& moving_object : moving_objects) {
    moving_object->bbox = moving_object->dest;
    moving_object->movement = Vector(0, 0);
  }
}

bool
Sector::is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid) const
{
  using namespace collision;

  for(auto& solids : solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(rect);

    for(int x = test_tiles.left; x < test_tiles.right; ++x) {
      for(int y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const Tile* tile = solids->get_tile(x, y);
        if(!tile) continue;
        if(!(tile->getAttributes() & Tile::SOLID))
          continue;
        if(tile->is_unisolid () && ignoreUnisolid)
          continue;
        if(tile->is_slope ()) {
          AATriangle triangle;
          Rectf tbbox = solids->get_tile_bbox(x, y);
          triangle = AATriangle(tbbox, tile->getData());
          Constraints constraints;
          if(!collision::rectangle_aatriangle(&constraints, rect, triangle))
            continue;
        }
        // We have a solid tile that overlaps the given rectangle.
        return false;
      }
    }
  }

  return true;
}

bool
Sector::is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object, const bool ignoreUnisolid) const
{
  using namespace collision;

  if (!is_free_of_tiles(rect, ignoreUnisolid)) return false;

  for(auto& moving_object : moving_objects) {
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if (moving_object->get_group() == COLGROUP_STATIC) {
      if(intersects(rect, moving_object->get_bbox())) return false;
    }
  }

  return true;
}

bool
Sector::is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object) const
{
  using namespace collision;

  if (!is_free_of_tiles(rect)) return false;

  for(auto moving_object : moving_objects) {
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if ((moving_object->get_group() == COLGROUP_MOVING)
        || (moving_object->get_group() == COLGROUP_MOVING_STATIC)
        || (moving_object->get_group() == COLGROUP_STATIC)) {
      if(intersects(rect, moving_object->get_bbox())) return false;
    }
  }

  return true;
}

bool
Sector::free_line_of_sight(const Vector& line_start, const Vector& line_end, const MovingObject* ignore_object) const
{
  using namespace collision;

  // check if no tile is in the way
  float lsx = std::min(line_start.x, line_end.x);
  float lex = std::max(line_start.x, line_end.x);
  float lsy = std::min(line_start.y, line_end.y);
  float ley = std::max(line_start.y, line_end.y);
  for (float test_x = lsx; test_x <= lex; test_x += 16) {
    for (float test_y = lsy; test_y <= ley; test_y += 16) {
      for(auto& solids : solid_tilemaps) {
        const Tile* tile = solids->get_tile_at(Vector(test_x, test_y));
        if(!tile) continue;
        // FIXME: check collision with slope tiles
        if((tile->getAttributes() & Tile::SOLID)) return false;
      }
    }
  }

  // check if no object is in the way
  for(const auto& moving_object : moving_objects) {
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if ((moving_object->get_group() == COLGROUP_MOVING)
        || (moving_object->get_group() == COLGROUP_MOVING_STATIC)
        || (moving_object->get_group() == COLGROUP_STATIC)) {
      if(intersects_line(moving_object->get_bbox(), line_start, line_end)) return false;
    }
  }

  return true;
}

bool
Sector::can_see_player(const Vector& eye) const
{
    const std::vector<Player*> players = get_players();
    for (auto& pl : players) {
      // test for free line of sight to any of all four corners and the middle of the player's bounding box
      if (free_line_of_sight(eye, pl->get_bbox().p1, pl)) return true;
      if (free_line_of_sight(eye, Vector(pl->get_bbox().p2.x, pl->get_bbox().p1.y), pl)) return true;
      if (free_line_of_sight(eye, pl->get_bbox().p2, pl)) return true;
      if (free_line_of_sight(eye, Vector(pl->get_bbox().p1.x, pl->get_bbox().p2.y), pl)) return true;
      if (free_line_of_sight(eye, pl->get_bbox().get_middle(), pl)) return true;
    }
    return false;
}


bool
Sector::add_smoke_cloud(const Vector& pos)
{
  add_object(std::make_shared<SmokeCloud>(pos));
  return true;
}

void
Sector::play_music(MusicType type)
{
  currentmusic = type;
  switch(currentmusic) {
    case LEVEL_MUSIC:
      SoundManager::current()->play_music(music);
      break;
    case HERRING_MUSIC:
      SoundManager::current()->play_music("music/invincible.ogg");
      break;
    case HERRING_WARNING_MUSIC:
      SoundManager::current()->stop_music(TUX_INVINCIBLE_TIME_WARNING);
      break;
    default:
      SoundManager::current()->play_music("");
      break;
  }
}

void
Sector::resume_music()
{
  if(SoundManager::current()->get_current_music() == music)
  {
    SoundManager::current()->resume_music(3.2f);
  }
  else
  {
    SoundManager::current()->stop_music();
    SoundManager::current()->play_music(music, true);
  }
}

MusicType
Sector::get_music_type() const
{
  return currentmusic;
}

int
Sector::get_total_badguys() const
{
  int total_badguys = 0;
  for(auto i = gameobjects.begin(); i != gameobjects.end(); ++i) {
    BadGuy* badguy = dynamic_cast<BadGuy*>(i->get());
    if (badguy && badguy->countMe)
      total_badguys++;
  }

  return total_badguys;
}

bool
Sector::inside(const Rectf& rect) const
{
  for(auto& solids : solid_tilemaps) {
    Rectf bbox = solids->get_bbox();
    bbox.p1.y = -INFINITY; // pretend the tilemap extends infinitely far upwards

    if (bbox.contains(rect))
      return true;
  }
  return false;
}

float
Sector::get_width() const
{
  float width = 0;
  for(auto& solids: solid_tilemaps) {
    width = std::max(width, solids->get_bbox().get_right());
  }

  return width;
}

float
Sector::get_height() const
{
  float height = 0;
  for(auto& solids: solid_tilemaps) {
    height = std::max(height, solids->get_bbox().get_bottom());
  }

  return height;
}

Size
Sector::get_editor_size() const
{
  // Find the solid tilemap with the greatest surface
  size_t max_surface = 0;
  Size size;
  for(const auto& solids: solid_tilemaps) {
    size_t surface = solids->get_width() * solids->get_height();
    if (surface > max_surface) {
      max_surface = surface;
      size = solids->get_size();
    }
  }

  return size;
}

void
Sector::resize_sector(Size& old_size, Size& new_size)
{
  for(const auto& object : gameobjects) {
    auto tilemap = dynamic_cast<TileMap*>(object.get());
    if (tilemap && tilemap->get_size() == old_size) {
      tilemap->resize(new_size);
    }
  }
}

void
Sector::change_solid_tiles(uint32_t old_tile_id, uint32_t new_tile_id)
{
  for(auto solids: solid_tilemaps) {
    solids->change_all(old_tile_id, new_tile_id);
  }
}

void
Sector::set_ambient_light(float red, float green, float blue)
{
  ambient_light.red = red;
  ambient_light.green = green;
  ambient_light.blue = blue;
}

float
Sector::get_ambient_red() const
{
  return ambient_light.red;
}

float
Sector::get_ambient_green() const
{
  return ambient_light.green;
}

float
Sector::get_ambient_blue() const
{
  return ambient_light.blue;
}

void
Sector::set_gravity(float gravity_)
{
  log_warning << "Changing a Sector's gravitational constant might have unforeseen side-effects" << std::endl;
  this->gravity = gravity_;
}

float
Sector::get_gravity() const
{
  return gravity;
}

Player*
Sector::get_nearest_player (const Vector& pos) const
{
  Player *nearest_player = NULL;
  float nearest_dist = std::numeric_limits<float>::max();

  std::vector<Player*> players = get_players();
  for (auto& this_player : players)
  {
    if (this_player->is_dying() || this_player->is_dead())
      continue;

    float this_dist = this_player->get_bbox ().distance(pos);

    if (this_dist < nearest_dist) {
      nearest_player = this_player;
      nearest_dist = this_dist;
    }
  }

  return nearest_player;
} /* Player *get_nearest_player */

std::vector<MovingObject*>
Sector::get_nearby_objects (const Vector& center, float max_distance) const
{
  std::vector<MovingObject*> ret;
  std::vector<Player*> players = Sector::current()->get_players();

  for (size_t i = 0; i < players.size (); i++) {
    float distance = players[i]->get_bbox ().distance (center);
    if (distance <= max_distance)
      ret.push_back (players[i]);
  }

  for (size_t i = 0; i < moving_objects.size (); i++) {
    float distance = moving_objects[i]->get_bbox ().distance (center);
    if (distance <= max_distance)
      ret.push_back (moving_objects[i]);
  }

  return (ret);
}

void
Sector::stop_looping_sounds()
{
  for(auto& object : gameobjects) {
    object->stop_looping_sounds();
  }
}

void Sector::play_looping_sounds()
{
  for(auto& object : gameobjects) {
    object->play_looping_sounds();
  }
}

void
Sector::save(Writer &writer)
{
  writer.start_list("sector", false);

  writer.write("name", name, false);
  writer.write("gravity", gravity);
  writer.write("ambient-light", ambient_light.toVector());

  if (init_script != "") {
    writer.write("init-script", init_script,false);
  }
  if (music != "") {
    writer.write("music", music, false);
  }

  // saving spawnpoints
  /*for(auto i = spawnpoints.begin(); i != spawnpoints.end(); ++i) {
    std::shared_ptr<SpawnPoint> spawny = *i;
    spawny->save(writer);
  }*/
  // Do not save spawnpoints since we have spawnpoint markers.

  // saving oběcts (not really)
  for(auto& obj : gameobjects) {
    if (obj->do_save()) {
      writer.start_list(obj->get_class());
      obj->save(writer);
      writer.end_list(obj->get_class());
    }
  }

  writer.end_list("sector");
}

/* vim: set sw=2 sts=2 et : */
/* EOF */
