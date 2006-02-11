#ifndef __FLOATING_IMAGE_HPP__
#define __FLOATING_IMAGE_HPP__

#ifndef SCRIPTING_API
#define __suspend
#include <string>

class FloatingImage;
typedef FloatingImage _FloatingImage;
#endif

namespace Scripting
{

class FloatingImage
{
public:
  FloatingImage(const std::string& spritefile);
  ~FloatingImage();

  void set_layer(int layer);
  int get_layer();
  void set_pos(float x, float y);
  float get_pos_x();
  float get_pos_y();
  void set_anchor_point(int anchor);
  int get_anchor_point();
  void set_visible(bool visible);
  bool get_visible();
  
#ifndef SCRIPTING_API
private:
  _FloatingImage* floating_image;
#endif
};

}

#endif

