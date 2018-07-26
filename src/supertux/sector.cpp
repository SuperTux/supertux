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
#include <utility>
#include <cmath>

#include "scripting/scripting.hpp"
#include "scripting/squirrel_util.hpp"
#include "scripting/sector.hpp"
#include <physfs.h>

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "math/broadphase.hpp"
#include "math/spatial_hashing.hpp"
#include "math/collision_graph.hpp"
#include "math/aabb_polygon.hpp"
#include "math/rect.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/display_effect.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/smoke_cloud.hpp"
#include "object/text_object.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "scripting/sector.hpp"
#include "supertux/collision.hpp"
#include "supertux/constants.hpp"
#include "supertux/game_session.hpp"
#include "supertux/level.hpp"
#include "supertux/savegame.hpp"
#include "supertux/spawn_point.hpp"
#include "supertux/tile.hpp"
#include "util/file_system.hpp"
#include "util/writer.hpp"

Sector* Sector::_current = 0;

bool Sector::show_collrects = false;
bool Sector::draw_solids_only = false;

Sector::Sector(Level* parent) :
  level(parent),
  name(),
  bullets(),
  platforms(),
  init_script(),
  gameobjects_new(),
  currentmusic(LEVEL_MUSIC),
  sector_table(),
  scripts(),
  ambient_light(1.0f, 1.0f, 1.0f, 1.0f ),
  ambient_light_fading(false),
  source_ambient_light(1.0f, 1.0f, 1.0f, 1.0f),
  target_ambient_light(1.0f, 1.0f, 1.0f, 1.0f),
  ambient_light_fade_duration(0.0f),
  ambient_light_fade_accum(0.0f),
  foremost_layer(),
  gameobjects(),
  moving_objects(),
  spawnpoints(),
  portables(),
  music(),
  gravity(10.0),
  colgraph(),
  player(0),
  solid_tilemaps(),
  camera(0),
  effect(0),
  broadphase(nullptr)
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

  release_scripts(global_vm, scripts, sector_table);

  update_game_objects();
  assert(gameobjects_new.size() == 0);

  for(const auto& object: gameobjects) {
    before_object_remove(object);
  }
}

Level*
Sector::get_level() const
{
  return level;
}

HSQUIRRELVM
Sector::run_script(const std::string& script, const std::string& sourcename)
{
  if(script.empty())
  {
    return NULL;
  }
  std::istringstream stream(script);
  return run_script(stream, sourcename);
}

HSQUIRRELVM
Sector::run_script(std::istream& in, const std::string& sourcename)
{
  try {
    return scripting::run_script(in, "Sector " + name + " - " + sourcename,
                                 scripts, &sector_table);
  }
  catch(const std::exception& e)
  {
    log_warning << "Error running sector script: " << e.what() << std::endl;
    return NULL;
  }
}

void
Sector::add_object(GameObjectPtr object)
{
  // make sure the object isn't already in the list
#ifndef NDEBUG
  for(const auto& game_object : gameobjects) {
    assert(game_object != object);
  }
  for(const auto& gameobject : gameobjects_new) {
    assert(gameobject != object);
  }
#endif

  gameobjects_new.push_back(object);
}

