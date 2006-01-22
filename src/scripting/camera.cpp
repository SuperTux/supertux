#include <config.h>

#include <string>
#include <stdio.h>
#include "object/camera.hpp"
#include "scripting/camera.hpp"
#include "math/vector.hpp"

#define NOIMPL      printf("%s not implemented.\n", __PRETTY_FUNCTION__);

namespace Scripting
{
  Camera::Camera(::Camera* camera)
    : camera(camera)
  { }

  Camera::~Camera()
  { }
  
  void
  Camera::shake(float , float , float )
  {
    NOIMPL;
  }
  
  void
  Camera::set_pos(float , float )
  {
    NOIMPL;
  }

  void
  Camera::set_mode(const std::string& )
  {
    NOIMPL;
  }

  void
  Camera::scroll_to(float x, float y, float scrolltime)
  {
    camera->scroll_to(Vector(x, y), scrolltime);
  }
}
