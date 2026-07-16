//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
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

// Headless, dependency-free coverage for util/string_util.hpp.
// Uses the project's st_assert.hpp harness so it builds without gtest/glm/SDL.

#include "st_assert.hpp"
#include "util/string_util.hpp"

#include <algorithm>
#include <string>
#include <vector>

int main(void)
{
  // --- has_suffix --------------------------------------------------------
  ST_ASSERT("has_suffix: basic match",
            StringUtil::has_suffix("level.stl", ".stl") == true);
  ST_ASSERT("has_suffix: no match",
            StringUtil::has_suffix("level.stl", ".txt") == false);
  ST_ASSERT("has_suffix: empty suffix matches everything",
            StringUtil::has_suffix("anything", "") == true);
  ST_ASSERT("has_suffix: longer suffix than string",
            StringUtil::has_suffix("ab", "abcd") == false);
  ST_ASSERT("has_suffix: empty string with non-empty suffix",
            StringUtil::has_suffix("", ".stl") == false);
  ST_ASSERT("has_suffix: exact full-string suffix",
            StringUtil::has_suffix("abc", "abc") == true);

  // --- starts_with -------------------------------------------------------
  ST_ASSERT("starts_with: basic match",
            StringUtil::starts_with("SuperTux", "Super") == true);
  ST_ASSERT("starts_with: no match",
            StringUtil::starts_with("SuperTux", "tux") == false);
  ST_ASSERT("starts_with: empty prefix matches",
            StringUtil::starts_with("x", "") == true);
  ST_ASSERT("starts_with: empty string with non-empty prefix",
            StringUtil::starts_with("", "x") == false);

  // --- tolower -----------------------------------------------------------
  ST_ASSERT("tolower: mixed case",
            StringUtil::tolower("SuperTux") == "supertux");
  ST_ASSERT("tolower: already lower",
            StringUtil::tolower("supertux") == "supertux");
  ST_ASSERT("tolower: digits untouched",
            StringUtil::tolower("Tux123") == "tux123");
  ST_ASSERT("tolower: empty",
            StringUtil::tolower("") == "");

  // --- replace_all -------------------------------------------------------
  ST_ASSERT("replace_all: single occurrence",
            StringUtil::replace_all("a.b.c", ".", "/") == "a/b/c");
  ST_ASSERT("replace_all: no occurrence",
            StringUtil::replace_all("abc", "x", "y") == "abc");
  ST_ASSERT("replace_all: empty needle is no-op",
            StringUtil::replace_all("abc", "", "x") == "abc");
  ST_ASSERT("replace_all: empty replacement deletes needle",
            StringUtil::replace_all("aXXb", "XX", "") == "ab");
  ST_ASSERT("replace_all: overlapping not expected to merge",
            StringUtil::replace_all("aaa", "aa", "b") == "ba");

  // --- split -------------------------------------------------------------
  {
    std::vector<std::string> out;
    StringUtil::split(out, "a,b,c", ',');
    ST_ASSERT("split: three parts", out.size() == 3);
    ST_ASSERT("split: part[0]", out[0] == "a");
    ST_ASSERT("split: part[2]", out[2] == "c");
  }
  {
    std::vector<std::string> out;
    StringUtil::split(out, "no-delimiter", ',');
    ST_ASSERT("split: no delimiter -> single part", out.size() == 1 && out[0] == "no-delimiter");
  }
  {
    std::vector<std::string> out;
    StringUtil::split(out, "", ',');
    ST_ASSERT("split: empty string -> no parts", out.empty());
  }
  {
    std::vector<std::string> out;
    StringUtil::split(out, "x,,y", ',');
    ST_ASSERT("split: empty middle field kept", out.size() == 3 && out[1] == "");
  }

  // --- numeric_less (sort -n semantics) ----------------------------------
  {
    std::vector<std::string> lst{"A12", "A123", "A1A1", "A1A12", "Z1", "B1235"};
    std::sort(lst.begin(), lst.end(), StringUtil::numeric_less);
    std::vector<std::string> expected{"A1A1", "A1A12", "A12", "A123", "B1235", "Z1"};
    ST_ASSERT("numeric_less: sorts embedded numbers correctly",
              lst == expected);
  }
  ST_ASSERT("numeric_less: pure number ordering",
            (StringUtil::numeric_less("2", "10")) == true);
  ST_ASSERT("numeric_less: equal strings not less",
            (StringUtil::numeric_less("abc", "abc")) == false);

  return 0;
}
