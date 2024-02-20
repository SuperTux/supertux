//  SuperTux - Thunderstorm Game Object
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_THUNDERSTORM_HPP
#define HEADER_SUPERTUX_OBJECT_THUNDERSTORM_HPP

#include <map>

#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"

class DrawingContext;
class ReaderMapping;

/** Thunderstorm scriptable GameObject; plays thunder, lightning and
    electrifies water at regular interval */
class Thunderstorm final : public GameObject
{
public:
  static void register_class(ssq::VM& vm);

public:
  Thunderstorm(const ReaderMapping& reader);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  static std::string class_name() { return "thunderstorm"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "Thunderstorm"; }
  static std::string display_name() { return _("Thunderstorm"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;

  virtual const std::string get_icon_path() const override { return "images/engine/editor/thunderstorm.png"; }

  /** @name Scriptable Methods
      @{ */

  /**
   * Starts playing thunder and lightning at a configured interval.
   */
  void start();
  /**
   * Stops playing thunder and lightning at a configured interval.
   */
  void stop();

  /**
   * Plays thunder.
   */
  void thunder();
  /**
   * Plays lightning, i.e. calls ""flash()"" and ""electrify()"".
   */
  void lightning();
  /**
   * Displays a flash.
   */
  void flash();
  /**
   * Electrifies water throughout the whole sector for a short time.
   */
  void electrify();

  /** @} */

private:
  bool running; /**< whether we currently automatically trigger lightnings */
  float interval; /**< time between two lightnings */
  int layer; /**< layer, where flash will be painted */
  
  std::string m_strike_script;

  Timer time_to_thunder; /**< counts down until next thunder */
  Timer time_to_lightning; /**< counts down until next lightning */
  Timer flash_display_timer; /**< counts down while flash is displayed */

  std::map<uint32_t, uint32_t> changing_tiles; /**< preserves the tiles which an electrocution should change */

private:
  Thunderstorm(const Thunderstorm&) = delete;
  Thunderstorm& operator=(const Thunderstorm&) = delete;
};

#endif

/* EOF */
