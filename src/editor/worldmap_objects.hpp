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

    HitResponse collision(GameObject& other, const CollisionHit& hit) override {
      return FORCE_MOVE;
    }

    virtual std::string get_class() const override {
      return "worldmap-object";
    }

    virtual void move_to(const Vector& pos) override;

    //virtual ObjectSettings get_settings();
    virtual void save(Writer& writer) override;
};

class LevelDot : public WorldmapObject
{
  public:
    LevelDot(const ReaderMapping& lisp);

    virtual std::string get_class() const override {
      return "level";
    }

    virtual void draw(DrawingContext& context) override;

    virtual ObjectSettings get_settings() override;
    virtual void save(Writer& writer) override;
    virtual void after_editor_set() override;

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

    virtual std::string get_class() const override {
      return "teleporter";
    }

    virtual void draw(DrawingContext& context) override;

    virtual ObjectSettings get_settings() override;
    virtual void save(Writer& writer) override;

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

    virtual std::string get_class() const override {
      return "worldmap-spawnpoint";
    }

    virtual ObjectSettings get_settings() override;
    virtual void save(Writer& writer) override;

  private:
    worldmap::Direction dir;
};

class SpriteChange : public WorldmapObject
{
  public:
    SpriteChange(const ReaderMapping& lisp);

    virtual std::string get_class() const override {
      return "sprite-change";
    }

    virtual ObjectSettings get_settings() override;
    virtual void save(Writer& writer) override;

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

    virtual std::string get_class() const override {
      return "special-tile";
    }

    virtual ObjectSettings get_settings() override;
    virtual void save(Writer& writer) override;

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
