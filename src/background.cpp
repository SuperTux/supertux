#include "background.h"

#include "globals.h"
#include "viewport.h"
#include "display_manager.h"

Background::Background(DisplayManager& displaymanager)
{
  displaymanager.add_drawable(this, LAYER_BACKGROUND0);
}

Background::~Background()
{
}

void
Background::action(float)
{
}

void
Background::set_image(Surface* image, float speed)
{
  bgtype = BACKGROUND_IMAGE;
  this->image = image;
  this->speed = speed;
}

void
Background::set_gradient(Color top, Color bottom)
{
  bgtype = BACKGROUND_GRADIENT;
  gradient_top = top;
  gradient_bottom = bottom;
}

void
Background::draw(ViewPort& viewport, int )
{
  if(bgtype == BACKGROUND_GRADIENT) {
    drawgradient(gradient_top, gradient_bottom);
  } else if(bgtype == BACKGROUND_IMAGE) {
    int sx = int(-viewport.get_translation().x * float(speed/100.))
      % image->w - image->w;
    int sy = int(-viewport.get_translation().y * float(speed/100.))
      % image->h - image->h;
    for(int x = sx; x < screen->w; x += image->w)
      for(int y = sy; y < screen->h; y += image->h)
        image->draw(x, y);
  }
}

