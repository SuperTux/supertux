//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_HAYWIRE_HPP
#define HEADER_SUPERTUX_BADGUY_HAYWIRE_HPP

#include "badguy/walking_badguy.hpp"

class SoundSource;

class Haywire : public WalkingBadguy
{
public:
  Haywire(const ReaderMapping& reader);

  void kill_fall();
  void ignite();

  void active_update(float elapsed_time);

  bool is_freezable() const;
  void freeze();

  void stop_looping_sounds();
  void play_looping_sounds();

  std::string get_class() const {
    return "haywire";
  }
  std::string get_display_name() const {
    return _("Haywire");
  }

protected:
  bool collision_squished(GameObject& object);

private:
  void start_exploding();
  void stop_exploding();

private:
  bool is_exploding;
  float time_until_explosion;
  bool is_stunned;
  float time_stunned;

  std::unique_ptr<SoundSource> ticking;
  std::unique_ptr<SoundSource> grunting;
};

#endif /* HEADER_SUPERTUX_BADGUY_HAYWIRE_HPP */

/* EOF */
