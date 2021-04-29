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

#ifndef HEADER_SUPERTUX_OBJECT_DISPLAY_EFFECT_HPP
#define HEADER_SUPERTUX_OBJECT_DISPLAY_EFFECT_HPP

#include "supertux/game_object.hpp"
#include "scripting/display_effect.hpp"
#include "squirrel/exposed_object.hpp"

class DisplayEffect final : public GameObject,
                      public ExposedObject<DisplayEffect, scripting::DisplayEffect>
{
public:
  DisplayEffect(const std::string& name = std::string());
  ~DisplayEffect() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual bool is_singleton() const override { return true; }
  virtual bool is_saveable() const override { return false; }

  /** @name Scriptable Methods
      @{ */

  void fade_out(float fadetime);
  void fade_in(float fadetime);
  void set_black(bool enabled);
  bool is_black() const;
  void sixteen_to_nine(float fadetime);
  void four_to_three(float fadetime);

  /** @} */

private:
  enum class FadeType {
    NO_FADE, FADE_IN, FADE_OUT
  };

private:
  FadeType screen_fade;
  float screen_fadetime;
  float screen_fading;
  FadeType border_fade;
  float border_fadetime;
  float border_fading;
  float border_size;

  bool black;
  bool borders;

private:
  DisplayEffect(const DisplayEffect&) = delete;
  DisplayEffect& operator=(const DisplayEffect&) = delete;
};

#endif

/* EOF */
