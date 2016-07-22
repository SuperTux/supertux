//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_WORLDMAP_OBJECTS_HPP
#define HEADER_SUPERTUX_EDITOR_WORLDMAP_OBJECTS_HPP

#include "object/moving_sprite.hpp"
#include "video/color.hpp"
#include "worldmap/direction.hpp"

namespace worldmap_editor {

class WorldmapObject : public MovingSprite
{
  public:
    WorldmapObject(const ReaderMapping& lisp, const std::string& default_sprite);
    WorldmapObject(const ReaderMapping& lisp);
    WorldmapObject(const Vector& pos, const std::string& default_sprite);
    ~WorldmapObject();

    void update(float elapsed_time) {
      //No updates needed
    }

    void collision_solid(const CollisionHit& hit) {
      //This function wouldn't be called anyway.
    }

    HitResponse collision(GameObject& other, const CollisionHit& hit) {
      return FORCE_MOVE;
    }

    virtual std::string get_class() const {
      return "worldmap-object";
    }

    virtual void move_to(const Vector& pos);

    //virtual ObjectSettings get_settings();
    virtual void save(Writer& writer);
};

class LevelDot : public WorldmapObject
{
  public:
    LevelDot(const ReaderMapping& lisp);
    ~LevelDot();

    virtual std::string get_class() const {
      return "level";
    }

    void draw(DrawingContext& context);

    ObjectSettings get_settings();
    void save(Writer& writer);
    void after_editor_set();

  private:
    std::string level;
    std::string extro_script;
    bool auto_play;
    Color title_color;
};

class Teleporter : public WorldmapObject
{
  public:
    Teleporter(const ReaderMapping& lisp);
    ~Teleporter();

    virtual std::string get_class() const {
      return "teleporter";
    }

    void draw(DrawingContext& context);

    ObjectSettings get_settings();
    void save(Writer& writer);

  private:
    std::string worldmap;
    std::string spawnpoint;
    std::string message;
    bool automatic;
    bool change_worldmap;
};

class WorldmapSpawnPoint : public WorldmapObject
{
  public:
    WorldmapSpawnPoint(const ReaderMapping& lisp);
    WorldmapSpawnPoint(const std::string& name_, const Vector& pos);
    ~WorldmapSpawnPoint();

    virtual std::string get_class() const {
      return "worldmap-spawnpoint";
    }

    ObjectSettings get_settings();
    void save(Writer& writer);

  private:
    worldmap::Direction dir;
};

class SpriteChange : public WorldmapObject
{
  public:
    SpriteChange(const ReaderMapping& lisp);
    ~SpriteChange();

    virtual std::string get_class() const {
      return "sprite-change";
    }

    ObjectSettings get_settings();
    void save(Writer& writer);

  private:
    std::string target_sprite;
    std::string stay_action;
    bool initial_stay_action;
    std::string stay_group;
    bool change_on_touch;
};

class SpecialTile : public WorldmapObject
{
  public:
    SpecialTile(const ReaderMapping& lisp);
    ~SpecialTile();

    virtual std::string get_class() const {
      return "special-tile";
    }

    ObjectSettings get_settings();
    void save(Writer& writer);

  private:
    std::string map_message;
    std::string script;
    bool passive_message;
    bool invisible_tile;
    worldmap::Direction apply_to_direction;
};

} // namespace worldmap_editor

#endif // HEADER_SUPERTUX_EDITOR_WORLDMAP_OBJECTS_HPP

/* EOF */
