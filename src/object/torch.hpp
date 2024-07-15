//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2017 M. Teufel <mteufel@supertux.org>
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

#ifndef HEADER_SUPERTUX_OBJECT_TORCH_HPP
#define HEADER_SUPERTUX_OBJECT_TORCH_HPP

#include "object/moving_sprite.hpp"

class ReaderMapping;

/**
 * @scripting
 * @summary A ""Torch"" that was given a name can be controlled by scripts.
 * @instances A ""Torch"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Torch final : public MovingSprite
{
public:
  static void register_class(ssq::VM& vm);

public:
  Torch(const ReaderMapping& reader);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float) override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& ) override;

  static std::string class_name() { return "torch"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "Torch"; }
  static std::string display_name() { return _("Torch"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual int get_layer() const override { return m_layer; }

  virtual void on_flip(float height) override;

  /**
   * @scripting
   * @deprecated Use the ""burning"" property instead!
   * @description Returns ""true"" if the torch is burning.
   */
  bool get_burning() const;
  /**
   * @scripting
   * @deprecated Use the ""burning"" property instead!
   * @description Switches the burning state of the torch.
   * @param bool $burning
   */
  void set_burning(bool burning);

private:
  Color m_light_color;
  SpritePtr m_flame;
  SpritePtr m_flame_glow;
  SpritePtr m_flame_light;

  /**
   * @scripting
   * @description Determines whether the torch is burning.
   */
  bool m_burning;

private:
  Torch(const Torch&) = delete;
  Torch& operator=(const Torch&) = delete;
};

#endif

/* EOF */
