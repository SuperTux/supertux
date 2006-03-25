#ifndef __COLOR_HPP__
#define __COLOR_HPP__

#include <vector>
#include "msg.hpp"

class Color
{
public:
  Color()
    : red(0), green(0), blue(0), alpha(1.0)
  { }
  Color(float red, float green, float blue, float alpha = 1.0)
    : red(red), green(green), blue(blue), alpha(alpha)
  {
#ifdef DEBUG
    check_color_ranges();
#endif
  }
  Color(const std::vector<float>& vals)
  {
    red = vals[0];
    green = vals[1];
    blue = vals[2];
    if(vals.size() > 3)
      alpha = vals[3];
    else
      alpha = 1.0;
#ifdef DEBUG
    check_color_ranges();
#endif
  }

  void check_color_ranges()
  {
    if(red < 0 || red > 1.0 || green < 0 || green > 1.0
            || blue < 0 || blue > 1.0
            || alpha < 0 || alpha > 1.0)
      msg_warning("color value out of range: " << red << ", " << green << ", " << blue << ", " << alpha);
  }

  float red, green, blue, alpha;
};

#endif

