//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

//
// TEST DISABLED:
//  - I could not get it to work because i bad
//

#include "st_assert.hpp"
#include "util/dynamic_scoped_ref.hpp"

DynamicScopedRef<const int> d_value;

int main(void)
{
  int v1 = 1;
  int v2 = 2;
  int v3 = 3;

  {
    auto guard1 = d_value.bind(v1);

    ST_ASSERT("guard1 is bound test", d_value);
    ST_ASSERT("v1 access check", *d_value == 1);
    ST_ASSERT("check function for v1", *d_value.get() == 1);
    {
      auto guard2 = d_value.bind(v2);

      ST_ASSERT("guard2 is bound test", d_value);
      ST_ASSERT("v2 access check", *d_value == 2);
      ST_ASSERT("check function for v2", *d_value.get() == 2);
      {
        auto guard3 = d_value.bind(v3);
        ST_ASSERT("v3 access check", *d_value == 3);
        ST_ASSERT("check function for v3", *d_value.get() == 3);
      }
      ST_ASSERT("guard3 scope check", d_value);
      ST_ASSERT("v2 access check again", *d_value == 2);
      ST_ASSERT("check function for v2 again", *d_value.get() == 2);
    }
    ST_ASSERT("guard2 scope check", d_value);
    ST_ASSERT("v1 access check again", *d_value == 1);
    ST_ASSERT("check function for v1 again", *d_value.get() == 1);
  }

  ST_ASSERT("see if the guards actually work (false check)", !d_value);
}

/* EOF */
