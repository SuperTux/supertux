//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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
#include "display_manager.h"

#include <algorithm>

DisplayManager::DisplayManager()
{
}

DisplayManager::~DisplayManager()
{
}

void
DisplayManager::add_drawable(Drawable* drawable, int layer)
{
  DisplayList::iterator i 
    = std::lower_bound(displaylist.begin(), displaylist.end(), layer);
  if(i == displaylist.end())
    displaylist.push_back(DrawingQueueEntry(drawable, layer));
  else
    displaylist.insert(i, DrawingQueueEntry(drawable, layer));
}

void
DisplayManager::remove_drawable(Drawable* drawable)
{
  for(DisplayList::iterator i = displaylist.begin(); i != displaylist.end();) {
    if(i->object == drawable)
      i = displaylist.erase(i);
    else
      ++i;
  }
}

void
DisplayManager::draw(Camera& camera)
{
  for(DisplayList::iterator i = displaylist.begin(); i != displaylist.end();
      ++i)
    i->object->draw(camera, i->layer);
}

