//  $Id$
//
//  SuperTux - Add-on
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
#ifndef ADDON_H
#define ADDON_H

#include <string>
#include <vector>

/**
 * Represents an (available or installed) Add-on, e.g. a level set
 */
class Addon
{
public:
  std::string title;
  std::string url;
  std::string fname;
  bool isInstalled;

  /**
   * Download and install Add-on
   */
  void install();

  /**
   * Physically delete Add-on
   */
  void remove();

};

#endif
