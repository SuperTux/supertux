#ifndef __COLOR_HPP__
#define __COLOR_HPP__

#include <vector>

class Color
{
public:
  Color()
    : red(0), green(0), blue(0), alpha(1.0)
  { }
  Color(float red, float green, float blue, float alpha = 1.0)
    : red(red), green(green), blue(blue), alpha(alpha)
  { }
  Color(const std::vector<float>& vals)
  {
    red = vals[0];
    green = vals[1];
    blue = vals[2];
    if(vals.size() > 3)
      alpha = vals[3];
  }

  float red, green, blue, alpha;
};

#endif

