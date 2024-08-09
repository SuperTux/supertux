//  SuperTux
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//                2023 Vankata453
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

#ifndef HEADER_SUPERTUX_SQUIRREL_SUGGESTIONS_HPP
#define HEADER_SUPERTUX_SQUIRREL_SUGGESTIONS_HPP

#include "squirrel/scripting_reference.hpp"

class ReaderMapping;

namespace squirrel {

struct Suggestion final
{
  Suggestion(const std::string& name_, const ScriptingObject* ref_,
             const bool is_instance_);

  std::string name;
  const ScriptingObject* reference;
  bool is_instance;
};
typedef std::vector<Suggestion> SuggestionStack;

SuggestionStack get_suggestions(const std::string& prefix, bool remove_prefix);

} // namespace squirrel

#endif
