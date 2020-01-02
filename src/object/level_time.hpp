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

#ifndef HEADER_SUPERTUX_OBJECT_LEVEL_TIME_HPP
#define HEADER_SUPERTUX_OBJECT_LEVEL_TIME_HPP

#include "squirrel/exposed_object.hpp"
#include "scripting/level_time.hpp"
#include "supertux/game_object.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class ReaderMapping;

class LevelTime final : public GameObject,
                        public ExposedObject<LevelTime, scripting::LevelTime>
{
  static Color text_color;
public:
  LevelTime(const ReaderMapping& reader);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  /** @name Scriptable Methods
      @{ */

  /** Resumes the countdown */
  void start();

  /** Pauses the countdown */
  void stop();

  /** Returns the number of seconds left on the clock */
  float get_time() const;

  /** Changes the number of seconds left on the clock */
  void set_time(float time_left);

  /** @} */
  virtual std::string get_class() const override { return "leveltime"; }
  virtual std::string get_display_name() const override { return _("Time Limit"); }

  virtual ObjectSettings get_settings() override;

  virtual const std::string get_icon_path() const override { return "images/engine/editor/clock.png"; }

private:
  SurfacePtr time_surface;
  bool running;
  float time_left;

private:
  LevelTime(const LevelTime&) = delete;
  LevelTime& operator=(const LevelTime&) = delete;
};

#endif

/* EOF */
