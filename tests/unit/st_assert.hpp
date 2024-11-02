// st_assert.hpp - Assert function for tests
// Copyright (C) 2024 Hyland B. <me@ow.swag.toys>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#pragma once
#include <iostream>
#include <optional>
#include <cstdlib>
#include <string_view>

namespace {

#define ST_ASSERT(name, expr) st_assert(__FILE__, __LINE__, name, expr)

// TODO Make macros for these assert functions to also pass a line number
void st_assert(const std::string_view filename,
               int linenum,
               const std::optional<std::string_view>& name,
               bool expr)
{
	if (expr)
	{
		if (name.has_value())
			std::cout << "-- Test \"" << *name << "\" passed!" << std::endl;
		return;
	}
	
	if (name.has_value())
		std::cerr << "\e[31m(" << filename << ":" << linenum << ")\n-- Test \"" << *name << "\" failed!\e[0m" << std::endl;
	abort();
}

// template<typename T, typename Q>
// void st_assert(const std::optional<std::string_view>& name,
//                const T& one,
// 			   const Q& two)
// {
// 	st_assert(one == two);
// }

}
