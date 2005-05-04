#include <config.h>

#include <string>
#include <stdio.h>
#include "camera.h"

#define NOIMPL      printf("%s not implemented.\n", __PRETTY_FUNCTION__);

namespace Scripting
{
  Camera::Camera()
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
}
