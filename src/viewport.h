#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__

#include "vector.h"
#include "rectangle.h"

class ViewPort
{
public:
  ViewPort();
  ~ViewPort();

  Vector world2screen(const Vector& worldpos) const
  {
    return worldpos - translation;
  }
    
  /** returns the current translation (=scroll) vector of the viewport */
  const Vector& get_translation() const
  { return translation; }

  void set_translation(const Vector& translation);

private:
  Vector translation;
};

#endif

