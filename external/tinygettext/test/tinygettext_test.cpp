//  TinyGetText
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <iostream>
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include "tinygettext/po_parser.hpp"
#include "tinygettext/tinygettext.hpp"

using namespace tinygettext;

void print_msg(const std::string& msgid, const std::vector<std::string>& msgstrs)
{
  std::cout << "Msgid: " << msgid << std::endl;
  for(std::vector<std::string>::const_iterator i = msgstrs.begin(); i != msgstrs.end(); ++i)
    {
      std::cout << *i << std::endl;
    }
}

void print_msg_ctxt(const std::string& ctxt, const std::string& msgid, const std::vector<std::string>& msgstrs) 
{
  std::cout << "Msgctxt: " << ctxt << std::endl;
  std::cout << "Msgid: " << msgid << std::endl;
  for(std::vector<std::string>::const_iterator i = msgstrs.begin(); i != msgstrs.end(); ++i)
    {
      std::cout << *i << std::endl;
    }
}

void print_usage(int /*argc*/, char** argv)
{
  std::cout << "Usage: " << argv[0] << " translate FILE MESSAGE" << std::endl;
  std::cout << "       " << argv[0] << " translate FILE MESSAGE_S MESSAGE_P NUM" << std::endl;
  std::cout << "       " << argv[0] << " directory DIRECTORY MESSAGE [LANG]" << std::endl;
  std::cout << "       " << argv[0] << " language LANGUAGE" << std::endl;
  std::cout << "       " << argv[0] << " language-dir DIR" << std::endl;
  std::cout << "       " << argv[0] << " list-msgstrs FILE" << std::endl;
}

void read_dictionary(const std::string& filename, Dictionary& dict)
{
  std::ifstream in(filename.c_str());
  
  if (!in)
    {
      throw std::runtime_error("Couldn't open " + filename);
    }
  else
    {
      POParser::parse(filename, in, dict);
      in.close();
    }
}

int main(int argc, char** argv)
{
  try 
  {
    if (argc == 3 && strcmp(argv[1], "language-dir") == 0)
    {
      DictionaryManager dictionary_manager;
      dictionary_manager.add_directory(argv[2]);
      const std::set<Language>& languages = dictionary_manager.get_languages();
      std::cout << "Number of languages: " << languages.size() << std::endl;
      for (std::set<Language>::const_iterator i = languages.begin(); i != languages.end(); ++i)
      {
        const Language& language = *i;
        std::cout << "Env:       " << language.str()           << std::endl
                  << "Name:      " << language.get_name()      << std::endl
                  << "Language:  " << language.get_language()  << std::endl
                  << "Country:   " << language.get_country()   << std::endl
                  << "Modifier:  " << language.get_modifier()  << std::endl
                  << std::endl;
      }
    }
    else if (argc == 3 && strcmp(argv[1], "language") == 0)
    {
      Language language = Language::from_name(argv[2]);

      if (language)
        std::cout << "Env:       " << language.str()           << std::endl
                  << "Name:      " << language.get_name()      << std::endl
                  << "Language:  " << language.get_language()  << std::endl
                  << "Country:   " << language.get_country()   << std::endl
                  << "Modifier:  " << language.get_modifier()  << std::endl;
      else
        std::cout << "not found" << std::endl;
    }
    else if (argc == 4 && strcmp(argv[1], "translate") == 0)
    {
      const char* filename = argv[2];
      const char* message  = argv[3];

      Dictionary dict;
      read_dictionary(filename, dict);
      std::cout << "TRANSLATION: \"\"\"" << dict.translate(message) << "\"\"\""<< std::endl;
    }
    else if (argc == 5 && strcmp(argv[1], "translate") == 0)
    {
      const char* filename = argv[2];
      const char* context  = argv[3];
      const char* message  = argv[4];

      Dictionary dict;
      read_dictionary(filename, dict);
      std::cout << dict.translate_ctxt(context, message) << std::endl;
    }
    else if (argc == 6 && strcmp(argv[1], "translate") == 0)
    {
      const char* filename = argv[2];
      const char* message_singular = argv[3];
      const char* message_plural   = argv[4];
      int num = atoi(argv[5]);

      Dictionary dict;
      read_dictionary(filename, dict);
      std::cout << dict.translate_plural(message_singular, message_plural, num) << std::endl;
    }
    else if (argc == 7 && strcmp(argv[1], "translate") == 0)
    {
      const char* filename = argv[2];
      const char* context  = argv[3];
      const char* message_singular = argv[4];
      const char* message_plural   = argv[5];
      int num = atoi(argv[6]);

      Dictionary dict;
      read_dictionary(filename, dict);
      std::cout << dict.translate_ctxt_plural(context, message_singular, message_plural, num) << std::endl;
    }
    else if ((argc == 4 || argc == 5) && strcmp(argv[1], "directory") == 0)
    {
      const char* directory = argv[2];
      const char* message   = argv[3];
      const char* language  = (argc == 5) ? argv[4] : NULL;
          
      DictionaryManager manager;
      manager.add_directory(directory);
          
      if (language)
      {
        Language lang = Language::from_name(language);
        if (lang)
        {
          manager.set_language(lang);
        }
        else
        {
          std::cout << "Unknown language: " << language << std::endl;
          exit(EXIT_FAILURE);
        }
      }

      std::cout << "Directory:   '" << directory << "'"  << std::endl;
      std::cout << "Message:     '" << message << "'" << std::endl;
      std::cout << "Language:    '" << manager.get_language().str() << "' (name: '" 
                << manager.get_language().get_name() << "', language: '"
                << manager.get_language().get_language() << "', country: '"
                << manager.get_language().get_country() << "', modifier: '"
                << manager.get_language().get_modifier() << "')"
                << std::endl;
      std::cout << "Translation: '" << manager.get_dictionary().translate(message) << "'" << std::endl;
    }
    else if ((argc == 3) && strcmp(argv[1], "list-msgstrs") == 0)
    {
      const char* filename = argv[2];

      Dictionary dict;
      read_dictionary(filename, dict);
      dict.foreach(print_msg);
      dict.foreach_ctxt(print_msg_ctxt);
    }
    else
    {
      print_usage(argc, argv);
    }
  }
  catch(std::exception& err) 
  {
    std::cout << "Exception: " << err.what() << std::endl;
  }
  
  return 0;
}

/* EOF */
