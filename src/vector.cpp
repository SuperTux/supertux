#include "vector.h"
#include <math.h>

Vector Vector::unit() const
{
  return *this / norm();
}

float Vector::norm() const
{
  return sqrt(x*x + y*y);
}
