#ifndef __DISPLAY_MANAGER_H__
#define __DISPLAY_MANAGER_H__

#include <vector>

#include "drawable.h"
#include "viewport.h"

// some constants for predefined layer values
enum {
  LAYER_BACKGROUND0 = -300,
  LAYER_BACKGROUND1 = -200,
  LAYER_BACKGROUNDTILES = -100,
  LAYER_TILES = 0,
  LAYER_OBJECTS = 100,
  LAYER_FOREGROUND0 = 200,
  LAYER_FOREGROUND1 = 300
};

/** This class holds a list of all things that should be drawn to screen
 */
class DisplayManager
{
public:
  DisplayManager();
  ~DisplayManager();
  
  /** adds an object to the list of stuff that should be drawn each frame.
   * The layer argument specifies how early an object is drawn.
   */
  void add_drawable(Drawable* object, int layer);

  void remove_drawable(Drawable* object);

  void draw();

  ViewPort& get_viewport()
  { return viewport; }

private:
  class DrawingQueueEntry {
  public:
    DrawingQueueEntry(Drawable* newobject, int newlayer)
      : object(newobject), layer(newlayer)
    { }

    bool operator <(int olayer) const
    {
      return layer < olayer;
    }

    Drawable* object;
    int layer;
  };

  typedef std::vector<DrawingQueueEntry> DisplayList;
  DisplayList displaylist;
  ViewPort viewport;
};

#endif

