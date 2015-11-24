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

#include <gtest/gtest.h>

#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

TEST(ReaderTest, test)
{
  std::istringstream in(
    "(supertux-test\n"
    "   (mybool #t)\r"
    "   (myint 123456789)\r\n"
    "   (myfloat 1.125)\n\r"
    "   (mystring \"Hello World\")\n"
    "   (mymapping (a 1) (b 2))\n"
    ")\n");

  auto doc = ReaderDocument::parse(in);
  auto root = doc.get_root();
  ASSERT_EQ("supertux-test", root.get_name());
  auto mapping = root.get_mapping();

  bool mybool;
  mapping.get("mybool", mybool);
  ASSERT_EQ(true, mybool);

  int myint;
  mapping.get("myint", myint);
  ASSERT_EQ(123456789, myint);

  float myfloat;
  mapping.get("myfloat", myfloat);
  ASSERT_EQ(1.125, myfloat);

  std::string mystring;
  mapping.get("mystring", mystring);
  ASSERT_EQ("Hello World", mystring);

  ReaderMapping child_mapping;
  mapping.get("mymapping", child_mapping);

  int a;
  child_mapping.get("a", a);
  ASSERT_EQ(1, a);

  int b;
  child_mapping.get("b", b);
  ASSERT_EQ(2, b);
}

/* EOF */
