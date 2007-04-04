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
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"

/**
 * Represents an (available or installed) Add-on, e.g. a level set
 */
class Addon
{
public:
  std::string kind;
  std::string title;
  std::string author;
  std::string license;
  std::string http_url;
  std::string file;
  std::string md5;

  bool isInstalled;

  /**
   * Download and install Add-on
   */
  void install();

  /**
   * Physically delete Add-on
   */
  void remove();

  /**
   * Read additional information from given contents of a (supertux-addoninfo ...) block
   */
  void parse(const lisp::Lisp& lisp);

  /**
   * Read additional information from given file
   */
  void parse(std::string fname);

  /**
   * Writes out Add-on metainformation to a Lisp Writer
   */
  void write(lisp::Writer& writer) const;

  /**
   * Writes out Add-on metainformation to a file
   */
  void write(std::string fname) const;

  /**
   * Checks if Add-on is the same as given one. 
   * If available, checks MD5 sum, else relies on title alone.
   */
  bool equals(const Addon& addon2) const;

};

#endif
