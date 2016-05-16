//  Dart - Your average poison dart
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

#ifndef HEADER_SUPERTUX_BADGUY_DART_HPP
#define HEADER_SUPERTUX_BADGUY_DART_HPP

#include "badguy/badguy.hpp"

class SoundSource;

/**
 * Badguy "Dart" - Your average poison dart
 */
class Dart : public BadGuy
{
public:
  Dart(const ReaderMapping& reader);
  Dart(const Vector& pos, Direction d, const BadGuy* parent);
  ~Dart();

  void initialize();
  void activate();
  void deactivate();

  void active_update(float elapsed_time);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);

  virtual bool updatePointers(const GameObject* from_object, GameObject* to_object);
  std::string get_class() const {
    return "dart";
  }
  std::string get_display_name() const {
    return _("Dart");
  }

  bool is_flammable() const;

  void stop_looping_sounds();
  void play_looping_sounds();

  void after_editor_set();

protected:
  const BadGuy* parent; /**< collisions with this BadGuy will be ignored */
  std::unique_ptr<SoundSource> sound_source; /**< SoundSource for ambient sound */

private:
  Dart(const Dart&);
  Dart& operator=(const Dart&);
};

#endif

/* EOF */
