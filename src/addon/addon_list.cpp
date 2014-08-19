//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "addon/addon_list.hpp"

#include <memory>
#include <sstream>
#include <stdexcept>

#include "addon/addon.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/parser.hpp"
#include "util/log.hpp"

static const char* allowed_characters = "-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

std::vector<std::unique_ptr<Addon> >
AddonList::parse(const std::string& addoninfos)
{
  std::vector<std::unique_ptr<Addon> > m_addons;

  try
  {
    lisp::Parser parser;
    std::stringstream addoninfos_stream(addoninfos);
    const lisp::Lisp* root = parser.parse(addoninfos_stream, "supertux-addons");

    const lisp::Lisp* addons_lisp = root->get_lisp("supertux-addons");
    if(!addons_lisp)
    {
      throw std::runtime_error("Downloaded file is not an Add-on list");
    }

    lisp::ListIterator iter(addons_lisp);
    while(iter.next())
    {
      const std::string& token = iter.item();
      if(token != "supertux-addoninfo")
      {
        log_warning << "Unknown token '" << token << "' in Add-on list" << std::endl;
        continue;
      }
      std::unique_ptr<Addon> addon(new Addon(m_addons.size()));
      addon->parse(*(iter.lisp()));
      addon->installed = false;
      addon->loaded = false;

      // make sure the list of known Add-ons does not already contain this one
      bool exists = false;
      for (auto i = m_addons.begin(); i != m_addons.end(); ++i) {
        if (**i == *addon) {
          exists = true;
          break;
        }
      }

      if (exists)
      {
        // do nothing
      }
      else if (addon->suggested_filename.find_first_not_of(allowed_characters) != std::string::npos)
      {
        // make sure the Add-on's file name does not contain weird characters
        log_warning << "Add-on \"" << addon->title << "\" contains unsafe file name. Skipping." << std::endl;
      }
      else
      {
        m_addons.push_back(std::move(addon));
      }
    }

    return m_addons;
  }
  catch(std::exception& e)
  {
    std::stringstream msg;
    msg << "Problem when reading Add-on list: " << e.what();
    throw std::runtime_error(msg.str());
  }
}

/* EOF */
