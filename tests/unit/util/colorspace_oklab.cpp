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

  // Can't use strict equality because of imprecision
  // 1 / 256 = 0.00390625, so 0.001 should make no difference.
  EXPECT_NEAR(color.L, .3f, 0.001f);
  EXPECT_NEAR(color.C, .4f, 0.001f);
  EXPECT_NEAR(color.h, .5f, 0.001f);
}

TEST(ColorOKLCh, ctor_Color)
{
  Color col(.3f, .4f, .5f);
  ColorOKLCh color(col);
  EXPECT_NEAR(color.L, .50023675f, 0.001f);
  EXPECT_NEAR(color.C, .050982524f, 0.001f);
  EXPECT_NEAR(color.h, -1.9393998f, 0.001f);
}

TEST(ColorOKLCh, to_srgb)
{
  Color col(.1f, 1.f, 0.f);
  ColorOKLCh color(col);

  col = color.to_srgb();

  EXPECT_NEAR(col.red, .1f, 0.001f);
  EXPECT_NEAR(col.green, 1.f, 0.001f);
  EXPECT_NEAR(col.blue, 0.f, 0.001f);
}

TEST(ColorOKLCh, get_maximum_chroma)
{
  ColorOKLCh color(.6f, 1.f, .3f);

  float chroma = color.get_maximum_chroma();

  EXPECT_NEAR(chroma, .24032472f, 0.001f);
}

TEST(ColorOKLCh, get_maximum_chroma_any_l)
{
  ColorOKLCh color(0.f, .1f, .05f);

  float chroma = color.get_maximum_chroma_any_l();

  EXPECT_NEAR(chroma, .26009744f, 0.001f);
}

TEST(ColorOKLCh, clip_chroma)
{
  ColorOKLCh color(.45f, .67f, .12f);

  color.clip_chroma();

  EXPECT_NEAR(color.L, .45f, 0.001f);
  EXPECT_NEAR(color.C, .1804768f, 0.001f);
  EXPECT_NEAR(color.h, .12f, 0.001f);
}

TEST(ColorOKLCh, clip_lightness)
{
  ColorOKLCh color(.45f, .67f, .12f);

  color.clip_lightness();

  EXPECT_NEAR(color.L, .64147455f, 0.001f);
  EXPECT_NEAR(color.C, .2572695f, 0.001f);
  EXPECT_NEAR(color.h, .12f, 0.001f);
}

TEST(ColorOKLCh, clip_adaptive_L0_L_cusp)
{
  ColorOKLCh color(1.f, 1.f, 1.f);

  color.clip_adaptive_L0_L_cusp(0.25f);

  EXPECT_NEAR(color.L, .83574224f, 0.001f);
  EXPECT_NEAR(color.C, .10836758f, 0.001f);
  EXPECT_NEAR(color.h, 1.f, 0.001f);
}

/* EOF */