void
Sector::activate(const std::string& spawnpoint)
{
  std::shared_ptr<SpawnPoint> sp;
  for(const auto& spawn_point : spawnpoints) {
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
    scripting::store_object(vm, "sector", sector_table);
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
    run_script(init_script, "init-script");
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
  scripting::delete_table_entry(vm, "sector");
  sq_pop(vm, 1);

  for(const auto& object: gameobjects) {
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
  for(const auto& obj : gameobjects)
  {
    const auto& tm = dynamic_cast<TileMap*>(obj.get());
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

  if(ambient_light_fading)
  {
    ambient_light_fade_accum += elapsed_time;
    float percent_done = ambient_light_fade_accum / ambient_light_fade_duration * 1.0f;
    float r = (1.0f - percent_done) * source_ambient_light.red + percent_done * target_ambient_light.red;
    float g = (1.0f - percent_done) * source_ambient_light.green + percent_done * target_ambient_light.green;
    float b = (1.0f - percent_done) * source_ambient_light.blue + percent_done * target_ambient_light.blue;

    if(r > 1.0)
      r = 1.0;
    if(g > 1.0)
      g = 1.0;
    if(b > 1.0)
      b = 1.0;

    if(r < 0)
      r = 0;
    if(g < 0)
      g = 0;
    if(b < 0)
      b = 0;

    ambient_light = Color(r, g, b);

    if(ambient_light_fade_accum >= ambient_light_fade_duration)
    {
      ambient_light = target_ambient_light;
      ambient_light_fading = false;
      ambient_light_fade_accum = 0;
    }
  }

  /* update objects */
  for(const auto& object : gameobjects) {
    if(!object->is_valid())
      continue;

    object->update(elapsed_time);
  }

  /* Handle all possible collisions. */
  for(int i = 0;i<4;i++)
  handle_collisions();
  update_game_objects();
}

void
Sector::update_game_objects()
{
  /** cleanup marked objects */
  for(auto i = gameobjects.begin();
      i != gameobjects.end(); /* nothing */) {
    const GameObjectPtr& object = *i;

    if(object->is_valid()) {
      ++i;
      continue;
    }

    before_object_remove(object);

    i = gameobjects.erase(i);
  }

  /* add newly created objects */
  for(const auto& object : gameobjects_new)
  {
    before_object_add(object);

    gameobjects.push_back(object);
  }
  gameobjects_new.clear();

  /* update solid_tilemaps list */
  //FIXME: this could be more efficient
  solid_tilemaps.clear();
  for(const auto& obj : gameobjects)
  {
    const auto& tm = dynamic_cast<TileMap*>(obj.get());
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
    // Tell CollisionEngine about this object
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
    if(camera != 0) {
      log_warning << "Multiple cameras added. Ignoring" << std::endl;
      return false;
    }
    camera = camera_;
  }

  auto player_ = dynamic_cast<Player*>(object.get());
  if(player_) {
    if(player != 0) {
      log_warning << "Multiple players added. Ignoring" << std::endl;
      return false;
    }
    player = player_;
  }

  auto effect_ = dynamic_cast<DisplayEffect*>(object.get());
  if(effect_) {
    if(effect != 0) {
      log_warning << "Multiple DisplayEffects added. Ignoring" << std::endl;
      return false;
    }
    effect = effect_;
  }

  if(_current == this) {
    try_expose(object);
  }

  return true;
}

void
Sector::try_expose(GameObjectPtr object)
{
  scripting::try_expose(object, sector_table);
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
  auto portable = dynamic_cast<Portable*>(object.get());
  if (portable) {
    portables.erase(std::find(portables.begin(), portables.end(), portable));
  }
  auto bullet = dynamic_cast<Bullet*>(object.get());
  if (bullet) {
    bullets.erase(std::find(bullets.begin(), bullets.end(), bullet));
  }
  auto moving_object = dynamic_cast<MovingObject*>(object.get());
  if (moving_object) {
    moving_objects.erase(
      std::find(moving_objects.begin(), moving_objects.end(), moving_object));
    // Tell Collision Engine that this object has been removed
    if (broadphase)
      broadphase->remove(moving_object);

  }

  if(_current == this)
    try_unexpose(object);
}

void
Sector::try_unexpose(GameObjectPtr object)
{
  scripting::try_unexpose(object, sector_table);
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

  for(const auto& object : gameobjects) {
    if(!object->is_valid())
      continue;

    if (draw_solids_only)
    {
      auto tm = dynamic_cast<TileMap*>(object.get());
      if (tm && !tm->is_solid())
        continue;
    }

    object->draw(context);
  }

  if(show_collrects) {
    Color color(1.0f, 0.0f, 0.0f, 0.75f);
    for(auto& object : moving_objects) {
      const Rectf& rect = object->get_bbox();

      context.color().draw_filled_rect(rect, color, LAYER_FOREGROUND1 + 10);
    }
  }

  context.pop_transform();
}

void
Sector::on_window_resize()
{
  for(const auto& obj : gameobjects)
  {
    obj->on_window_resize();
  }
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

  auto moving_object = dynamic_cast<MovingObject*> (object);
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

  constraints->ground_movement = other_movement;
  if(other != NULL && object != NULL) {
    HitResponse response = other->collision(*object, dummy);
    if(response == ABORT_MOVE)
      return;

    if(other->get_movement() != Vector(0, 0)) {
      // TODO what todo when we collide with 2 moving objects?!?
      constraints->ground_movement = other->get_movement();
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
                          const Vector& movement, Rectf& dest,
                          MovingObject& object, std::vector<Manifold>& contacts, bool slope_adjust_x) const
{
  using namespace collision;
  // calculate rectangle where the object will move
  float x1 = dest.get_left();
  float x2 = dest.get_right();
  float y1 = dest.get_top();
  float y2 = dest.get_bottom();
  std::set< CollisionHit > hits; // Use a set (intentionally => do not generate CollisionHit twice)
  for(const auto& solids : solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2));

    for(int x = test_tiles.left; x < test_tiles.right; ++x) {
      for(int y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const auto& tile = solids->get_tile(x, y);
        if( !tile)
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
        }

        bool use_aabbpoly = !tile->is_slope();
        int dir[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (const auto& offset : dir) {
          use_aabbpoly &= !solids->get_tile(x+offset[0], y+offset[1])->is_slope();
        }
        CollisionHit h;
        Vector overlapV(0, 0);
        Manifold m;
        /*if(use_aabbpoly) {
          AABBPolygon mobjp(dest);
          std::shared_ptr<AABBPolygon> tilepoly(new AABBPolygon(tile_bbox));
          for (const auto& offset : dir) {
            const auto& nb = solids->get_tile(x+offset[0], y+offset[1]);
            Rectf nb_bbox = solids->get_tile_bbox(x+offset[0], y+offset[1]);
            if (!nb->is_solid())
              continue;
            if (nb->is_unisolid()) {
                Vector relative_movement = movement
                  - solids->get_movement( true);

                if (!nb->is_solid (nb_bbox, object.get_bbox(), relative_movement))
                  continue;
              }
              tilepoly->process_neighbor(offset[0], offset[1]);
          }

          mobjp.handle_collision(*tilepoly.get(), m);
          if (!m.collided)
            continue;
          // log_debug << m.depth << " " << m.normal.x << " " << m.normal.y <<std::endl;
          overlapV = Vector(m.normal.x*m.depth, m.normal.y*m.depth);
          if(tile->is_slope())
          {
            overlapV.y = overlapV.y + (overlapV.x*overlapV.x)/(overlapV.y);
            overlapV.x = 0;
          }
          if (std::max(std::abs(overlapV.x), std::abs(overlapV.y))
              == std::abs(overlapV.x)) {
            h.right = overlapV.x > 0;
            h.left =  overlapV.x < 0;
          } else {
            h.bottom = overlapV.y > 0;
            h.top    = overlapV.y < 0;
          }
          m.normal = overlapV;
          m.depth = 1;
          // Check if they overlap
          std::swap(h.top, h.bottom);
          std::swap(h.right, h.left);
          if ((h.bottom || h.top || h.left || h.right)) {
              if(slope_adjust_x)
                  dest.move(overlapV);
            object.collision_solid(h);
            contacts.push_back(m);
          }

            continue;
        }*/

        // Do collision response
        //AABBPolygon mobjp(dest); // = dest.to_polygon();
        Polygon mobjp = dest.to_polygon();
        Polygon tile_poly = tile->tile_to_poly(tile_bbox);
        for (const auto& offset : dir) {
          const auto& nb = solids->get_tile(x+offset[0], y+offset[1]);
          Rectf nb_bbox = solids->get_tile_bbox(x+offset[0], y+offset[1]);
          if (!nb->is_solid())
            continue;
          if (nb->is_unisolid()) {
              Vector relative_movement = movement
                - solids->get_movement(/* actual = */ true);

              if (!nb->is_solid (nb_bbox, object.get_bbox(), relative_movement))
                continue;
            }

          Polygon npoly = tile->tile_to_poly(nb_bbox);
          if(!tile->is_slope())
          tile_poly.process_neighbor(npoly);
        }
        mobjp.handle_collision(tile_poly, m);
        if (!m.collided)
          continue;
        if(slope_adjust_x)
          dest.move(solids->get_movement(false));
        overlapV = Vector(m.normal.x*m.depth, m.normal.y*m.depth);
        if (tile->is_slope()) {
            if(overlapV.y != 0)
            {
              // Only adjust on y axis.
              overlapV.y = overlapV.y + (overlapV.x*overlapV.x)/(overlapV.y);
              overlapV.x = 0;
            }else{
                continue;
            }
          Rectf tbbox = solids->get_tile_bbox(x, y);
          AATriangle triangle = AATriangle(tbbox, tile->getData());
          auto rect = dest;

          Vector normal;
            float c = 0.0;
            Vector p1;
            Rectf area;
            switch (triangle.dir & AATriangle::DEFORM_MASK) {
              case 0:
                area.p1 = triangle.bbox.p1;
                area.p2 = triangle.bbox.p2;
                break;
              case AATriangle::DEFORM_BOTTOM:
                area.p1 = Vector(triangle.bbox.p1.x, triangle.bbox.p1.y + triangle.bbox.get_height()/2);
                area.p2 = triangle.bbox.p2;
                break;
              case AATriangle::DEFORM_TOP:
                area.p1 = triangle.bbox.p1;
                area.p2 = Vector(triangle.bbox.p2.x, triangle.bbox.p1.y + triangle.bbox.get_height()/2);
                break;
              case AATriangle::DEFORM_LEFT:
                area.p1 = triangle.bbox.p1;
                area.p2 = Vector(triangle.bbox.p1.x + triangle.bbox.get_width()/2, triangle.bbox.p2.y);
                break;
              case AATriangle::DEFORM_RIGHT:
                area.p1 = Vector(triangle.bbox.p1.x + triangle.bbox.get_width()/2, triangle.bbox.p1.y);
                area.p2 = triangle.bbox.p2;
                break;
              default:
                assert(false);
            }

            switch(triangle.dir & AATriangle::DIRECTION_MASK) {
              case AATriangle::SOUTHWEST:
                p1 = Vector(rect.p1.x, rect.p2.y);
                makePlane(area.p1, area.p2, normal, c);
                break;
              case AATriangle::NORTHEAST:
                p1 = Vector(rect.p2.x, rect.p1.y);
                makePlane(area.p2, area.p1, normal, c);
                break;
              case AATriangle::SOUTHEAST:
                p1 = rect.p2;
              makePlane(Vector(area.p1.x, area.p2.y),
                          Vector(area.p2.x, area.p1.y), normal, c);
                break;
              case AATriangle::NORTHWEST:
                p1 = rect.p1;
                makePlane(Vector(area.p2.x, area.p1.y),
                          Vector(area.p1.x, area.p2.y), normal, c);
                break;
              default:
                assert(false);
            }

          h.slope_normal = normal;
        }
        if (std::max(std::abs(overlapV.x), std::abs(overlapV.y))
            == std::abs(overlapV.x)) {
          h.right = overlapV.x > 0;
          h.left =  overlapV.x < 0;
        } else {
          h.bottom = overlapV.y > 0;
          h.top    = overlapV.y < 0;
        }
        // Check if they overlap
        std::swap(h.top, h.bottom);
        std::swap(h.right, h.left);
        if ((h.bottom || h.top || h.left || h.right)) {
            if(slope_adjust_x)
            {
                dest.move(overlapV);
            }
            else
            {
              hits.insert(h);
              contacts.push_back(m);
            }
        }
        }
      }
  }
  // After removing duplicates (done by set automatically),
  // inform object about collisions
  for (const auto& h : hits) {
    object.collision_solid(h);
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
  for (auto& solids : solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2));
    // For ice (only), add a little fudge to recognize tiles Tux is standing on.
    Rect test_tiles_ice = solids->get_tiles_overlapping(Rectf(x1, y1, x2,
                                                      y2 + SHIFT_DELTA));

    for (int x = test_tiles.left; x < test_tiles.right; ++x) {
      int y;
      for (y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const auto& tile = solids->get_tile(x, y);
        if (!tile)
          continue;
        if (tile->is_collisionful(solids->get_tile_bbox(x, y), dest, mov)) {
          result |= tile->getAttributes();
        }
      }
      for (; y < test_tiles_ice.bottom; ++y) {
        const auto& tile = solids->get_tile(x, y);
        if (!tile)
          continue;
        if ( tile->is_collisionful(solids->get_tile_bbox(x, y), dest, mov) ) {
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
Sector::collision_object(MovingObject* object1, MovingObject* object2, collision_graph& graph) const
{
  using namespace collision;

  const Rectf& r1 = object1->dest;
  const Rectf& r2 = object2->dest;

  CollisionHit hit;
  if (intersects(object1->dest, object2->dest)) {
    Vector normal;
    get_hit_normal(r1, r2, hit, normal);

    if (!object1->collides(*object2, hit))
      return;
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);
    if (!object2->collides(*object1, hit))
      return;
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);

    HitResponse response1 = object1->collision(*object2, hit);
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);
    HitResponse response2 = object2->collision(*object1, hit);
    graph.register_collision_hit(hit, object1, object2);
    if (response1 == CONTINUE && response2 == CONTINUE) {
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

void Sector::collision_moving_static(const Vector& movement, Rectf& dest,
MovingObject& object, collision_graph& graph, std::vector<Manifold>& contacts)
{
  std::set< CollisionHit > colhits;
  for (auto& moving_object : moving_objects) {

    if (moving_object->get_group() != COLGROUP_STATIC
       && moving_object->get_group() != COLGROUP_MOVING_STATIC)
      continue;
    if (!moving_object->is_valid())
      continue;

    if (moving_object != &object) {
      // First check if rectfs intersect
      if (!collision::intersects(dest, moving_object->get_bbox()))
        continue;
      Polygon mobjp = dest.to_polygon();
      Polygon tile_poly = moving_object->get_bbox().to_polygon();
      Manifold m;
      CollisionHit h;
      std::set< MovingObject* > possible_neighbours;
      broadphase->search(moving_object->get_bbox().grown(10), []{},
                  possible_neighbours);
      for (const auto& mobject : possible_neighbours) {
        // TODO Specail case: Same object on multiple layers
        // => detect collision (?) use contacts?
        if (mobject->get_bbox() == object.get_bbox() || mobject->get_bbox() == moving_object->get_bbox())
          continue;
        Polygon mobject_poly = mobject->get_bbox().to_polygon();
        tile_poly.process_neighbor(mobject_poly);
        tile_poly.debug();
        mobject_poly.debug();
      }
      // TODO(christ2go) Take static tilemap into account.
      mobjp.handle_collision(tile_poly, m);
      if (!m.collided)
        continue;
      Vector overlapV = m.normal*m.depth;
      h.right = h.left = h.bottom = h.top = false;
       if (std::max(std::abs(overlapV.x), std::abs(overlapV.y))
          == std::abs(overlapV.x)) {
        h.right = overlapV.x > 0;
        h.left =  overlapV.x < 0;
      } else {
        h.bottom = overlapV.y > 0;
        h.top    = overlapV.y < 0;
        // log_debug << "*** TOP OR BOT "<< std::endl << m.normal.x << " " << m.normal.y << std::endl;
      }
      moving_object->collision(object, h);
      std::swap(h.top, h.bottom);
      std::swap(h.right, h.left);

      m.normal = m.normal;
      m.depth = m.depth;
      colhits.insert(h);
      contacts.push_back(m);
      // Insert into collision graph
      graph.register_collision_hit(h, &object, moving_object);
      }

    }
    for(const auto& hit : colhits)
      object.collision_solid(hit);
}
void
Sector::collision_static(collision::Constraints* constraints,
                         const Vector& movement, Rectf& dest,
                         MovingObject& object, collision_graph& graph) {
  std::vector< Manifold > contacts;
  std::vector< Manifold > all_contacts;
  // Always resolve biggest
  for (int i = 0;i<2;i++) {
    collision_tilemap(constraints, movement, dest, object, contacts, i != 0);
  }
  contacts.clear();
  collision_moving_static(movement, dest, object, graph, contacts);
  for (const auto& m : contacts) {
    Vector overlapV((m.depth*m.normal.x)/static_cast<double>(contacts.size()),
                  (m.depth*m.normal.y)/(static_cast<double>(contacts.size())));
    dest.move(overlapV);
  }
  double extend_left = 0.0f,
         extend_right = 0.0f,
         extend_top = 0.0f,
         extend_bot = 0.0f;
  all_contacts.clear();
  collision_tilemap(constraints, movement, dest, object, all_contacts, false);
  collision_moving_static(movement, dest, object, graph, all_contacts);

  for (const auto& m : all_contacts) {
    Vector v (m.normal.x*m.depth, m.normal.y*m.depth);
    log_debug << "v is " << v.x << " " << v.y << std::endl;
    if (v.x < 0 && std::abs(v.x) > std::abs(extend_left)) {
      extend_left = std::abs(v.x);
    }
    if (v.x > 0 && std::abs(v.x) > std::abs(extend_right)) {
      extend_right = std::abs(v.x);
    }
    if (v.y < 0 && std::abs(v.y) > std::abs(extend_top)) {
      extend_top = std::abs(v.y);
    }
    if (v.y > 0 && std::abs(v.y) > std::abs(extend_bot)) {
      extend_bot = std::abs(v.y);
    }
  }
  if (extend_top > 4 && extend_bot > 4) {
    CollisionHit h;
    h.crush = h.top = h.bottom = h.left = h.right = true;
    object.collision_solid(h);
  }
  if (extend_left > 8 && extend_right > 8) {
    CollisionHit h;
    h.crush = h.left = h.bottom = h.top = h.right = true;
    object.collision_solid(h);
  }
}
void
Sector::collision_static_constrains(MovingObject& object, collision_graph& graph)
{

  using namespace collision;
  Constraints constraints;
  Vector movement = object.get_movement();
  Rectf& dest = object.dest;

  collision_static(&constraints, Vector(movement.x, movement.y),
                  dest, object, graph);
}

namespace {
const float MAX_SPEED = 16.0f;
}

void
Sector::handle_collisions()
{
  if (Editor::is_active()) {
    return;
    // Oběcts in editor shouldn't collide.
  }

  using namespace collision;
  if (!broadphase)
  {
    broadphase.reset(new spatial_hashing(get_width(), get_height()));
    log_debug << "Error :: Reset" << std::endl;
  }
  const int pixeld_x = 2;
  const int pixeld_y = 1;
  // calculate destination positions of the objects
  for (const auto& moving_object : moving_objects) {
    Vector mov = moving_object->get_movement();

    // make sure movement is never faster than MAX_SPEED. Norm is pretty fat, so two addl. checks are done before.
    if (((mov.x > MAX_SPEED * M_SQRT1_2) || (mov.y > MAX_SPEED * M_SQRT1_2)) && (mov.norm() > MAX_SPEED)) {
      moving_object->movement = mov.unit() * MAX_SPEED;
      //log_debug << "Temporarily reduced object's speed of " << mov.norm() << " to " << moving_object->movement.norm() << "." << std::endl;
    }

    moving_object->dest = moving_object->get_bbox();
    moving_object->dest.move(moving_object->get_movement());
  }
  for (const auto& mobj : moving_objects) {
    if (!(mobj->get_group() != COLGROUP_MOVING
        && mobj->get_group() != COLGROUP_MOVING_STATIC
        && mobj->get_group() != COLGROUP_MOVING_ONLY_STATIC))
    {
      mobj->dest =   mobj->dest.grown_xy(-pixeld_x, -pixeld_y);
    }

  }

  // part 0: Handle moving objects
  // Get a list of all objects which move
  platforms.clear();
  colgraph.reset();
  for (const auto& moving_object : moving_objects) {
    // Check for correct collision group and actual movement in last frame
    if (moving_object->get_group() == COLGROUP_STATIC &&
      moving_object->get_movement() != Vector(0, 0))
      platforms.insert(moving_object);
  }
  // part1: COLGROUP_MOVING vs COLGROUP_STATIC and tilemap
  for (const auto& obj : moving_objects) {
    broadphase->insert(obj->dest, obj);
  }

  for (const auto& moving_object : moving_objects) {
    if ((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC
        && moving_object->get_group() != COLGROUP_MOVING_ONLY_STATIC)
       || !moving_object->is_valid())
      continue;

    collision_static_constrains(*moving_object, colgraph);
  }
  // part2: COLGROUP_MOVING vs tile attributes
  for (const auto& moving_object : moving_objects) {
    if ((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC
        && moving_object->get_group() != COLGROUP_MOVING_ONLY_STATIC)
       || !moving_object->is_valid())
      continue;

    uint32_t tile_attributes = collision_tile_attributes(moving_object->dest,
                                            moving_object->get_movement());
    if (tile_attributes >= Tile::FIRST_INTERESTING_FLAG) {
      moving_object->collision_tile(tile_attributes);
    }
  }

  // part2.5: COLGROUP_MOVING vs COLGROUP_TOUCHABLE
  for (const auto& moving_object : moving_objects) {
    if ((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC)
       || !moving_object->is_valid())
      continue;
    std::set< MovingObject* > possibleCollisions;
    broadphase->search(moving_object->dest.grown(4), []{} , possibleCollisions);
    for (auto& moving_object_2 : possibleCollisions) {
      if (moving_object_2 == moving_object)
        continue;
      if (moving_object_2->get_group() != COLGROUP_TOUCHABLE
         || !moving_object_2->is_valid())
        continue;

      if (intersects(moving_object->dest, moving_object_2->dest)) {
        Vector normal;
        CollisionHit hit;
        get_hit_normal(moving_object->dest, moving_object_2->dest,
                       hit, normal);
        if (!moving_object->collides(*moving_object_2, hit))
          continue;
        if (!moving_object_2->collides(*moving_object, hit))
          continue;
        moving_object->collision(*moving_object_2, hit);
        moving_object_2->collision(*moving_object, hit);

        broadphase->insert(moving_object->dest, moving_object);
        broadphase->insert(moving_object_2->dest, moving_object_2);
      }
    }
  }
  // part3: COLGROUP_MOVING vs COLGROUP_MOVING
  for (auto i = moving_objects.begin(); i != moving_objects.end(); ++i) {
    auto moving_object = *i;

    if ((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC)
       || !moving_object->is_valid())
      continue;
    // Query the broadphase
    std::set< MovingObject* > possibleCollisions;
    broadphase->search(moving_object->dest.grown(4), []{} , possibleCollisions);
    for (auto i2 = possibleCollisions.begin(); i2 != possibleCollisions.end(); ++i2)
    {
      auto moving_object_2 = *i2;
      if (moving_object_2 == moving_object)
        continue;
      if ((moving_object_2->get_group() != COLGROUP_MOVING
          && moving_object_2->get_group() != COLGROUP_MOVING_STATIC)
         || !moving_object_2->is_valid())
        continue;

      collision_object(moving_object, moving_object_2, colgraph);

      // Update the objects positions
      broadphase->insert(moving_object->dest, moving_object);
      broadphase->insert(moving_object_2->dest, moving_object_2);
    }
  }
  for (const auto& plf : platforms) {
    std::vector< MovingObject* > children;
    colgraph.directional_hull(plf, 0 /** 0 is direction top */, children);
    log_debug << "Hull has " << children.size() << " elements." << std::endl;
    for (const auto& child : children) {
        child->collision_parent = plf;
        child->parent_updated = true;
        child->dest.move(plf->get_movement());
    }
  }
  // apply object movement
  for (const auto& moving_object : moving_objects) {

    moving_object->movement = Vector(0, 0);
    if (!moving_object->parent_updated) {
        moving_object->collision_parent = NULL;
    }
    moving_object->parent_updated = false;
    if (!(moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC
        && moving_object->get_group() != COLGROUP_MOVING_ONLY_STATIC))
    {
      moving_object->dest =   moving_object->dest.grown_xy(pixeld_x, pixeld_y);
    }
    moving_object->bbox = moving_object->dest;
  }

}

bool
Sector::is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid) const
{
  using namespace collision;

  for(const auto& solids : solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(rect.grown(-1));

    for(int x = test_tiles.left; x < test_tiles.right; ++x) {
      for(int y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const auto& tile = solids->get_tile(x, y);
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
          if(!collision::rectangle_aatriangle(&constraints, rect.grown(-1), triangle))
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

  if (!is_free_of_tiles(rect.grown(-2), ignoreUnisolid)) return false;

  for(const auto& moving_object : moving_objects) {
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if (moving_object->get_group() == COLGROUP_STATIC) {
      if(intersects(rect.grown(-2), moving_object->get_bbox())) return false;
    }
  }

  return true;
}

bool
Sector::is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object) const
{
  using namespace collision;

  if (!is_free_of_tiles(rect)) return false;

  for(const auto& moving_object : moving_objects) {
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if ((moving_object->get_group() == COLGROUP_MOVING)
        || (moving_object->get_group() == COLGROUP_MOVING_STATIC)
        || (moving_object->get_group() == COLGROUP_STATIC)) {
      if(intersects(rect.grown(-1), moving_object->get_bbox().grown(-1))) return false;
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
      for(const auto& solids : solid_tilemaps) {
        const auto& tile = solids->get_tile_at(Vector(test_x, test_y));
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
    for (const auto& pl : players) {
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
  for(const auto& object : gameobjects) {
    auto badguy = dynamic_cast<BadGuy*>(object.get());
    if (badguy && badguy->countMe)
      total_badguys++;
  }

  return total_badguys;
}

bool
Sector::inside(const Rectf& rect) const
{
  for(const auto& solids : solid_tilemaps) {
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
  for(const auto& solids: solid_tilemaps) {
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
Sector::resize_sector(const Size& old_size, const Size& new_size, const Size& resize_offset)
{
  bool is_offset = resize_offset.width || resize_offset.height;
  Vector obj_shift = Vector(resize_offset.width * 32, resize_offset.height * 32);
  for(const auto& object : gameobjects) {
    auto tilemap = dynamic_cast<TileMap*>(object.get());
    if (tilemap) {
      if (tilemap->get_size() == old_size) {
        tilemap->resize(new_size, resize_offset);
      } else if (is_offset) {
        tilemap->move_by(obj_shift);
      }
    } else if (is_offset) {
      auto moving_object = dynamic_cast<MovingObject*>(object.get());
      if (moving_object) {
        moving_object->move_to(moving_object->get_pos() + obj_shift);
      }
    }
  }
}

void
Sector::change_solid_tiles(uint32_t old_tile_id, uint32_t new_tile_id)
{
  for(auto& solids: solid_tilemaps) {
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

void
Sector::fade_to_ambient_light(float red, float green, float blue, float seconds)
{
  if(seconds == 0)
  {
    ambient_light = Color(red, green, blue);
    return;
  }

  ambient_light_fading = true;
  ambient_light_fade_accum = 0;
  ambient_light_fade_duration = seconds;
  source_ambient_light = ambient_light;
  target_ambient_light = Color(red, green, blue);
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
  gravity = gravity_;
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

  for(const auto& player_ : players) {
    float distance = player_->get_bbox().distance(center);
    if (distance <= max_distance)
      ret.push_back(player_);
  }

  for (const auto& object_ : moving_objects) {
    float distance = object_->get_bbox().distance(center);
    if (distance <= max_distance)
      ret.push_back(object_);
  }

  return ret;
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
  for(const auto& object : gameobjects) {
    object->play_looping_sounds();
  }
}

void
Sector::save(Writer &writer)
{
  writer.start_list("sector", false);

  writer.write("name", name, false);
  writer.write("ambient-light", ambient_light.toVector());

  if (init_script.size()) {
    writer.write("init-script", init_script,false);
  }
  if (music.size()) {
    writer.write("music", music, false);
  }

  if (!Editor::is_active() || !Editor::current()->get_worldmap_mode()) {
    writer.write("gravity", gravity);
  }

  // saving spawnpoints
  /*for(auto i = spawnpoints.begin(); i != spawnpoints.end(); ++i) {
    std::shared_ptr<SpawnPoint> spawny = *i;
    spawny->save(writer);
  }*/
  // Do not save spawnpoints since we have spawnpoint markers.

  // saving oběcts (not really)
  for(auto& obj : gameobjects) {
    if (obj->is_saveable()) {
      writer.start_list(obj->get_class());
      obj->save(writer);
      writer.end_list(obj->get_class());
    }
  }

  writer.end_list("sector");
}

/* vim: set sw=2 sts=2 et : */
/* EOF */
