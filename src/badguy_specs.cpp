/***************************************************************************
               badguy_specs.cpp  -  badguys properties table
                     -------------------
    begin                : Oct, 11 2004
    copyright            : (C) 2004 by Ricardo Cruz
    email                : rick2@aeiou.pt
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <iostream>

#include "special/sprite_manager.h"
#include "resources.h"

#include "badguy_specs.h"

BadGuySpecsManager* badguyspecs_manager = 0;

BadGuySpecsManager::BadGuySpecsManager(const std::string& filename)
{
  load_resfile(filename);
}

BadGuySpecsManager::~BadGuySpecsManager()
{
  for(std::map<std::string, BadGuySpecs*>::iterator i =
     badguys_specs.begin(); i != badguys_specs.end(); ++i)
    delete i->second;
}

void
BadGuySpecsManager::load_resfile(const std::string& filename)
{
  lisp_object_t* root_obj = lisp_read_from_file(filename);
  if (!root_obj)
    {
      std::cout << "BadGuySpecsManager: Couldn't load: " << filename << std::endl;
      return;
    }

  lisp_object_t* cur = root_obj;

  if (strcmp(lisp_symbol(lisp_car(cur)), "supertux-badguys-specifications") != 0)
    return;
  cur = lisp_cdr(cur);

  while(cur)
    {
      lisp_object_t* el = lisp_car(cur);

      if (strcmp(lisp_symbol(lisp_car(el)), "badguy") == 0)
        {
          LispReader reader(lisp_cdr(lisp_car(cur)));
          BadGuySpecs* badguy_specs = new BadGuySpecs(reader);

          BadGuysSpecs::iterator i = badguys_specs.find(badguy_specs->get_name());
          if (i == badguys_specs.end())
            {
              badguys_specs[badguy_specs->get_name()] = badguy_specs;
            }
          else
            {
              delete i->second;
              i->second = badguy_specs;
              std::cerr << "Warning: dulpicate entry: '" << badguy_specs->get_name() << "'" << std::endl;
            }
        }
      else
        {
          std::cout << "BadGuySpecsManager: Unknown tag" << std::endl;
        }

      cur = lisp_cdr(cur);
    }

  lisp_free(root_obj);
}

BadGuySpecs*
BadGuySpecsManager::load(const std::string& name)
{
  BadGuysSpecs::iterator i = badguys_specs.find(name);
  if (i == badguys_specs.end())
    {
      std::cerr << "Warning: BadGuy specification '" << name << "' not found" << std::endl;
      return 0;
    }
  return i->second;
}

BadGuySpecs::BadGuySpecs(LispReader& reader)
{
  reset();

  std::string str;
  reader.read_string("kind", str);
  kind = str;

  str.clear();
  reader.read_string("inherits", str);
  if(!str.empty())
    {
    BadGuySpecs* bgspecs = badguyspecs_manager->load(str);
    if(bgspecs)
      {
      sprite = bgspecs->sprite;
      }
    else
      std::cerr << "Warning: inherited '" << str
                << "was not found.\nProbably, it was declared after"
                   "this entry '" << kind << std::endl;
    }

  str.clear();
  reader.read_string("sprite", str);
  if(str.empty())
    std::cerr << "Warning: No sprite has been set to badguy " << kind << std::endl;
  else
    sprite = sprite_manager->load(str);

  if(!sprite)
    {
    std::cerr << "Warning: Sprite '" << str << "' could not be loaded.\n";
    }
}

BadGuySpecs::BadGuySpecs(std::string& kind_)
{
  reset();
  kind = kind_;
}

BadGuySpecs::~BadGuySpecs()
{
}

void
BadGuySpecs::reset()
{
  kind.clear();
}

std::string
BadGuySpecs::get_name()
{
return kind;
}

// EOF //
