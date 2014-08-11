//  SuperTux - Add-on
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#include "addon/addon.hpp"

#include <physfs.h>
#include <stdexcept>
#include <sstream>

#include "addon/md5.hpp"
#include "lisp/parser.hpp"
#include "util/reader.hpp"
#include "util/writer.hpp"
#include "util/log.hpp"

std::string
Addon::get_md5() const
{
  if (!installed) {
    if (stored_md5 == "") { log_warning << "Add-on not installed and no stored MD5 available" << std::endl; }
    return stored_md5;
  }

  if (calculated_md5 != "") return calculated_md5;

  if (installed_physfs_filename == "") throw std::runtime_error("Tried to calculate MD5 of Add-on with unknown filename");

  // TODO: this does not work as expected for some files -- IFileStream seems to not always behave like an ifstream.
  //IFileStream ifs(installed_physfs_filename);
  //std::string md5 = MD5(ifs).hex_digest();

  MD5 md5;
  PHYSFS_file* file;
  file = PHYSFS_openRead(installed_physfs_filename.c_str());
  unsigned char buffer[1024];
  while (true) {
    PHYSFS_sint64 len = PHYSFS_read(file, buffer, 1, sizeof(buffer));
    if (len <= 0) break;
    md5.update(buffer, len);
  }
  PHYSFS_close(file);

  calculated_md5 = md5.hex_digest();
  log_debug << "MD5 of " << title << ": " << calculated_md5 << std::endl;

  return calculated_md5;
}

void
Addon::parse(const Reader& lisp)
{
  try {
    lisp.get("kind", kind);  
    lisp.get("title", title);
    lisp.get("author", author);
    lisp.get("license", license);
    lisp.get("http-url", http_url);
    lisp.get("file", suggested_filename);
    lisp.get("md5", stored_md5);
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
  if (kind != "") writer.write("kind", kind);  
  if (title != "") writer.write("title", title);
  if (author != "") writer.write("author", author);
  if (license != "") writer.write("license", license);
  if (http_url != "") writer.write("http-url", http_url);
  if (suggested_filename != "") writer.write("file", suggested_filename);
  if (stored_md5 != "") writer.write("md5", stored_md5);
  writer.end_list("supertux-addoninfo");
}

void 
Addon::write(std::string fname) const
{
  lisp::Writer writer(fname);
  write(writer);
}

bool 
Addon::operator==(Addon addon2) const
{
  std::string s1 = this->get_md5();
  std::string s2 = addon2.get_md5();

  if ((s1 != "") && (s2 != "")) return (s1 == s2);

  if (this->title != addon2.title) return false;
  if (this->author != addon2.author) return false;
  if (this->kind != addon2.kind) return false;
  return true; 
}

/* EOF */
