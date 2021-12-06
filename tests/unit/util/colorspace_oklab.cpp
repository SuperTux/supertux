//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "util/colorspace_oklab.hpp"

#include <gtest/gtest.h>

#include "video/color.hpp"

TEST(ColorOKLCh, ctor)
{
  ColorOKLCh color(.3f, .4f, .5f);
  EXPECT_FLOAT_EQ(color.L, .3f);
  EXPECT_FLOAT_EQ(color.C, .4f);
  EXPECT_FLOAT_EQ(color.h, .5f);
}

TEST(ColorOKLCh, ctor_Color)
{
  Color col(.3f, .4f, .5f);
  ColorOKLCh color(col);
  EXPECT_FLOAT_EQ(color.L, .50023675f);
  EXPECT_FLOAT_EQ(color.C, .050982524f);
  EXPECT_FLOAT_EQ(color.h, -1.9393998f);
}

TEST(ColorOKLCh, to_srgb)
{
  Color col(.1f, 1.f, 0.f);
  ColorOKLCh color(col);

  col = color.to_srgb();

  // Can't use strict equality because of imprecision
  // 1 / 256 = 0.00390625, so 0.001 should make no difference.
  EXPECT_NEAR(col.red, .1f, 0.001f);
  EXPECT_NEAR(col.green, 1.f, 0.001f);
  EXPECT_NEAR(col.blue, 0.f, 0.001f);
}

TEST(ColorOKLCh, get_maximum_chroma)
{
  ColorOKLCh color(.6f, 1.f, .3f);

  float chroma = color.get_maximum_chroma();

  EXPECT_FLOAT_EQ(chroma, .24032472f);
}

TEST(ColorOKLCh, get_maximum_chroma_any_l)
{
  ColorOKLCh color(0.f, .1f, .05f);

  float chroma = color.get_maximum_chroma_any_l();

  EXPECT_FLOAT_EQ(chroma, .26009744f);
}

TEST(ColorOKLCh, clip_chroma)
{
  ColorOKLCh color(.45f, .67f, .12f);

  color.clip_chroma();

  EXPECT_FLOAT_EQ(color.L, .45f);
  EXPECT_FLOAT_EQ(color.C, .1804768f);
  EXPECT_FLOAT_EQ(color.h, .12f);
}

TEST(ColorOKLCh, clip_lightness)
{
  ColorOKLCh color(.45f, .67f, .12f);

  color.clip_lightness();

  EXPECT_FLOAT_EQ(color.L, .64147455f);
  EXPECT_FLOAT_EQ(color.C, .2572695f);
  EXPECT_FLOAT_EQ(color.h, .12f);
}

TEST(ColorOKLCh, clip_adaptive_L0_L_cusp)
{
  ColorOKLCh color(1.f, 1.f, 1.f);

  color.clip_adaptive_L0_L_cusp(0.25f);

  EXPECT_FLOAT_EQ(color.L, .83574224f);
  EXPECT_FLOAT_EQ(color.C, .10836758f);
  EXPECT_FLOAT_EQ(color.h, 1.f);
}

/* EOF */
