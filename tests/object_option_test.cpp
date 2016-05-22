//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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
#include <iostream>
#include <errno.h>
#include <string.h>
#include <vector>

#include "editor/object_option.hpp"
#include "video/color.hpp"

TEST(ObjectOption, to_string)
{
  {
    std::string mystring = "field";
    ObjectOption textfield(MN_TEXTFIELD, "test", &mystring);
    ASSERT_EQ(mystring, textfield.to_string());
  }

  {
    int myint = 73258;
    ObjectOption intfield(MN_INTFIELD, "test", &myint);
    ASSERT_EQ("73258", intfield.to_string());
  }

  {
    float myfloat = 2.125;
    ObjectOption numfield(MN_NUMFIELD, "test", &myfloat);
    ASSERT_EQ("2.125000", numfield.to_string());
  }

  {
    std::vector<std::string> select;
    select.push_back("foo");
    select.push_back("bar");
    enum FooBar {
      FOO, BAR
    };
    FooBar fb1 = FOO;
    FooBar fb2 = BAR;
    ObjectOption stringselect1(MN_STRINGSELECT, "test", &fb1);
    stringselect1.select = select;
    ObjectOption stringselect2(MN_STRINGSELECT, "test", &fb2);
    stringselect2.select = select;
    ASSERT_EQ("foo", stringselect1.to_string());
    ASSERT_EQ("bar", stringselect2.to_string());
  }

  {
    std::vector<std::string> select;
    select.push_back("foo");
    select.push_back("bar");
    select.push_back("blb");
    ObjectOption badguyselect(MN_BADGUYSELECT, "test", &select);
    ASSERT_EQ("3", badguyselect.to_string());
  }

  {
    Color mycolor = Color::YELLOW;
    ObjectOption color(MN_COLOR, "test", &mycolor);
    ASSERT_EQ("1.000000 1.000000 0.000000", color.to_string());
  }

  {
    std::string myscript = "sector.set_trolling_mode(true)";
    ObjectOption script(MN_SCRIPT, "text", &myscript);
    ASSERT_EQ("...", script.to_string());
  }
}

/* EOF */
