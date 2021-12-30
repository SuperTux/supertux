//  SuperTux
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

#ifndef HEADER_SUPERTUX_OBJECT_RUBLIGHT_HPP
#define HEADER_SUPERTUX_OBJECT_RUBLIGHT_HPP

#include "object/moving_sprite.hpp"
#include "sprite/sprite_ptr.hpp"
#include "video/color.hpp"
#include "video/flip.hpp"

/** A triboluminescent (or something similar) block */
class RubLight final : public MovingSprite
{
public:
  RubLight(const ReaderMapping& mapping);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual std::string get_class() const override { return "rublight"; }
  virtual std::string get_display_name() const override { return _("Rublight"); }
  virtual ObjectSettings get_settings() override;

  virtual void on_flip(float height) override;

private:
  enum State {
    STATE_DARK,
    STATE_FADING
  };

  State state;
  float stored_energy;
  SpritePtr light;

  Color color;
  float fading_speed;
  float strength_multiplier;

  Flip m_flip;

private:
  void rub(float strength);
  float get_brightness() const;

  RubLight(const RubLight&) = delete;
  RubLight& operator=(const RubLight&) = delete;
};

#endif

/* EOF */
