//  SuperTux
//  Copyright (C) 2016 Tapesh Mandal <tapesh.mandal@gmail.com>
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

#include "st_assert.hpp"
#include "math/aatriangle.hpp"

int main(void)
{
  ST_ASSERT("verticle_flip of 30", 33 == AATriangle::vertical_flip(30));
  ST_ASSERT("verticle_flip of 40", 19 == AATriangle::vertical_flip(40));
  ST_ASSERT("verticle_flip of 50", 49 == AATriangle::vertical_flip(50));
  
  return 0;
}

/* EOF */
