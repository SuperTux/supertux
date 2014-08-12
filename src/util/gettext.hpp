//  SuperTux
//  Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_UTIL_GETTEXT_HPP
#define HEADER_SUPERTUX_UTIL_GETTEXT_HPP

#include <tinygettext/tinygettext.hpp>
#include <assert.h>

#include "supertux/globals.hpp"

/*
 * If you need to do a nontrivial substitution of values into a pattern, use
 * boost::format rather than an ad-hoc concatenation.  That way, translators can
 * translate the format string as a whole (and even rearrange the values if
 * necessary with "%1$s"-style codes) instead of multiple pieces.  Patterns like
 * "Label: %s" with only one string piece are a borderline case where
 * boost::format is not really necessary.
 *
 * http://www.mihai-nita.net/article.php?artID=20060430a
 *
 * Bad:
 *     std::string msg = _("You collected ") + num + _(" coins");
 *     std::cout << _("You collected ") << num << _(" coins");
 * Good:
 *     #include <boost/format.hpp>
 *     std::string msg = str(boost::format(_("You collected %d coins")) % num);
 *     std::cout << boost::format(_("You collected %d coins")) % num;
 */

static inline std::string _(const std::string& message)
{
  if (dictionary_manager)
  {
    return dictionary_manager->get_dictionary().translate(message);
  }
  else
  {
    return message;
  }
}

#endif

/* EOF */
