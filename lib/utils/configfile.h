//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Michael George <mike@georgetech.com>
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

#ifndef SUPERTUX_CONFIGFILE_H
#define SUPERTUX_CONFIGFILE_H

#include "../utils/lispreader.h"

namespace SuperTux {

FILE * opendata(const std::string& filename, const char * mode);

class Config {
  public:
  void load ();
  void save ();
  virtual void customload(LispReader& reader) {};
  virtual void customsave(FILE * config) {};
};

extern Config* config;

} //namespace SuperTux

#endif

/* Local Variables: */
/* mode:c++ */
/* End: */
