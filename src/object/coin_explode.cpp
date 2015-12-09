//  CoinExplode - several coins are hurled through the air
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#include "object/coin_explode.hpp"

#include "math/random_generator.hpp"
#include "object/coin.hpp"
#include "supertux/sector.hpp"

CoinExplode::CoinExplode(const Vector& pos) :
  position(pos)
{
}

void
CoinExplode::update(float )
{
  int mag = 100; // madnitude that coins are to be thrown
  int rand = 30; // max variation to be subtracted from magnitide

  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (2.5,-4.5)*(mag-gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (2,-5)*(mag-gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (1.5,-5.5)*(mag-gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (1,-6)*(mag+gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (0.5,-6.5)*(mag-gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (-2.5,-4.5)*(mag-gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (-2,-5)*(mag-gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (-1.5,-5.5)*(mag-gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (-1,-6)*(mag+gameRandom.rand(rand))));
  Sector::current()->add_object(std::make_shared<HeavyCoin>(position, Vector (-0.5,-6.5)*(mag-gameRandom.rand(rand))));

  remove_me();
}

void
CoinExplode::draw(DrawingContext &)
{
}

/* EOF */
