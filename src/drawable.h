#ifndef __DRAWABLE_H__
#define __DRAWABLE_H__

class ViewPort;

/** interface for all game objects that can be drawn on screen.
 */
class Drawable
{
public:
  /** This function draws the object on screen.
   */                                              
  virtual void draw(ViewPort& viewport, int layer) = 0;
};

#endif

