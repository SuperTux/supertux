//  SuperTux
//  Copyright (C) 2026 MatusGuy
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

#pragma once

#include <string>
#include <cinttypes>

#include "util/gettext.hpp"

enum CheevoRarity : std::uint8_t {
  CHEEVO_RARITY_NORMAL,
  CHEEVO_RARITY_RARE
};

struct CheevoData {
  std::string name;
  std::string requirement;
  std::string image;
  CheevoRarity rarity;

  inline std::string get_name() const {
    return _(name);
  }

  inline std::string get_requirement() const {
    return _(requirement);
  }
};

using CheevoId = unsigned;

enum Cheevo : CheevoId {
  CHEEVO_BEAT_YETI,
  CHEEVO_BEAT_GHOSTTREE
};

// Don't expect these strings to be translated.
// In order to get the real information, use the CheevoData::get_* methods.
static const CheevoData g_cheevo_data[] = {
  {_("Angry Fuzzball"),   _("Beat the Yeti"),       "", CHEEVO_RARITY_NORMAL},
  {_("Nature's Vaccine"), _("Beat the Ghost Tree"), "", CHEEVO_RARITY_RARE  },
};
