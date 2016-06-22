//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#ifndef HEADER_SUPERTUX_BADGUY_OWL_HPP
#define HEADER_SUPERTUX_BADGUY_OWL_HPP

#include "badguy/badguy.hpp"

class Portable;

class Owl : public BadGuy
{
public:
  Owl(const ReaderMapping& reader);
  Owl(const Vector& pos, Direction d);
  virtual void save(Writer& writer);

  void initialize();
  void collision_solid(const CollisionHit& hit);
  void kill_fall();

  void freeze();
  void unfreeze();
  bool is_freezable() const;
  void ignite();
  std::string get_class() const {
    return "owl";
  }
  std::string get_display_name() const {
    return _("Owl");
  }

  void after_editor_set();

protected:
  bool is_above_player() const;
  void active_update (float elapsed_time);
  bool collision_squished(GameObject& object);

  std::string carried_obj_name;
  Portable *carried_object;

private:
  Owl(const Owl&);
  Owl& operator=(const Owl&);
};

#endif /* HEADER_SUPERTUX_BADGUY_OWL_HPP */

/* EOF */
