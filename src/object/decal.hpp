//  SuperTux - Decal
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_DECAL_HPP
#define HEADER_SUPERTUX_OBJECT_DECAL_HPP

#include "object/moving_sprite.hpp"
#include "scripting/decal.hpp"
#include "squirrel/exposed_object.hpp"
#include "supertux/timer.hpp"

class ReaderMapping;

/** A decorative image, perhaps part of the terrain */
class Decal final : public MovingSprite,
                    public ExposedObject<Decal, scripting::Decal>
{
  friend class FlipLevelTransformer;

public:
  Decal(const ReaderMapping& reader);
  ~Decal() override;

  virtual HitResponse collision(GameObject& , const CollisionHit& ) override { return FORCE_MOVE; }

  virtual std::string get_class() const override { return "decal"; }
  virtual std::string get_display_name() const override { return _("Decal"); }

  virtual ObjectSettings get_settings() override;

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  virtual void on_flip(float height) override;

  void fade_in(float fade_time);
  void fade_out(float fade_time);
  void fade_sprite(const std::string& new_sprite, float fade_time);

  void set_visible(bool v) { m_visible = v; }
  bool is_visible() const { return m_visible; }

private:
  std::string m_default_action;
  bool m_solid;
  Flip m_flip;
  SpritePtr m_fade_sprite;
  Timer m_fade_timer;
  Timer m_sprite_timer;
  bool m_visible;

private:
  Decal(const Decal&) = delete;
  Decal& operator=(const Decal&) = delete;
};

#endif

/* EOF */
