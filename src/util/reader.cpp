/*
 * src/util/reader.cpp - Utility functions for config handling.
 * Copyright (C) 2010  Florian Forster
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors:
 *   Florian "octo" Forster <supertux at octo.it>
 */

#include "util/reader.hpp"
#include "video/drawing_request.hpp" /* LAYER_GUI */

int reader_get_layer (const Reader& reader, int def)
{
  int tmp = 0;
  bool status;

  status = reader.get ("z-pos", tmp);

  if (!status)
    status = reader.get ("layer", tmp);

  if (!status)
    tmp = def;

  if (tmp > (LAYER_GUI - 100))
    tmp = LAYER_GUI - 100;

  return (tmp);
} /* int reader_get_layer */

/* vim: set sw=2 et sts=2 : */
