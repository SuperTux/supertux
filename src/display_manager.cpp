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
DisplayManager::draw()
{
  for(DisplayList::iterator i = displaylist.begin(); i != displaylist.end();
      ++i)
    i->object->draw(viewport, i->layer);
}

