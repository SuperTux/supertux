#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

#include "texture.h"
#include "game_object.h"
#include "drawable.h"

enum {
  BACKGROUND_GRADIENT,
  BACKGROUND_IMAGE
};

class DisplayManager;

class Background : public _GameObject, public Drawable
{
public:
  Background(DisplayManager& displaymanager);
  virtual ~Background();

  void set_image(Surface* image, float bkgd_speed);

  void set_gradient(Color top, Color bottom);

  virtual std::string type() const
  { return "Background"; }

  virtual void action(float elapsed_time);

  virtual void draw(ViewPort& viewport, int layer);

private:
  int bgtype;
  float speed;
  Surface* image;
  Color gradient_top, gradient_bottom;
};

#endif

