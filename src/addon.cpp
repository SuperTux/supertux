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

#include <sstream>
#include <stdexcept>
#include "addon.hpp"
#include "addon_manager.hpp"

void
Addon::install()
{
  AddonManager& adm = AddonManager::get_instance();
  adm.install(*this);
}

void
Addon::remove()
{
  AddonManager& adm = AddonManager::get_instance();
  adm.remove(*this);
}
  
void
Addon::parse(const lisp::Lisp& lisp)
{
  try {
    lisp.get("kind", kind);  
    lisp.get("title", title);
    lisp.get("author", author);
    lisp.get("license", license);
    lisp.get("http-url", http_url);
    lisp.get("file", file);
    lisp.get("md5", md5);
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when parsing addoninfo: " << e.what();
    throw std::runtime_error(msg.str());
  }
}

void
Addon::parse(std::string fname)
{
  try {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(fname);
    const lisp::Lisp* addon = root->get_lisp("supertux-addoninfo");
    if(!addon) throw std::runtime_error("file is not a supertux-addoninfo file.");
    parse(*addon);
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when reading addoninfo '" << fname << "': " << e.what();
    throw std::runtime_error(msg.str());
  }
}

void
Addon::write(lisp::Writer& writer) const
{
  writer.start_list("supertux-addoninfo");
  if (kind != "") writer.write_string("kind", kind);  
  if (title != "") writer.write_string("title", title);
  if (author != "") writer.write_string("author", author);
  if (license != "") writer.write_string("license", license);
  if (http_url != "") writer.write_string("http-url", http_url);
  if (file != "") writer.write_string("file", file);
  if (md5 != "") writer.write_string("md5", md5);
  writer.end_list("supertux-addoninfo");
}

void 
Addon::write(std::string fname) const
{
  lisp::Writer writer(fname);
  write(writer);
}

bool 
Addon::equals(const Addon& addon2) const
{
  if ((this->md5 == "") || (addon2.md5 == "")) return (this->title == addon2.title);
  return (this->md5 == addon2.md5);
}

