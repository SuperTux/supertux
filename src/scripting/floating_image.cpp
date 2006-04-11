#include <config.h>

#include <assert.h>
#include "floating_image.hpp"
#include "sector.hpp"
#include "object/floating_image.hpp"

namespace Scripting
{

FloatingImage::FloatingImage(const std::string& spritefile)
{
  assert(Sector::current() != NULL);
  floating_image = new _FloatingImage(spritefile); 
  Sector::current()->add_object(floating_image);
}

FloatingImage::~FloatingImage()
{
  floating_image->remove_me();
  // no delete here, Sector will do that
}

void
FloatingImage::set_layer(int layer)
{
  floating_image->set_layer(layer);
}

int
FloatingImage::get_layer()
{
  return floating_image->get_layer();
}

void
FloatingImage::set_pos(float x, float y)
{
  floating_image->set_pos(Vector(x, y));
}

float
FloatingImage::get_pos_x()
{
  return floating_image->get_pos().x;
}

float
FloatingImage::get_pos_y()
{
  return floating_image->get_pos().y;
}

void
FloatingImage::set_anchor_point(int anchor)
{
  floating_image->set_anchor_point((AnchorPoint) anchor);
}

int
FloatingImage::get_anchor_point()
{
  return (int) floating_image->get_anchor_point();
}

bool
FloatingImage::get_visible()
{
  return floating_image->get_visible();
}

void
FloatingImage::set_visible(bool visible)
{
  floating_image->set_visible(visible);
}

}
