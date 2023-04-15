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

#ifndef HEADER_SUPERTUX_OBJECT_SPOTLIGHT_HPP
#define HEADER_SUPERTUX_OBJECT_SPOTLIGHT_HPP

#include "scripting/spotlight.hpp"
#include "sprite/sprite_ptr.hpp"
#include "squirrel/exposed_object.hpp"
#include "supertux/moving_object.hpp"
#include "video/color.hpp"

class ReaderMapping;

class Spotlight final : public MovingObject,
                        public ExposedObject<Spotlight, scripting::Spotlight>
{
public:
  enum class Direction {
    CLOCKWISE,
    COUNTERCLOCKWISE,
    STOPPED
  };

  static Direction Direction_from_string(const std::string& s);
  static std::string Direction_to_string(Direction dir);

public:
  Spotlight(const ReaderMapping& reader);
  ~Spotlight() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit_) override;

  static std::string class_name() { return "spotlight"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Spotlight"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;

  virtual int get_layer() const override { return m_layer; }

  Color get_color() const { return color; }
  const Sprite & get_lightcone() const { return *lightcone; }

  void set_angle(float angle_) { angle = angle_; }
  void set_speed(float speed_) { speed = speed_; }
  void set_color(Color color_) { color = color_; }
  void set_direction(Direction dir) { m_direction = dir; }

  void ease_angle(float time, float target, EasingMode ease = EasingMode::EaseNone)
  {
    m_fade_helpers.push_back(std::make_unique<FadeHelper>(&angle, time, target, getEasingByName(ease)));
  }

  void ease_speed(float time, float target, EasingMode ease = EasingMode::EaseNone)
  {
    m_fade_helpers.push_back(std::make_unique<FadeHelper>(&speed, time, target, getEasingByName(ease)));
  }

  void ease_color(float time, Color target, EasingMode ease = EasingMode::EaseNone)
  {
    m_fade_helpers.push_back(std::make_unique<FadeHelper>(&color.red,   time, target.red,   getEasingByName(ease)));
    m_fade_helpers.push_back(std::make_unique<FadeHelper>(&color.green, time, target.green, getEasingByName(ease)));
    m_fade_helpers.push_back(std::make_unique<FadeHelper>(&color.blue,  time, target.blue,  getEasingByName(ease)));
    m_fade_helpers.push_back(std::make_unique<FadeHelper>(&color.alpha, time, target.alpha, getEasingByName(ease)));
  }

private:
  float   angle;
  SpritePtr center;
  SpritePtr base;
  SpritePtr lights;
  SpritePtr light;
  SpritePtr lightcone;

  Color   color;

  /** Speed that the spotlight is rotating with */
  float speed;

  /** The direction of the spotlight */
  Direction m_direction;

  /** The layer (z-pos) of the spotlight. */
  int m_layer;

private:
  Spotlight(const Spotlight&) = delete;
  Spotlight& operator=(const Spotlight&) = delete;
};

#endif

/* EOF */
