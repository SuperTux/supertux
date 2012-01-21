//  tinygettext - A gettext replacement that works directly on .po files
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

#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "tinygettext/po_parser.hpp"
#include "tinygettext/tinygettext.hpp"
#include "tinygettext/log.hpp"

void my_log_callback(const std::string& err)
{
  std::cerr << err;
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout << argv[0] << " FILENAME..." << std::endl;
  }
  else
  {
    tinygettext::Log::set_log_info_callback(my_log_callback);
    tinygettext::Log::set_log_warning_callback(my_log_callback);
    tinygettext::Log::set_log_error_callback(my_log_callback);

    for(int i = 1; i < argc; ++i)
    {
      std::ifstream in(argv[i]);
      if (!in)
      {
        std::cerr << argv[0] << ": cannot access " << argv[i] << ": " << strerror(errno) << std::endl;
      }
      else
      {
        try 
        {
          tinygettext::Dictionary dict1;
          tinygettext::POParser::parse(argv[i], in, dict1);

          //tinygettext::Dictionary dict2;
          //tinygettext::POFileReader::read(in, dict2);
        }
        catch(std::runtime_error& err)
        {
          std::cout << argv[i] << ": exception: " << err.what() << std::endl;
        }
      }
    }
  }  
}

/* EOF */
