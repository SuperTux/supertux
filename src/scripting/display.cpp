#include <config.h>

#include <string>
#include <stdio.h>
#include "display.h"

#define NOIMPL      printf(__FUNCTION__ " not implemented\n");

namespace Scripting
{
  Display::Display()
  {}

  Display::~Display()
  {}

  void
  Display::set_effect(const std::string& )
  {}
}

