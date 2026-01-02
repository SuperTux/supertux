//  SuperTux
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

#pragma once

#include "object/draggable_region.hpp"
#include "supertux/moving_object.hpp"

#include "video/layer.hpp"

class ReaderMapping;

/** A tile that starts falling down if tux stands to long on it */
class InvisibleWall final : public DraggableRegion
{
public:
  InvisibleWall(const ReaderMapping& mapping);

  virtual HitResponse collision(MovingObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "invisible_wall"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Invisible Wall"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return MovingObject::get_class_types().add(typeid(InvisibleWall)); }
  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual int get_layer() const override { return LAYER_OBJECTS; }

private:
  virtual void update(float dt_sec) override;

private:
  float width;
  float height;

private:
  InvisibleWall(const InvisibleWall&) = delete;
  InvisibleWall& operator=(const InvisibleWall&) = delete;
};
