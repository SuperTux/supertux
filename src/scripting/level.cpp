#include <config.h>

#include <string>
#include <stdio.h>
#include "level.h"

#define NOIMPL      printf("%s not implemented.\n", __PRETTY_FUNCTION__);

namespace Scripting
{
  Level::Level()
  {}

  Level::~Level()
  {}

  void
  Level::finish()
  {
    NOIMPL;
  }

  void
  Level::spawn(const std::string& , const std::string& )
  {
    NOIMPL;
  }
}
