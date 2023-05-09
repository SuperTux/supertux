//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#ifndef HEADER_SUPERTUX_SUPERTUX_SECTOR_BASE_HPP
#define HEADER_SUPERTUX_SUPERTUX_SECTOR_BASE_HPP

#include "supertux/game_object_manager.hpp"

#include "squirrel/squirrel_environment.hpp"

class Level;

namespace Base {

/** A base for sector classes. Contains main properties and functions. */
class Sector : public GameObjectManager
{
public:
  Sector(Level& parent, const std::string& type);
  virtual ~Sector() {}

  /** Needs to be called after parsing to finish the construction of
      the Sector before using it. */
  virtual void finish_construction(bool editable) {}

  virtual void draw(DrawingContext& context) = 0;
  virtual void update(float dt_sec) = 0;

  Level& get_level() const { return m_level; }

  void set_name(const std::string& name_) { m_name = name_; }
  const std::string& get_name() const { return m_name; }

  /** set gravity throughout sector */
  void set_gravity(float gravity);
  float get_gravity() const { return m_gravity; }

  void set_init_script(const std::string& init_script) { m_init_script = init_script; }
  void run_script(const std::string& script, const std::string& sourcename);

protected:
  Level& m_level; // Parent level

  std::string m_name;
  std::string m_init_script;
  float m_gravity;

  std::unique_ptr<SquirrelEnvironment> m_squirrel_environment;

private:
  Sector(const Sector&) = delete;
  Sector& operator=(const Sector&) = delete;
};

} // namespace Base

#endif

/* EOF */
