/***************************************************************************
               badguy_specs.h  -  badguys properties table
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

#ifndef SUPERTUX_BADGUY_SPECS_H
#define SUPERTUX_BADGUY_SPECS_H

#include <string>
#include <map>

#include "utils/lispreader.h"

using namespace SuperTux;

namespace SuperTux {
class Sprite;
}

class BadGuySpecs;
class BadGuySpecsManager;
extern BadGuySpecsManager* badguyspecs_manager;

class BadGuySpecsManager
{
public:
  BadGuySpecsManager(const std::string& filename);
  ~BadGuySpecsManager();

  void  load_resfile(const std::string& filename);
  /** loads a sprite.
   * WARNING: You must not delete the returned object.
   */
  BadGuySpecs* load(const std::string& name);

private:
  typedef std::map<std::string, BadGuySpecs*> BadGuysSpecs;
  BadGuysSpecs badguys_specs;
};

class BadGuySpecs
{
public:
  BadGuySpecs(LispReader& reader);
  BadGuySpecs(std::string& kind);
  ~BadGuySpecs();

  std::string get_name();

  // proprities
  std::string kind;
  Sprite* sprite;

private:
  void reset();
};


#endif /*SUPERTUX_BADGUY_SPECS_H*/
