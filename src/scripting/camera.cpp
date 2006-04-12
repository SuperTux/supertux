#include <config.h>

#include <string>
#include <stdio.h>
#include "object/camera.hpp"
#include "scripting/camera.hpp"
#include "math/vector.hpp"

#define NOIMPL      log_fatal << __PRETTY_FUNCTION__ << " not implemented."

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
  Camera::set_mode(const std::string& mode)
  {
    if(mode == "normal") {
      camera->mode = ::Camera::NORMAL;
    } else if(mode == "manual") {
      camera->mode = ::Camera::MANUAL;
    } else {
      log_fatal << "Camera mode '" << mode << "' unknown.";
    }
  }

  void
  Camera::scroll_to(float x, float y, float scrolltime)
  {
    camera->scroll_to(Vector(x, y), scrolltime);
  }
}
