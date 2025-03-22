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
#include <memory>
#include <regex>

#include "control/controller.hpp"
#include "control/keyboard_config.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/keyboard_menu.hpp"
#include "util/log.hpp"

extern std::unique_ptr<tinygettext::DictionaryManager> g_dictionary_manager;

/**
 * @brief Substitutes variables within translated text with their actual value
 *        Currently, only {keyref} is supported for substitution.
 *
 * @param message The text to replace variables in
 * @return std::string Text with replaced variables
 */
static inline std::string replace_variables(const std::string& message)
{
  std::string translated_message = message;
  const std::regex keyref_format("\\{keyref\\s+(.+)\\}");
  std::smatch matches;

  while (std::regex_search(translated_message, matches, keyref_format))
  {
    const auto maybe_control = Control_from_string(matches[1].str());
    if(maybe_control && maybe_control.has_value())
    {
      auto key_code = g_config->keyboard_config.reversemap_key(0, maybe_control.value());
      translated_message = std::regex_replace(translated_message, keyref_format, KeyboardMenu::get_key_name(key_code));
    }
  }
  return translated_message;
}

/*
 * If you need to do a nontrivial substitution of values into a pattern, use
 * fmt::format rather than an ad-hoc concatenation.  That way, translators can
 * translate the format string as a whole (and even rearrange the values if
 * necessary with "%1$s"-style codes) instead of multiple pieces.  Patterns like
 * "Label: {}" with only one string piece are a borderline case where
 * fmt::format is not really necessary.
 *
 * http://www.mihai-nita.net/article.php?artID=20060430a
 * https://fmt.dev/latest/syntax.html
 *
 * Bad:
 *     std::string greeting = _("Hello ") + name + _("!");
 *     std::cout << _("Hello ") << name << _("!");
 * Good:
 *     #include <fmt/format.h>
 *     std::string greeting = fmt::format(fmt::runtime(_("Hello {}!")), name);
 *     std::cout << fmt::format(fmt::runtime(_("Hello {}!")), name);
 *
 * If you need singular and plural forms use __ instead of _ and fmt::format
 * if necessary.
 *
 * https://www.gnu.org/software/gettext/manual/html_node/Plural-forms.html
 *
 * Bad:
 *     std::cout << _("You collected ") << num << _(" coins");
 * Good:
 *     #include <fmt/format.h>
 *     std::cout << fmt::format(fmt::runtime(__("You collected {} coin",
 *                                              "You collected {} coins", num)),
 *                              num));
 */

static inline std::string _(const std::string& message)
{
  std::string translated_message = message;
  if (g_dictionary_manager)
  {
    translated_message = g_dictionary_manager->get_dictionary().translate(message);
  }

  return replace_variables(translated_message);
}

static inline std::string __(const std::string& message,
    const std::string& message_plural, int num)
{
  std::string translated_message = message;
  if (g_dictionary_manager)
  {
    translated_message = g_dictionary_manager->get_dictionary().translate_plural(message,
        message_plural, num);
  }
  else if (num == 1)
  {
    translated_message = message;
  }
  else
  {
    translated_message = message_plural;
  }

  return replace_variables(translated_message);
}

#endif

/* EOF */
