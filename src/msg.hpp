//  $Id: debug.cpp 2650 2005-06-28 12:42:08Z sommer $
// 
//  SuperTux Debug Helper Functions
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef __SUPERTUX_MSG_H__
#define __SUPERTUX_MSG_H__

#include <iostream>
#include <stdio.h>

#include "console.hpp"

// TODO: make macros more C++ish?

#ifdef DEBUG

#define msg_debug(message) Console::output << "[DEBUG] " << __FILE__ << " l." << __LINE__ << ": " << message << std::endl
#define msg_info(message) Console::output << "[INFO] " << message << std::endl
#define msg_warning(message) Console::output << "[WARNING] " << __FILE__ << " l." << __LINE__ << ": " << message << std::endl
#define msg_fatal(message) Console::output << "[FATAL] " << __FILE__ << " l." << __LINE__ << ": " << message << std::endl

#else

#define msg_debug(message) 
#define msg_info(message) Console::output << message << std::endl
#define msg_warning(message) Console::output << "Warning: " << message << std::endl
#define msg_fatal(message) Console::output << "Fatal: " << message << std::endl

#endif

#endif

