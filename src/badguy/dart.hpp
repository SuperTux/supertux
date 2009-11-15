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

#ifndef __DART_H__
#define __DART_H__

#include "badguy.hpp"

class SoundSource;

/**
 * Badguy "Dart" - Your average poison dart
 */
class Dart : public BadGuy
{
public:
  Dart(const lisp::Lisp& reader);
  Dart(const Vector& pos, Direction d, const BadGuy* parent);
  Dart(const Dart& dart);
  ~Dart();

  void initialize();
  void activate();
  void deactivate();
  void write(lisp::Writer& writer);

  void active_update(float elapsed_time);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);

  virtual Dart* clone() const { return new Dart(*this); }

  virtual bool updatePointers(const GameObject* from_object, GameObject* to_object);

protected:
  const BadGuy* parent; /**< collisions with this BadGuy will be ignored */
  std::auto_ptr<SoundSource> sound_source; /**< SoundSource for ambient sound */

private:
  Dart& operator=(const Dart&);
};

#endif
