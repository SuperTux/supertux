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

#include <gtest/gtest.h>

#include "math/rect.hpp"

TEST(RectTest, contains_point)
{
  ASSERT_TRUE(Rect(100, 100, 200, 200).contains(150, 150));
  ASSERT_FALSE(Rect(100, 100, 200, 200).contains(250, 150));
}

TEST(RectTest, contains_rect)
{
  ASSERT_TRUE(Rect(100, 100, 200, 200).contains(Rect(150, 150, 190, 190)));
  ASSERT_TRUE(Rect(100, 100, 200, 200).contains(Rect(100, 100, 200, 200)));
  ASSERT_FALSE(Rect(100, 100, 200, 200).contains(Rect(150, 150, 250, 250)));
}

TEST(RectTest, moved)
{
  ASSERT_EQ(Rect(0, 0, 100, 300).moved(16, 32), Rect(16, 32, 116, 332));
  ASSERT_EQ(Rect(0, 0, 100, 300).moved(-16, -32), Rect(-16, -32, 84, 268));
}

TEST(RectTest, size)
{
  ASSERT_EQ(Rect(50, 50, 100, 300).get_width(), 50);
  ASSERT_EQ(Rect(50, 50, 100, 300).get_height(), 250);
}

TEST(RectTest, from_center)
{
  ASSERT_EQ(Rect::from_center(16, 16, 32, 32), Rect(0, 0, 32, 32));
}

TEST(RectTest, SDL)
{
  // SDL
  const SDL_Rect sdl_rect_result = Rect(50, 50, 100, 100).to_sdl();
  const SDL_Rect sdl_rect_expected{50, 50, 50, 50};
  ASSERT_TRUE(SDL_RectEquals(&sdl_rect_result, &sdl_rect_expected));
  ASSERT_EQ(Rect(SDL_Rect{50, 50, 50, 50}), Rect(50, 50, 100, 100));
}

/* EOF */
