//  SuperTux
//  Copyright (C) 2022 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_LIT_OBJECT_HPP
#define HEADER_SUPERTUX_OBJECT_LIT_OBJECT_HPP

#include "scripting/lit_object.hpp"
#include "sprite/sprite_ptr.hpp"
#include "squirrel/exposed_object.hpp"
#include "supertux/moving_object.hpp"
#include "video/flip.hpp"

class ReaderMapping;

class LitObject final :
  public MovingObject,
  public ExposedObject<LitObject, scripting::LitObject>
{
public:
  LitObject(const ReaderMapping& reader);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float) override;

  virtual HitResponse collision(GameObject&, const CollisionHit&) override
    { return ABORT_MOVE; }

  static std::string class_name() { return "lit-object"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Lit object"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual int get_layer() const override { return m_layer; }

  virtual void on_flip(float height) override;

  const std::string& get_action() const;
  void set_action(const std::string& action);
  const std::string& get_light_action() const;
  void set_light_action(const std::string& action);

private:
  Vector m_light_offset;
  std::string m_sprite_name;
  std::string m_light_sprite_name;
  std::string m_sprite_action;
  std::string m_light_sprite_action;
  SpritePtr m_sprite;
  SpritePtr m_light_sprite;
  int m_layer;
  Flip m_flip;

private:
  LitObject(const LitObject&) = delete;
  LitObject& operator=(const LitObject&) = delete;
};

#endif

/* EOF */
